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

@interface RakMDLController : NSObject
{
	MDL* _tabMDL;
	
	MANGAS_DATA * cache;
	uint sizeCache;
	
	THREAD_TYPE coreWorker;
	DATA_LOADED *** todoList;
	uint	* IDToPosition;
	int8_t ** status;
	int8_t ** statusCache;

	uint nbElem;
	uint discardedCount;

	bool quit;
}

- (id) init : (MDL *) tabMDL : (NSString *) state;

- (void) needToQuit;
- (NSString *) serializeData;

- (uint) getNbElem : (BOOL) considerDiscarded;
- (DATA_LOADED **) getData : (uint) row : (BOOL) considerDiscarded;

- (int8_t) statusOfID : (uint) row : (BOOL) considerDiscarded;
- (void) setStatusOfID : (uint) row : (BOOL) considerDiscarded : (uint8_t) value;
- (void) addElement : (MANGAS_DATA) data : (BOOL) isTome : (int) element : (BOOL) partOfBatch;
- (BOOL) checkForCollision : (MANGAS_DATA) data : (BOOL) isTome : (int) element;
- (void) discardElement : (uint) element;

- (void) refreshCT : (uint) ID;

@end
