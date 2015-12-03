/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **		Source code and assets are property of Taiki, distribution is stricly forbidden		**
 **                                                                                         **
 *********************************************************************************************/

enum
{
	BORDER_BOTTOM = 14,
	BORDER_BOTTOM_EXTENDED = 14 + 20,
	OFFSET_BUTTON = 28
};

@implementation MDL

- (instancetype) init : (NSView *) contentView : (NSString *) state
{
	self = [super init];
	if(self != nil)
	{
		flag = TAB_MDL;
		_needUpdateMainViews = NO;
		self.forcedToShowUp = NO;
		_popover = nil;
		
		[self initView: contentView : state];
		canDeploy = NO;
		
		self.layer.borderColor = [Prefs getSystemColor:COLOR_TABS_BORDER].CGColor;
		self.layer.borderWidth = 2;
		
		if(![self initContent:state])
			self = nil;
	}
	return self;
}

- (BOOL) initContent : (NSString *) state
{
	//We have to split state in two parts: the first char (0/1, _wantCollapse), and if there is content afterward, \n and the dump of the download list
	BOOL oldWantCollapse = NO;
	if([state length] != 0)	//Any content
	{
		if([state UTF8String][0] == '1')
			oldWantCollapse = YES;
		
		if([state length] > 2 && [state isNotEqualTo:STATE_EMPTY])	//We may have some actual content
			state = [state substringFromIndex:2];
	}
	
	//Actual initialization
	controller = [[RakMDLController alloc] init: self : state];
	if(controller == nil)
		return NO;
	
	coreView = [[RakMDLView alloc] initContent:[self getCoreviewFrame : _bounds] : state : controller];
	if(coreView != nil)
		[self addSubview:coreView];
	
	footer = [[RakMDLFooter alloc] initWithFrame:[self getFooterFrame:_bounds] : oldWantCollapse];
	if(footer != nil)
	{
		footer.controller = controller;
		footer.hidden = self.mainThread != TAB_SERIES;
		[self addSubview:footer];
	}
	
	self.wantCollapse = oldWantCollapse;
	
	return YES;
}

- (BOOL) available
{
	return coreView != nil && [controller getNbElem:YES] != 0;
}

- (void) wakeUp
{
	[coreView wakeUp];
	
	_needUpdateMainViews = YES;
	
	if(self.mainThread == TAB_SERIES)
		[[NSApp delegate] serie].forceNextFrameUpdate = YES;

	[self updateDependingViews : YES];
}

- (NSString *) byebye
{
	[controller needToQuit];
	
	NSString * dataDump = [controller serializeData];
	
	if(dataDump == nil)
		return [NSString stringWithFormat:@"%d", _wantCollapse];
	
	return [NSString stringWithFormat:@"%d\n%@", _wantCollapse, dataDump];
}

- (void) dealloc
{
	[coreView removeFromSuperview];
}

- (BOOL) acceptsFirstMouse:(NSEvent *)theEvent { return NO; }
- (BOOL) acceptsFirstResponder { return NO; }

#pragma mark - Proxy

- (void) proxyAddElement : (PROJECT_DATA) data  isTome : (BOOL) isTome element : (uint) newElem  partOfBatch : (BOOL) partOfBatch
{
	if(controller != nil)
		[controller addElement:data :isTome :newElem :partOfBatch];
}

- (BOOL) proxyCheckForCollision : (PROJECT_DATA) data : (BOOL) isTome : (uint) element
{
	return controller != nil ? [controller checkForCollision:data :isTome :element] : NO;
}

- (void) removingEmailAddress
{
	[controller removingEmailAddress];
}

#pragma mark - View sizing manipulation

- (CGFloat) getBottomBorder
{
	return self.mainThread == TAB_SERIES ? BORDER_BOTTOM_EXTENDED : BORDER_BOTTOM;
}

- (NSRect) getCoreviewFrame : (NSRect) frame
{
	NSRect output = frame;
	
	output.origin.x = frame.size.width / 20;
	output.size.width -= 2 * output.origin.x;
	output.origin.y = 0;
	
	output.size.height -= [self getBottomBorder];
	
	return output;
}

- (NSRect) getFooterFrame : (NSRect) frame
{
	NSRect output = frame;
	
	output.origin.y = output.size.height - BORDER_BOTTOM_EXTENDED;
	output.size.height = BORDER_BOTTOM_EXTENDED;
	
	return output;
}

- (NSRect) lastFrame
{
	if(_lastFrame.size.height + _lastFrame.origin.y <= 0 || _lastFrame.size.width + _lastFrame.origin.x <= 0)
		return NSZeroRect;
	
	return [super lastFrame];
}

- (void) resizingCanceled
{
	//The MDL is used as an anchor from time to time
	if(_popover != nil)
	{
		dispatch_async(dispatch_get_main_queue(), ^{
			[_popover locationUpdated:_lastFrame:YES];
		});
	}
}

- (void) resizeAnimation
{
	[super resizeAnimation];
	
	if(_popover != nil && ![self isDisplayed])
		[_popover locationUpdated :[self createFrame] :YES];
}

- (void) resize : (NSRect) frame : (BOOL) animated
{
	NSRect bounds = frame;		bounds.origin = NSZeroPoint;

	if(coreView != nil)
	{
		NSRect coreFrame = [self getCoreviewFrame : bounds];
		
		if(animated)
			[coreView resizeAnimation:coreFrame];
		else
			[coreView setFrame:coreFrame];
	}
	
	if(footer != nil)
	{
		NSRect footerFrame = [self getFooterFrame:bounds];
		
		if(animated)
			[footer resizeAnimation : footerFrame];
		else
			[footer setFrame:footerFrame];
	}
	
	if(_popover != nil)
		[_popover locationUpdated:frame:animated];
	
	if(_needUpdateMainViews)
		[self updateDependingViews : NO];
}

#pragma mark Collapse management

- (void) setWantCollapse : (BOOL) wantCollapse
{
	if(_wantCollapse == wantCollapse)
		return;
	
	_wantCollapse = wantCollapse;
	
	if(!_wantCollapse)
		[self releaseTrackingArea];
	else
		[self resizeTrackingArea];
}

- (void) resizeTrackingArea
{
	[super resizeTrackingArea];
	
	if(self.mainThread == TAB_SERIES)
	{
		NSRect frame = NSMakeRect(0, 0, _lastFrame.size.width, _lastFrame.size.height);
		
		trackingArea = [self addTrackingRect:frame owner:self userData:nil assumeInside:NSPointInRect([self convertPoint:[self.window mouseLocationOutsideOfEventStream] fromView:nil], frame)];
	}
}

- (void) rejectedMouseEntered
{
	if(self.mainThread == TAB_SERIES && seriesCollapsedBySetting)
		[self updateSeriesCollapsedBySettingState : NO];
}

- (void) rejectedMouseExited
{
	if(self.mainThread == TAB_SERIES && _wantCollapse)
		[self updateSeriesCollapsedBySettingState : YES];
}

- (void) updateSeriesCollapsedBySettingState : (BOOL) isCollapsed
{
	seriesCollapsedBySetting = isCollapsed;
	
	[[NSApp delegate] serie].forceNextFrameUpdate = YES;
	
	_needUpdateMainViews = YES;
	[self updateDependingViews:YES];
}

#pragma mark - Sizing

- (BOOL) isStillCollapsedReaderTab
{
	if(self.mainThread != TAB_READER)
		return NO;
	
	uint state;
	[Prefs getPref:PREFS_GET_READER_TABS_STATE :&state];
	return (state & STATE_READER_TAB_MDL_FOCUS) == 0;
}

- (NSRect) createFrameWithSuperView : (NSView*) superview
{
	NSRect maximumSize = [super createFrameWithSuperView:superview];
	
	//Our height is synced to the height of the coreview of the serie tab if focus is on series
	if(self.mainThread == TAB_SERIES)
		maximumSize.size.height = [[[NSApp delegate] serie] getHeightOfMainView];
	
	if(coreView != nil && !self.forcedToShowUp)
	{
		maximumSize.size.height = round(maximumSize.size.height);
		
		CGFloat contentHeight = [coreView getContentHeight] + [self getBottomBorder];
		
		if([controller getNbElem:YES] == 0)	//Let's get the fuck out of here, it's empty
		{
			//The animation is different depending of the focus
			//Series make us slide to the left, while the others to the bottom
			if(_lastFrame.size.width != - _lastFrame.origin.x && _lastFrame.size.height != - _lastFrame.origin.y)
				_needUpdateMainViews = YES;
			
			if(self.mainThread == TAB_SERIES)
			{
				maximumSize.origin.x = -maximumSize.size.width;
			}
			else
			{
				maximumSize.size.height = contentHeight;
				maximumSize.origin.y = -contentHeight;
			}
		}
		else
		{
			//We have more space than we really need
			if(maximumSize.size.height > contentHeight - 1)
			{
				if(self.mainThread != TAB_SERIES)
				{
					maximumSize.size.height = contentHeight;
					
					if(_lastFrame.size.height != contentHeight)
						_needUpdateMainViews = YES;
				}
			}
			
			if(self.mainThread == TAB_SERIES && seriesCollapsedBySetting)
			{
				maximumSize.origin.x = - (maximumSize.size.width * 0.8);
			}
		}
	}
	
	[self setLastFrame:maximumSize];
	return maximumSize;
}

- (void) updateDependingViews : (BOOL) animated
{
	if(!_needUpdateMainViews)
		return;
	
	RakAppDelegate * delegate = (RakAppDelegate *) [NSApp delegate];
	
	if(animated)
	{
		[delegate CT].forceNextFrameUpdate = YES;
		[self refreshLevelViewsAnimation : self.superview];
	}
	else
	{
		for(RakTabView * view in @[[delegate serie], [delegate CT], [delegate reader]])
			[view refreshViewSize];
	}
	
	_needUpdateMainViews = NO;
}

- (void) fastAnimatedRefreshLevel : (NSView*) superview
{
	if(self.mainThread == TAB_SERIES)
		[[NSApp delegate] serie].forceNextFrameUpdate = YES;
	
	else if(self.mainThread == TAB_CT)
		[[NSApp delegate] CT].forceNextFrameUpdate = YES;
	
	[super fastAnimatedRefreshLevel:superview];
}

- (NSRect) generatedReaderTrackingFrame
{
	NSSize svSize = self.superview.frame.size;
	NSRect frame = [self lastFrame];
	
	frame.origin = NSZeroPoint;
	
	[Prefs getPref : PREFS_GET_TAB_READER_POSX : &(frame.size.width) : &svSize];
	
	return frame;
}

- (void) refreshViewSize
{
	[self setFrame: [self createFrame]];
	[self refreshDataAfterAnimation];
}

- (NSRect) getFrameOfNextTab
{
	NSSize sizeSuperview = self.superview.bounds.size;
	NSRect output;
	[Prefs getPref:PREFS_GET_TAB_READER_FRAME :&output : &sizeSuperview];
	
	return output;
}

- (uint) getFrameCode
{
	return PREFS_GET_MDL_FRAME;
}

- (void) mouseEntered:(NSEvent *)theEvent
{
	if(!self.forcedToShowUp)
		[super mouseEntered:theEvent];
}

#pragma mark - Animation

- (void) setUpViewForAnimation : (uint) mainThread
{
	BOOL inSeries = mainThread == TAB_SERIES;
	
	if(inSeries == footer.isHidden)
	{
		if(inSeries)
		{
			footer.alphaValue = 0;
			footer.hidden = NO;
		}
		
		footer.animator.alphaValue = inSeries;
	}
	
	[super setUpViewForAnimation:mainThread];
}

- (void) refreshDataAfterAnimation
{
	if([controller getNbElem:YES] != 0)
	{
		[super refreshDataAfterAnimation];
		[self updateDependingViews : NO];
	}
	
	if(footer.alphaValue == 0)
		footer.hidden = YES;
}

#pragma mark - Login request

- (NSString *) waitingLoginMessage
{
	if(controller != NULL && controller.requestCredentials)
	{
		return NSLocalizedString(@"MDL-LOGIN-TO-PAY", nil);
	}
	
	return NSLocalizedString(@"MDL-LOGIN-TO-AUTH", nil);
}

#pragma mark - Intertab communication

- (void) propagateContextUpdate : (PROJECT_DATA) data : (BOOL) isTome : (uint) element
{
	[[(RakAppDelegate*) [NSApp delegate] CT]		updateContextNotification : data : isTome : INVALID_VALUE];
	[[(RakAppDelegate*) [NSApp delegate] reader]	updateContextNotification : data : isTome : element];
}

- (void) registerPopoverExistance : (RakReaderControllerUIQuery*) popover
{
	_popover = popover;
}

#pragma mark - Drag and drop UI effects

- (BOOL) isDisplayed
{
	return (self.forcedToShowUp || (_lastFrame.origin.y != -_lastFrame.size.height && _lastFrame.origin.x != -_lastFrame.size.width));
}

- (void) dragAndDropStarted : (BOOL)started : (BOOL) canDL
{
	if(!canDL)
		return;
	
	if(started)
	{
		if(self.forcedToShowUp)
			self.forcedToShowUp = NO;
		
		if([self isDisplayed] && !self.waitingLogin)
			return;
		
		self.forcedToShowUp = YES;
	}
	else if(self.forcedToShowUp)
		self.forcedToShowUp = NO;
	
	else
		return;
	
	[coreView hideList: self.forcedToShowUp];
	[coreView setFocusDrop : self.forcedToShowUp];
	
	_needUpdateMainViews = YES;
	[self updateDependingViews : YES];
}

#pragma mark - Drop support

- (NSDragOperation) dropOperationForSender : (uint) sender : (BOOL) canDL
{
	if(!canDL)
		return NSDragOperationNone;
	if(sender == TAB_SERIES || sender == TAB_CT)
		return NSDragOperationCopy;
	
	return [super dropOperationForSender:sender:canDL];
}

- (BOOL) receiveDrop : (PROJECT_DATA) data : (BOOL) isTome : (uint) element : (uint) sender;
{
	return (coreView != nil && [coreView proxyReceiveDrop:data :isTome :element :sender]);
}

@end
