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

@implementation RakContentViewBack

- (id) initWithCoder:(NSCoder *)aDecoder
{
	self = [super initWithCoder:aDecoder];
	
	if(self != nil)
	{
		[self setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
		[self setAutoresizesSubviews:NO];
		[self setNeedsDisplay:YES];
		[self setWantsLayer:YES];
	}
	
	return self;
}

- (void) setFrame:(NSRect)frameRect
{
	if([(RakWindow*) self.window isFullscreen])
	{
		frameRect.size.height = self.window.frame.size.height;
		frameRect.size.width = self.window.frame.size.width;
	}
	
	[super setFrame:frameRect];
	
	if(![self.subviews count])
		return;
	
	frameRect.size.width -= 2 * WIDTH_BORDER_FAREST;
	frameRect.size.height -= 2 * WIDTH_BORDER_FAREST;
	frameRect.origin.x += WIDTH_BORDER_FAREST;
	frameRect.origin.y += WIDTH_BORDER_FAREST;
	
	[self.subviews[0] setFrame:frameRect];
	
	frameRect.size.width -= 2 * WIDTH_BORDER_MIDDLE;
	frameRect.size.height -= 2 * WIDTH_BORDER_MIDDLE;
	frameRect.origin.x += WIDTH_BORDER_MIDDLE;
	frameRect.origin.y += WIDTH_BORDER_MIDDLE;
	
	[self.subviews[1] setFrame:frameRect];
		
	frameRect.size.width -= 2 * WIDTH_BORDER_INTERNAL;
	frameRect.size.height -= 2 * WIDTH_BORDER_INTERNAL;
	frameRect.origin.x += WIDTH_BORDER_INTERNAL;
	frameRect.origin.y += WIDTH_BORDER_INTERNAL;
	
	for(uint i = 2, count = [self.subviews count]; i < count; i++)
		[self.subviews[i] setFrame:frameRect];
}

- (void) setupBorders
{
	self.window.backgroundColor = [Prefs getSystemColor:GET_COLOR_EXTERNALBORDER_FAREST : self];
	NSRect frame = [self frame];
	
	frame.size.width -= 2 * WIDTH_BORDER_FAREST;
	frame.size.height -= 2 * WIDTH_BORDER_FAREST;
	frame.origin.x += WIDTH_BORDER_FAREST;
	frame.origin.y += WIDTH_BORDER_FAREST;
	
	internalRows1 = [[RakBorder alloc] initWithFrame:frame : WIDTH_BORDER_MIDDLE : 3.5 : [Prefs getSystemColor:GET_COLOR_EXTERNALBORDER_MIDDLE : nil]];
	if (internalRows1 != nil)
	{
		[self addSubview:internalRows1];
		[internalRows1 release];
	}
	
	frame.size.width -= 2 * WIDTH_BORDER_MIDDLE;
	frame.size.height -= 2 * WIDTH_BORDER_MIDDLE;
	frame.origin.x += WIDTH_BORDER_MIDDLE;
	frame.origin.y += WIDTH_BORDER_MIDDLE;
	
	internalRows2 = [[RakBorder alloc] initWithFrame:frame : WIDTH_BORDER_INTERNAL : 5.0 : [Prefs getSystemColor:GET_COLOR_EXTERNALBORDER_CLOSEST : nil]];
	if (internalRows2 != nil)
	{
		[self addSubview:internalRows2];
		[internalRows2 release];
	}
	
	frame.size.width -= 2 * WIDTH_BORDER_INTERNAL;
	frame.size.height -= 2 * WIDTH_BORDER_INTERNAL;
	frame.origin.x += WIDTH_BORDER_INTERNAL;
	frame.origin.y += WIDTH_BORDER_INTERNAL;
	
	firstResponder = [[RakContentView alloc] initWithFrame:frame];
	if(firstResponder != nil)
	{
		[self addSubview:firstResponder];
		[firstResponder release];
	}
}

- (void) updateUI
{
	self.window.backgroundColor = [Prefs getSystemColor:GET_COLOR_EXTERNALBORDER_FAREST : nil];
	[internalRows1 setColor:[Prefs getSystemColor:GET_COLOR_EXTERNALBORDER_MIDDLE : nil]];
	[internalRows2 setColor:[Prefs getSystemColor:GET_COLOR_EXTERNALBORDER_CLOSEST : nil]];
	
	[self setNeedsDisplay:YES];
}

- (RakContentView *) getFirstResponder
{
	return firstResponder;
}

- (void) keyDown:(NSEvent *)theEvent
{
	[firstResponder keyDown:theEvent];
}

- (void) dealloc
{
	[internalRows1 removeFromSuperview];
	[internalRows2 removeFromSuperview];
	[firstResponder removeFromSuperview];
	[super dealloc];
}

@end

@implementation RakContentView

- (id) initWithFrame:(NSRect)frameRect
{
	self = [super initWithFrame:frameRect];
	
	if(self != nil)
	{
		[self setAutoresizesSubviews:NO];
	}
	
	return self;
}

- (void) setFrame:(NSRect)frameRect
{
	[super setFrame:frameRect];

	BOOL oldVal = NO;
	NSView * view;
	NSArray * subviews = self.subviews;
	NSInteger i, count = [subviews count];
	
	for(i = 0; i < count; i++)
	{
		view = [subviews objectAtIndex:i];
		if([view superclass] == [RakTabView class])
		{
			if([view class] == [MDL class])
			{
				oldVal = ((MDL*) view)->needUpdateMainViews;
				((MDL*) view)->needUpdateMainViews = NO;
			}

			[(RakTabView *) view setFrame:[(RakTabView *) view createFrame]];
			
			if([view class] == [MDL class])
			{
				((MDL*) view)->needUpdateMainViews = oldVal;
			}
		}
	}
}

- (void) setupCtx : (Series*) tabSerie : (CTSelec*) tabCT : (Reader*) tabReader : (MDL*) tabMDL
{
	_tabSerie = tabSerie;
	_tabCT = tabCT;
	_tabReader = tabReader;
	_tabMDL = tabMDL;
}

- (void) updateContext : (uint) mainThread : (uint) stateTabsReader
{
	_mainThread = mainThread;
	_stateTabsReader = stateTabsReader;
}

- (void)keyDown:(NSEvent *)theEvent
{
	switch (_mainThread)
	{
		case GUI_THREAD_SERIES:
		{
			[_tabSerie keyDown:theEvent];
			break;
		}
			
		case GUI_THREAD_CT:
		{
			[_tabCT keyDown:theEvent];
			break;
		}
			
		case GUI_THREAD_MDL:
		{
			[_tabMDL keyDown:theEvent];
			break;
		}
			
		case GUI_THREAD_READER:
		{
			[_tabReader keyDown:theEvent];
			break;
		}
			
		default:
			break;
	}
}

@end
