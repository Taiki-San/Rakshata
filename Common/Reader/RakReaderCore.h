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

#include "lecteur.h"
#import "RakPageScrollView.h"

#define READER_MAGNIFICATION_MIN	0.25
#define READER_MAGNIFICATION_MAX	3.0
#define READER_PAGE_TOP_BORDER		50

@interface RakReaderCore : RakTabView
{
	BOOL initialized;
	
	MUTEX_VAR cacheMutex;
	
	PROJECT_DATA _project;
	uint _currentElem;
	uint _posElemInStructure;
}

@property BOOL isTome;

- (void) sharedInit;
- (void) deallocProcessing;
- (void) flushCache;

- (void) restoreProject : (PROJECT_DATA) project withInsertionPoint : (NSDictionary *) insertionPoint;
- (BOOL) startReading : (PROJECT_DATA) project : (uint) elemToRead : (BOOL) isTome : (uint) startPage;

- (void) preProcessStateRestoration : (STATE_DUMP) savedState project : (PROJECT_DATA) project;
- (void) postProcessStateRestoration : (STATE_DUMP) savedState;

- (BOOL) initPage : (PROJECT_DATA) dataRequest : (uint) elemRequest : (BOOL) isTomeRequest : (uint) startPage;
- (BOOL) initialLoading : (PROJECT_DATA) dataRequest : (uint) elemRequest : (BOOL) isTomeRequest : (uint) startPage;
- (void) changeProject : (PROJECT_DATA) projectRequest : (uint) elemRequest : (BOOL) isTomeRequest : (uint) startPage;
- (void) DBUpdated : (NSNotification*) notification;
- (void) postProcessingDBUpdated;

- (RakImageView *) getImage : (uint) page : (DATA_LECTURE*) data : (BOOL *) isPDF;

- (PROJECT_DATA) activeProject;
- (uint) currentElem;
- (void) preProcessingUpdateContext : (PROJECT_DATA) project : (BOOL) isTome;

@end
