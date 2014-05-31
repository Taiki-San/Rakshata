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

#define WIDTH_BORDER_ALL		4
#define WIDTH_BORDER_FAREST		2
#define WIDTH_BORDER_MIDDLE		1
#define WIDTH_BORDER_INTERNAL	1

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

@end

@interface RakContentViewBack : NSView
{
	RakBorder* internalRows1;
	RakBorder* internalRows2;
	
	RakContentView * firstResponder;
}

- (void) setupBorders;
- (RakContentView *) getFirstResponder;

@end

