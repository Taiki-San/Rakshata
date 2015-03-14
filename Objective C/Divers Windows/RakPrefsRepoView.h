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

#define PREFS_REPO_LIST_WIDTH 500
#define PREFS_REPO_BORDER_BELOW_LIST 30

@class RakPrefsRepoView;

@interface RakPrefsRepoList : RakList

@property id __weak responder;
@property (nonatomic) BOOL rootMode;
@property BOOL detailMode;

- (instancetype) initWithFrame : (NSRect) frame;
- (void) reloadContent : (BOOL) rootMode;

@end

@interface RakPrefsRepoListItem : NSView

- (void) cancelSelection;

@end

@interface RakPrefsRepoDetails : NSView


- (instancetype) initWithRepo : (NSRect) frame : (BOOL) isRoot : (void *) repo : (RakPrefsRepoView *) responder;
- (void) updateContent : (BOOL) isRoot : (void *) repo : (BOOL) animated;

- (void) nukeEverything : (id) responder;

@end

@interface RakPrefsRepoView : NSView
{
	ROOT_REPO_DATA ** root;
	REPO_DATA ** repo;
	
	uint nbRoot, nbRepo;
	uint activeElementInRoot, activeElementInSubRepo;
	
	RakPrefsRepoList * list;
	RakPrefsRepoDetails * details;
	
	RakRadioButton * radioSwitch;
	RakText * switchMessage;
}

- (void *) dataForMode : (BOOL) rootMode index : (uint) index;
- (uint) sizeForMode : (BOOL) rootMode;

- (NSString *) nameOfParent : (uint) parentID;
- (uint) posOfParent : (uint) parentID;

- (void) selectionUpdate : (BOOL) isRoot : (uint) index;

@end
