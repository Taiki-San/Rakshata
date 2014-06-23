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

@implementation MDL

- (id)init : (NSView*)contentView : (NSString *) state
{
    self = [super init];
    if (self)
	{
		flag = GUI_THREAD_MDL;
		needUpdateMainViews = NO;
		isForcedToShowUp = NO;
		_popover = nil;
		self = [self initView: contentView : state];
		
		self.layer.borderColor = [Prefs getSystemColor:GET_COLOR_BORDER_TABS].CGColor;
		self.layer.borderWidth = 2;
		
		[self initContent:state];
	}
    return self;
}

- (void) initContent : (NSString *) state
{
	controller = [[RakMDLController alloc] init: self : state];
	
	coreView = [[RakMDLView alloc] initContent:[self getCoreviewFrame : [self bounds]] : state : controller];
	if(coreView != nil)
	{
		[self addSubview:coreView];
		[coreView release];
		[self setFrame:[self createFrame]];	//Update the size if required
		needUpdateMainViews = YES;
		[self updateDependingViews];
	}
}

- (BOOL) available
{
	return coreView != nil && [controller getNbElem:YES] != 0;
}

- (void) wakeUp
{
	[coreView wakeUp];
	needUpdateMainViews = YES;
	[self updateDependingViews];
}

- (NSString *) byebye
{
	[controller needToQuit];
	
	if([self retainCount] > 1)
		[self release];
	
	NSString * output = [controller serializeData];
	return output != nil ? output : [super byebye];
}

- (void) dealloc
{
	[controller release];
	[coreView removeFromSuperview];
	[coreView release];
	[super dealloc];
}

/* Proxy */

- (void) proxyAddElement : (MANGAS_DATA) data : (bool) isTome : (int) newElem : (bool) partOfBatch
{
	if(controller != nil)
		[controller addElement:data :isTome :newElem :partOfBatch];
}

- (BOOL) proxyCheckForCollision : (MANGAS_DATA) data : (BOOL) isTome : (int) element
{
	if(controller != nil)
		return [controller checkForCollision:data :isTome :element];
	return false;
}

/*Coreview manipulation*/

- (NSRect) getCoreviewFrame : (NSRect) frame
{
	NSRect output = frame;
	
	output.origin.x = MDL_READERMODE_LATERAL_BORDER * frame.size.width / 100;
	output.size.height -= MDL_READERMODE_BOTTOMBAR_WIDTH;
	output.origin.y = MDL_READERMODE_BOTTOMBAR_WIDTH - MDL_READERMODE_BOTTOMBAR_WIDTH;
	output.size.width -= 2 * output.origin.x;
	
	return output;
}

/*Drag and drop UI effects*/

- (void) setForcedToShowUp : (BOOL) forced	{	isForcedToShowUp = forced;	}
- (BOOL) isForcedToShowUp {		return isForcedToShowUp;	}

- (BOOL) isDisplayed
{
	return (isForcedToShowUp || _lastFrame.origin.y != -_lastFrame.size.height);
}

- (void) dragAndDropStarted:(BOOL)started
{
	if(started)
	{
		if([self isForcedToShowUp])
			[self setForcedToShowUp:NO];
		
		if([self isDisplayed])
			return;
		
		[self setForcedToShowUp:YES];
		
		
	}
	else if ([self isForcedToShowUp])
		[self setForcedToShowUp:NO];

	else
		return;
	
	[coreView hideList:[self isForcedToShowUp]];
	[coreView setFocusDrop:[self isForcedToShowUp]];
	needUpdateMainViews = YES;
	[self updateDependingViews];
}

/*Internal stuffs*/

- (BOOL) isStillCollapsedReaderTab
{
	int state;
	[Prefs getPref:PREFS_GET_READER_TABS_STATE :&state];
	return (state & STATE_READER_TAB_MDL_FOCUS) == 0;
}

- (NSRect) createFrameWithSuperView : (NSView*) superView
{
	NSRect maximumSize = [super createFrameWithSuperView:superView];
	
	if(coreView != nil && !isForcedToShowUp)
	{
		maximumSize.size.height = round(maximumSize.size.height);
		
		CGFloat contentHeight = [coreView getContentHeight] + MDL_READERMODE_BOTTOMBAR_WIDTH;
		
		if([controller getNbElem:YES] == 0)	//Let's get the fuck out of here, it's empty
		{
			maximumSize.size.height = contentHeight;
			maximumSize.origin.y = -contentHeight;
			needUpdateMainViews = YES;
		}
		
		else if(maximumSize.size.height >= contentHeight - 2)
		{
			maximumSize.size.height = contentHeight;
			
			needUpdateMainViews = YES;
			[coreView updateScroller:YES];
		}

		else
			[coreView updateScroller:NO];
	}

	[self updateLastFrame:maximumSize];
	return maximumSize;
}

- (void) updateDependingViews
{
	if(!needUpdateMainViews)
		return;
	
	[self refreshLevelViewsAnimation : self.superview];
	
	needUpdateMainViews = NO;
}

- (NSRect) generateNSTrackingAreaSize : (NSRect) viewFrame
{
	CGFloat posReader;
	NSRect frame = viewFrame;
	[Prefs getPref:PREFS_GET_TAB_READER_POSX :&posReader];
	frame.size.width = posReader * self.superview.frame.size.width / 100;
	frame.origin.x = frame.origin.y = 0;
	
	return frame;
}

- (void) refreshViewSize
{
	[self setFrame: [self createFrame]];
	[self refreshDataAfterAnimation];
}

- (void) refreshDataAfterAnimation
{
	if([controller getNbElem:YES] != 0)
	{
		[super refreshDataAfterAnimation];
		[self updateDependingViews];
	}
}

- (NSRect) getFrameOfNextTab
{
	NSRect output;
	[Prefs getPref:PREFS_GET_TAB_READER_FRAME :&output];
	
	NSSize sizeSuperView = [self.superview frame].size;
	
	output.origin.x *= sizeSuperView.width / 100.0f;
	output.origin.y *= sizeSuperView.height / 100.0f;
	output.size.width *= sizeSuperView.width / 100.0f;
	output.size.height *= sizeSuperView.height / 100.0f;
	
	return output;
}

- (BOOL) acceptsFirstMouse:(NSEvent *)theEvent { return NO; }
- (BOOL) acceptsFirstResponder { return NO; }

- (void) mouseEntered:(NSEvent *)theEvent
{
	if(![self isForcedToShowUp])
		[super mouseEntered:theEvent];
}

/**	 Get View Size	**/

- (NSRect) lastFrame
{
	if(_lastFrame.size.height + _lastFrame.origin.y <= 0)
		return NSMakeRect(0, 0, 0, 0);

	return [super lastFrame];
}

- (CGFloat) getRequestedViewWidth: (CGFloat) widthWindow
{
	CGFloat prefData;
	[Prefs getPref:PREFS_GET_MDL_WIDTH:&prefData];
	return widthWindow * prefData / 100;
}

- (void) setFrame:(NSRect)frameRect
{
	if([self wouldFrameChange:frameRect])
	{
		[super setFrame:frameRect];
		
		if(coreView != nil)
			[coreView setFrame:[self getCoreviewFrame : frameRect]];
		
		if(_popover != nil)
			[_popover locationUpdated:frameRect:NO];
		
		if(needUpdateMainViews)
			[self updateDependingViews];
	}
}

- (void) resizeAnimation
{
	NSRect frame = [self createFrame];
	
	if([self wouldFrameChange:frame])
	{
		[self.animator setFrame:frame];
		if(coreView != nil)
			[coreView resizeAnimation : [self getCoreviewFrame : frame]];
	}
	
	if (([self wouldFrameChange:frame] || ![self isDisplayed]) && _popover != nil)
		[_popover locationUpdated:frame:YES];
}

- (int) getCodePref : (int) request
{
	int output;
	
	switch (request)
	{
		case CONVERT_CODE_POSX:
		{
			output = PREFS_GET_MDL_POSX;
			break;
		}
			
		case CONVERT_CODE_POSY:
		{
			output = PREFS_GET_MDL_POSY;
			break;
		}
			
		case CONVERT_CODE_HEIGHT:
		{
			output = PREFS_GET_MDL_HEIGHT;
			break;
		}
			
		case CONVERT_CODE_WIDTH:
		{
			output = PREFS_GET_MDL_WIDTH;
			break;
		}
			
		case CONVERT_CODE_FRAME:
		{
			output = PREFS_GET_MDL_FRAME;
			break;
		}
			
		default:
			output = 0;
	}
	
	return output;
}

/** Inter-tab communication **/

- (void) propagateContextUpdate : (MANGAS_DATA) data : (bool) isTome : (int) element
{
	NSArray* subviews = self.superview.subviews;
	
	if (subviews == nil)
		return;
	
	RakTabView * currentView;
	
	for (currentView in subviews)
	{
		if([currentView class] == [CTSelec class])
			[currentView updateContextNotification:data :isTome :VALEUR_FIN_STRUCTURE_CHAPITRE];
		else if([currentView class] == [Reader class])
			[currentView updateContextNotification:data :isTome :element];
	}

}

- (void) registerPopoverExistance : (RakReaderControllerUIQuery*) popover
{
	_popover = popover;
}

#pragma mark - Drop support

- (NSDragOperation) dropOperationForSender : (uint) sender
{
	if (sender == GUI_THREAD_SERIES || sender == GUI_THREAD_CT)
		return NSDragOperationCopy;
	
	return [super dropOperationForSender:sender];
}

- (BOOL) receiveDrop : (MANGAS_DATA) data : (bool) isTome : (int) element : (uint) sender;
{
	return (coreView != nil && [coreView proxyReceiveDrop:data :isTome :element :sender]);
}

@end
