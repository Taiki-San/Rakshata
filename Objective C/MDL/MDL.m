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
		flag = TAB_MDL;
		needUpdateMainViews = NO;
		self.forcedToShowUp = NO;
		_popover = nil;
		self = [self initView: contentView : state];
		canDeploy = false;
		
		self.layer.borderColor = [Prefs getSystemColor:GET_COLOR_BORDER_TABS:self].CGColor;
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
		[self updateDependingViews : NO];
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
	[self updateDependingViews : YES];
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

- (void) proxyAddElement : (PROJECT_DATA) data : (bool) isTome : (int) newElem : (bool) partOfBatch
{
	if(controller != nil)
		[controller addElement:data :isTome :newElem :partOfBatch];
}

- (BOOL) proxyCheckForCollision : (PROJECT_DATA) data : (BOOL) isTome : (int) element
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

- (BOOL) isDisplayed
{
	return (self.forcedToShowUp || _lastFrame.origin.y != -_lastFrame.size.height);
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
	else if (self.forcedToShowUp)
		self.forcedToShowUp = NO;

	else
		return;
	
	[coreView hideList: self.forcedToShowUp];
	[coreView setFocusDrop : self.forcedToShowUp];
	needUpdateMainViews = YES;
	[self updateDependingViews : YES];
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
	
	if(coreView != nil && !self.forcedToShowUp)
	{
		maximumSize.size.height = round(maximumSize.size.height);
		
		CGFloat contentHeight = [coreView getContentHeight] + MDL_READERMODE_BOTTOMBAR_WIDTH;
		
		if([controller getNbElem:YES] == 0)	//Let's get the fuck out of here, it's empty
		{
			if(_lastFrame.size.height != - _lastFrame.origin.y)
				needUpdateMainViews = YES;

			maximumSize.size.height = contentHeight;
			maximumSize.origin.y = -contentHeight;
		}
		
		else if(maximumSize.size.height > contentHeight - 1)
		{
			maximumSize.size.height = contentHeight;
			
			needUpdateMainViews = YES;
			[coreView updateScroller:YES];
		}

		else
			[coreView updateScroller:NO];
	}

	[self setLastFrame:maximumSize];
	return maximumSize;
}

- (void) updateDependingViews : (BOOL) animated
{
	if(!needUpdateMainViews)
		return;
	
	if(animated)
	{
		[self refreshLevelViewsAnimation : self.superview];
	}
	else
	{
		RakAppDelegate * delegate = [NSApp delegate];
		
		[[delegate serie]	createFrame];
		[[delegate CT]		createFrame];
		[[delegate reader]	createFrame];
	}

	needUpdateMainViews = NO;
}

- (NSRect) generateNSTrackingAreaSize : (NSRect) viewFrame
{
	NSSize svSize = self.superview.frame.size;
	NSRect frame = viewFrame;
	
	[Prefs getPref : PREFS_GET_TAB_READER_POSX : &(frame.size.width) : &svSize];
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
		[self updateDependingViews : NO];
	}
}

- (NSRect) getFrameOfNextTab
{
	NSSize sizeSuperview = self.superview.bounds.size;
	NSRect output;
	[Prefs getPref:PREFS_GET_TAB_READER_FRAME :&output : &sizeSuperview];
	
	return output;
}

- (BOOL) acceptsFirstMouse:(NSEvent *)theEvent { return NO; }
- (BOOL) acceptsFirstResponder { return NO; }

- (void) mouseEntered:(NSEvent *)theEvent
{
	if(!self.forcedToShowUp)
		[super mouseEntered:theEvent];
}

/**	 Get View Size	**/

- (NSRect) lastFrame
{
	if(_lastFrame.size.height + _lastFrame.origin.y <= 0)
		return NSZeroRect;

	return [super lastFrame];
}

- (CGFloat) getRequestedViewWidth: (CGFloat) widthWindow
{
	CGFloat prefData;
	NSSize size = NSMakeSize(widthWindow, 0);
	[Prefs getPref : PREFS_GET_MDL_WIDTH : &prefData : &size];
	return prefData;
}

- (void) setFrame:(NSRect)frameRect
{
	if([self wouldFrameChange:frameRect])
	{
		[super setFrame:frameRect];
		[foregroundView setFrame:NSMakeRect(0, 0, frameRect.size.width, frameRect.size.height)];
		
		if(coreView != nil)
			[coreView setFrame:[self getCoreviewFrame : frameRect]];
		
		if(_popover != nil)
			[_popover locationUpdated:frameRect:NO];
		
		if(needUpdateMainViews)
			[self updateDependingViews : NO];
	}
}

- (void) resizeAnimation
{
	NSRect frame = [self createFrame];
	
	if([self wouldFrameChange:frame])
	{
		[self.animator setFrame:frame];
		[foregroundView resizeAnimation:NSMakeRect(0, 0, frame.size.width, frame.size.height)];
		
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

#pragma mark - Login request

- (NSString *) waitingLoginMessage
{
	if(controller != NULL && controller.requestCredentials)
	{
		return @"Votre sélection contient des éléments payants,\nveuillez vous connecter pour lancer la transaction.";
	}
	else
	{
		return @"Suite à la politique des détenteurs des droits\nd'une partie de votre sélection, vous connecter\nest requis pour démarrer le téléchargement.";
	}
}

#pragma mark - Intertab communication

- (void) propagateContextUpdate : (PROJECT_DATA) data : (bool) isTome : (int) element
{
	[[[NSApp delegate] CT]		updateContextNotification : data : isTome : VALEUR_FIN_STRUCT];
	[[[NSApp delegate] reader]	updateContextNotification : data : isTome : element];
}

- (void) registerPopoverExistance : (RakReaderControllerUIQuery*) popover
{
	_popover = popover;
}

#pragma mark - Drop support

- (NSDragOperation) dropOperationForSender : (uint) sender : (BOOL) canDL
{
	if(!canDL)
		return NSDragOperationNone;
	if (sender == TAB_SERIES || sender == TAB_CT)
		return NSDragOperationCopy;
	
	return [super dropOperationForSender:sender:canDL];
}

- (BOOL) receiveDrop : (PROJECT_DATA) data : (bool) isTome : (int) element : (uint) sender;
{
	return (coreView != nil && [coreView proxyReceiveDrop:data :isTome :element :sender]);
}

@end
