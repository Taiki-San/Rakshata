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

- (id) initWithCoder:(NSCoder *)aDecoder
{
	self =  [super initWithCoder:aDecoder];
	return self;
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

- (NSRect) getFrame
{
	NSRect frame = [self frame];
	frame.origin.x += 4;
	frame.origin.y += 4;
	frame.size.height -= 2 * 4;
	frame.size.width -= 2 * 4;
	return frame;
}

@end
