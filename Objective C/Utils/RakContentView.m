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

- (NSRect) getFrame
{
	NSRect frame = [self frame];
	
	if (frame.origin.x != BORDER_WIDTH || frame.origin.y != BORDER_WIDTH)
	{
		[self setFrameOrigin:NSMakePoint(BORDER_WIDTH, BORDER_WIDTH)];
		frame.origin.x = frame.origin.y = BORDER_WIDTH;
		
		frame.size.height -= 2 * BORDER_WIDTH;
		frame.size.width -= 2 * BORDER_WIDTH;
		[self setFrameSize:frame.size];
	}
	
	return frame;
}

@end
