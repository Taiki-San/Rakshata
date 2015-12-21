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

@interface RakContentView : RakView
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
- (void) cleanCtx;

@end

@interface RakContentViewBack : RakView
{
	RakColor * backgroundColor;
	
	RakBorder* internalRows1, * internalRows2;
	
	RakContentView * firstResponder;

	RakText * titleView;
}

//Used when leaving fullscreen mode
@property (nonatomic) CGFloat heightOffset;
@property (nonatomic) NSString * title;
@property (nonatomic) BOOL isMainWindow;

- (void) setupBorders;
- (void) updateUI;
- (RakContentView *) getFirstResponder;

- (NSRect) internalFrame;

@end

