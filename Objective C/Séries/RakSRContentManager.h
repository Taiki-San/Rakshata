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
 ********************************************************************************************/

@interface RakSRContentManager : NSObject
{
	BOOL _initialized;
	
	//Model
	PROJECT_DATA * project;
	uint nbElemFull, nbElemActivated;
	
	uint * cacheList;
	
	BOOL * activatedList;
	uint * filteredToSorted;
	
	//Views
	NSRect previousFrame;
	BOOL _hidden;
	byte _activeView;

	RakGridView * _grid;
}

@property (weak) NSView * controlView;
@property byte activeView;

@property BOOL hidden;
@property NSMutableArray * sharedReference;

- (void) initViews;

- (void) setFrame : (NSRect) frame;
- (void) resizeAnimation : (NSRect) frame;

- (uint) nbElement;
- (PROJECT_DATA *) getDataAtIndex : (uint) index;

@end
