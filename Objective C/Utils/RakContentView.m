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

#define WIDTH_BORDER_FAREST		2
#define WIDTH_BORDER_MIDDLE		1
#define WIDTH_BORDER_INTERNAL	1

- (void) setupBorders
{
	[self setAutoresizesSubviews:YES];
	
	self.window.backgroundColor = [NSColor colorWithSRGBRed:20/255.0f green:20/255.0 blue:20/255.0 alpha:1.0];
	NSRect frame = [self frame];
	
	frame.size.width -= 2 * WIDTH_BORDER_FAREST;
	frame.size.height -= 2 * WIDTH_BORDER_FAREST;
	frame.origin.x += WIDTH_BORDER_FAREST;
	frame.origin.y += WIDTH_BORDER_FAREST;
	
	RakBorder * internalRows = [[RakBorder alloc] initWithFrame:frame : WIDTH_BORDER_MIDDLE : 3.5 : [NSColor colorWithSRGBRed:32/255.0f green:32/255.0 blue:32/255.0 alpha:1]];
	if (internalRows)
		[self addSubview:internalRows];
	
	frame.size.width -= 2 * WIDTH_BORDER_MIDDLE;
	frame.size.height -= 2 * WIDTH_BORDER_MIDDLE;
	frame.origin.x += WIDTH_BORDER_MIDDLE;
	frame.origin.y += WIDTH_BORDER_MIDDLE;
	
	internalRows = [[RakBorder alloc] initWithFrame:frame : WIDTH_BORDER_INTERNAL : 5.0 : [NSColor colorWithSRGBRed:50/255.0f green:50/255.0 blue:50/255.0 alpha:1]];
	if (internalRows)
		[self addSubview:internalRows];
	
	frame.size.width -= 2 * WIDTH_BORDER_INTERNAL;
	frame.size.height -= 2 * WIDTH_BORDER_INTERNAL;
	frame.origin.x += WIDTH_BORDER_INTERNAL;
	frame.origin.y += WIDTH_BORDER_INTERNAL;
	
	RakContentView * contentView = [[RakContentView alloc] initWithFrame:frame];
	if(contentView != nil)
		[self addSubview:contentView];
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
