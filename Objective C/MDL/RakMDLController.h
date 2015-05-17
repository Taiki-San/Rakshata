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

#define MDLCTRL_getDataFull(data, index, isTome) (isTome ? (data.tomesFull[index].ID) : data.chapitresFull[index])
#define MDLCTRL_getDataInstalled(data, index, isTome) (isTome ? (data.tomesInstalled[index].ID) : data.chapitresInstalled[index])

@interface RakMDLController : NSObject
{
	MDL* _tabMDL;
	
	PROJECT_DATA ** cache;
	uint sizeCache;
	
	THREAD_TYPE coreWorker;
	DATA_LOADED *** todoList;
	uint	* IDToPosition;
	int8_t ** status;

	uint nbElem;
	uint discardedCount;

	bool quit;
	
	//Credential request
	BOOL requestForPurchase;
}

@property RakMDLList * __weak list;
@property BOOL requestCredentials;
@property (readonly) BOOL isSerieMainThread;

- (instancetype) init : (MDL *) tabMDL : (NSString *) state;

- (void) needToQuit;
- (NSString *) serializeData;

- (uint) getNbElem : (BOOL) considerDiscarded;
- (uint) convertRowToPos : (uint) row;
- (DATA_LOADED **) getData : (uint) row : (BOOL) considerDiscarded;

- (int8_t) statusOfID : (uint) row : (BOOL) considerDiscarded;
- (void) discardInstalled;
- (void) setStatusOfID : (uint) row : (BOOL) considerDiscarded : (int8_t) value;
- (void) removingEmailAddress;
- (void) addElement : (PROJECT_DATA) data : (BOOL) isTome : (int) element : (BOOL) partOfBatch;
- (uint) addBatch : (PROJECT_DATA) data : (BOOL) isTome : (BOOL) launchAtTheEnd;
- (void) reorderElements : (uint) posStart : (uint) posEnd : (uint) injectionPoint;
- (BOOL) checkForCollision : (PROJECT_DATA) data : (BOOL) isTome : (int) element;
- (void) discardElement : (uint) element withSimilar: (BOOL) similar;

- (BOOL) areCredentialsComplete;
- (RakTabForegroundView *) getForegroundView;
- (void) setWaitingLogin : (NSNumber *) request;

- (void) collapseStateUpdate : (BOOL) wantCollapse;

@end
