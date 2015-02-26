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

- (instancetype) initWithFrame:(NSRect)frameRect
{
	self = [super initWithFrame:frameRect];
	
	if(self != nil)
	{
		[self setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
		[self setAutoresizesSubviews:NO];
		[self setNeedsDisplay:YES];
		[self setWantsLayer:YES];
	}
	
	return self;
}

- (void) setFrame : (NSRect) frameRect
{
	if(_heightOffset)
		frameRect.size.height -= _heightOffset;
		
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
	[Prefs getCurrentTheme:self];
	
	backgroundColor = [self firstBorderColor];
	NSRect frame = self.frame;
	
	frame.size.width -= 2 * WIDTH_BORDER_FAREST;
	frame.size.height -= 2 * WIDTH_BORDER_FAREST;
	frame.origin.x += WIDTH_BORDER_FAREST;
	frame.origin.y += WIDTH_BORDER_FAREST;
	
	internalRows1 = [[RakBorder alloc] initWithFrame:frame : WIDTH_BORDER_MIDDLE : 3.5 : [self middleBorderColor]];
	if (internalRows1 != nil)
		[self addSubview:internalRows1];
	
	frame.size.width -= 2 * WIDTH_BORDER_MIDDLE;
	frame.size.height -= 2 * WIDTH_BORDER_MIDDLE;
	frame.origin.x += WIDTH_BORDER_MIDDLE;
	frame.origin.y += WIDTH_BORDER_MIDDLE;
	
	if(self.window.isMainWindow)
	{
		internalRows2 = [[RakBorder alloc] initWithFrame:frame : WIDTH_BORDER_INTERNAL : 5.0 : [self lastBorderColor]];
		if (internalRows2 != nil)
			[self addSubview:internalRows2];
		
		if(firstResponder == nil)
			[self craftFirstResponder];
		else
		{
			[firstResponder removeFromSuperview];
			[self addSubview:firstResponder];
		}
		
		frame.size.width -= 2 * WIDTH_BORDER_INTERNAL;
		frame.size.height -= 2 * WIDTH_BORDER_INTERNAL;
		frame.origin.x += WIDTH_BORDER_INTERNAL;
		frame.origin.y += WIDTH_BORDER_INTERNAL;
		firstResponder.frame = frame;
	}
}

- (void) craftFirstResponder
{
	if(self.window.isMainWindow)
	{
		firstResponder = [[RakContentView alloc] initWithFrame:_bounds];
		if(firstResponder != nil)
			[self addSubview:firstResponder];
	}
}

- (void) updateUI
{
	backgroundColor = [self firstBorderColor];
	[internalRows1 setColor:[self middleBorderColor]];
	
	if(self.window.isMainWindow)
		[internalRows2 setColor:[self lastBorderColor]];
	
	[self setNeedsDisplay:YES];
}

- (RakContentView *) getFirstResponder
{
	if(firstResponder == nil)
		[self craftFirstResponder];

	return firstResponder;
}

- (void) keyDown:(NSEvent *)theEvent
{
	[firstResponder keyDown:theEvent];
}

- (void) drawRect:(NSRect)dirtyRect
{
	[backgroundColor setFill];
	NSRectFill(dirtyRect);
}

- (void) dealloc
{
	[internalRows1 removeFromSuperview];
	[internalRows2 removeFromSuperview];
	[firstResponder removeFromSuperview];
}

#pragma mark - Color

- (NSColor *) firstBorderColor
{
	return [Prefs getSystemColor:GET_COLOR_EXTERNALBORDER_FAREST : nil];
}

- (NSColor *) middleBorderColor
{
	return [Prefs getSystemColor:GET_COLOR_EXTERNALBORDER_MIDDLE : nil];
}

- (NSColor *) lastBorderColor
{
	return [Prefs getSystemColor:GET_COLOR_EXTERNALBORDER_CLOSEST : nil];
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
		case TAB_SERIES:
		{
			[_tabSerie keyDown:theEvent];
			break;
		}
			
		case TAB_CT:
		{
			[_tabCT keyDown:theEvent];
			break;
		}
			
		case TAB_MDL:
		{
			[_tabMDL keyDown:theEvent];
			break;
		}
			
		case TAB_READER:
		{
			[_tabReader keyDown:theEvent];
			break;
		}
			
		default:
			break;
	}
}

@end
