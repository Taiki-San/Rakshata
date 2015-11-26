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

#define NOTIFICATION_SEARCH_UPDATED	@"RakNotificationSearchUpdated"
#define NOTIFICATION_INSTALLED_ONLY_STAB @"RakNotificationInstalledOnlyInSerieMode"
#define NOTIFICATION_FREE_ONLY	@"RakNotificationFreeOnlyInSerieMode"
#define NOTIFICATION_FAVS_ONLY	@"RakNotificationFavsOnlyInSerieMode"

@interface RakSRStupidDataStructure : NSObject

@property uint index;

@end

@interface RakSRContentManager : NSObject
{
	BOOL _initialized, secondPass;
	
	NSString * commitedSearch;
	
	//Model
	BOOL installedOnly, freeOnly, favsOnly;
	PROJECT_DATA * project;
	uint nbElemFull, nbElemActivated;
	
	uint * cacheList;
	
	uint * orderedToSorted;
	uint * filteredToOrdered;
	
	//Views
	NSRect previousFrame;
	byte _activeView;

	RakGridView * _grid;
}

@property (weak) NSView * controlView;
@property byte activeView;

@property (readonly) NSView * getActiveView;
@property NSMutableArray * sharedReference;

- (void) initViews;

- (void) setFrame : (NSRect) frame;
- (void) resizeAnimation : (NSRect) frame;

- (uint) nbElement;
- (uint) nbActivatedElement;
- (PROJECT_DATA *) getDataAtIndex : (uint) index;

@end
