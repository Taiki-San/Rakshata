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

@implementation RakContentViewBack

#define WIDTH_BORDER_ALL		4
#define WIDTH_BORDER_FAREST		2
#define WIDTH_BORDER_MIDDLE		1
#define WIDTH_BORDER_INTERNAL	1

- (id) initWithCoder:(NSCoder *)aDecoder
{
	self = [super initWithCoder:aDecoder];
	
	if(self != nil)
	{
		[self setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
		[self setAutoresizesSubviews:YES];
		[self setNeedsDisplay:YES];
		[self setWantsLayer:YES];
	}
	
	return self;
}

- (void) setFrame:(NSRect)frameRect
{
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
	
	[self.subviews[2] setFrame:frameRect];
}

- (void) setupBorders
{
	self.window.backgroundColor = [Prefs getSystemColor:GET_COLOR_EXTERNALBORDER_FAREST];
	NSRect frame = [self frame];
	
	frame.size.width -= 2 * WIDTH_BORDER_FAREST;
	frame.size.height -= 2 * WIDTH_BORDER_FAREST;
	frame.origin.x += WIDTH_BORDER_FAREST;
	frame.origin.y += WIDTH_BORDER_FAREST;
	
	RakBorder * internalRows = [[RakBorder alloc] initWithFrame:frame : WIDTH_BORDER_MIDDLE : 3.5 : [Prefs getSystemColor:GET_COLOR_EXTERNALBORDER_MIDDLE]];
	if (internalRows)
		[self addSubview:internalRows];
	
	frame.size.width -= 2 * WIDTH_BORDER_MIDDLE;
	frame.size.height -= 2 * WIDTH_BORDER_MIDDLE;
	frame.origin.x += WIDTH_BORDER_MIDDLE;
	frame.origin.y += WIDTH_BORDER_MIDDLE;
	
	internalRows = [[RakBorder alloc] initWithFrame:frame : WIDTH_BORDER_INTERNAL : 5.0 : [Prefs getSystemColor:GET_COLOR_EXTERNALBORDER_CLOSEST]];
	if (internalRows)
		[self addSubview:internalRows];
	
	frame.size.width -= 2 * WIDTH_BORDER_INTERNAL;
	frame.size.height -= 2 * WIDTH_BORDER_INTERNAL;
	frame.origin.x += WIDTH_BORDER_INTERNAL;
	frame.origin.y += WIDTH_BORDER_INTERNAL;
	
	firstResponder = [[RakContentView alloc] initWithFrame:frame];
	if(firstResponder != nil)
		[self addSubview:firstResponder];
}

- (RakContentView *) getFirstResponder
{
	return firstResponder;
}

@end

@implementation RakContentView

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
