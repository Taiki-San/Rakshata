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

@interface RakMDLController : NSObject
{
	MANGAS_DATA * cache;
	
	THREAD_TYPE coreWorker;
	DATA_LOADED *** todoList;
	uint	* IDToPosition;
	int8_t ** status;
	int8_t ** statusCache;

	uint nbElem;
	uint discardedCount;

	bool quit;
}

- (void) needToQuit;

- (uint) getNbElem : (BOOL) considerDiscarded;
- (DATA_LOADED **) getData : (uint) row : (BOOL) considerDiscarded;
- (void) discardElement : (uint) element;

- (int8_t) statusOfID : (uint) row;
- (void) setStatusOfID : (uint) row : (uint8_t) value;

@end
