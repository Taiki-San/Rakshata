/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite  **
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
	[self updateDependingViews];
}

- (NSString *) byebye
{
	[coreView needToQuit];
	
	NSString * output = [controller serializeData];
	if(output != nil)
		return output;
	return [super byebye];
}

/* Proxy */

- (void) proxyAddElement : (MANGAS_DATA) data : (bool) isTome : (int) newElem
{
	if(controller != nil)
		[controller addElement:data :isTome :newElem];
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

- (void) setFrame:(NSRect)frameRect
{
	if([self wouldFrameChange:frameRect])
	{
		[self internalSetFrame:frameRect];
		
		if(coreView != nil)
			[coreView setFrame:[self getCoreviewFrame : frameRect]];
		
		if(needUpdateMainViews)
			[self updateDependingViews];
	}
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
	
	if(coreView != nil)
	{
		maximumSize.size.height = round(maximumSize.size.height);
		
		CGFloat contentHeight = [coreView getContentHeight] + MDL_READERMODE_BOTTOMBAR_WIDTH;
		
		if(maximumSize.size.height >= contentHeight - 2)
		{
			maximumSize.size.height = contentHeight;
			
			if([controller getNbElem:YES] == 0)	//Let's get the fuck out of here
				maximumSize.origin.y = -contentHeight;
			else
			{
				needUpdateMainViews = YES;
				[coreView updateScroller:YES];
			}
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
	
	NSView * view;
	NSArray * subviews = [[self superview] subviews];
	uint count = [subviews count];
	
	for (uint i = 0; i < count; i++)
	{
		view = [subviews objectAtIndex:i];

		if([view superclass] == [RakTabView class])
		{
			if([((RakTabView*) view) needToConsiderMDL])
				[view setFrame:[(RakTabView *) view createFrame]];
		}
	}
	
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

/**	 Get View Size	**/

- (CGFloat) getRequestedViewWidth: (CGFloat) widthWindow
{
	CGFloat prefData;
	[Prefs getPref:PREFS_GET_MDL_WIDTH:&prefData];
	return widthWindow * prefData / 100;
}

- (void) resizeAnimation
{
	NSRect frame = [self createFrame];
	
	if([self wouldFrameChange:frame])
	{
		[self.animator setFrame:frame];
		[coreView resizeAnimation : [self getCoreviewFrame : frame]];
	}
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
	
	uint count = [subviews count];
	RakTabView * currentView;
	
	for (uint i = 0; i < count; i++)
	{
		currentView = [subviews objectAtIndex:i];
		
		if([currentView class] == [CTSelec class])
			[currentView updateContextNotification:data :isTome :VALEUR_FIN_STRUCTURE_CHAPITRE];
		else if([currentView class] == [Reader class])
			[currentView updateContextNotification:data :isTome :element];
	}

}

@end
