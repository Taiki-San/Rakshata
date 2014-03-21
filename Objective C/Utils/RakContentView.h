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

@interface RakContentView : NSView
{
	uint _mainThread;
	uint _stateTabsReader;
	
	Series* _tabSerie;
	CTSelec* _tabCT;
	Reader* _tabReader;
	MDL* _tabMDL;
}

- (void) setupCtx : (Series*) tabSerie : (CTSelec*) tabCT : (Reader*) tabReader : (MDL*) tabMDL;
- (void) updateContext : (uint) mainThread : (uint) stateTabsReader;

- (NSRect) getFrame;

@end
