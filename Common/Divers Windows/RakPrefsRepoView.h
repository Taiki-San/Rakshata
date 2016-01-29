/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at https://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
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
- (void) removeContentAtLine :(uint) line;
- (void) updateContentAtLine :(uint) line;

@end

#import "RakListItemView.h"

@interface RakPrefsRepoListItemView : RakListItemView
{
	RakSwitchButton * activationButton;
	
	REPO_DATA * _repoUsedInDetail;
}

@property BOOL isRoot;
@property id __weak responder;

@property BOOL wantActivationState;

- (instancetype) initWithRepo : (BOOL) isCompact : (BOOL) isDetailColumn : (BOOL) isRoot : (void *) repo : (NSString *) detailString;
- (void) updateContent : (BOOL) isCompact : (BOOL) isDetailColumn : (BOOL) isRoot : (void *) repo : (NSString *) detailString;

- (BOOL) getButtonState;
- (void) refreshButtonState;
- (void) cancelSelection;

@end

@interface RakPrefsRepoDetails : RakView

- (instancetype) initWithRepo : (NSRect) frame : (BOOL) isRoot : (void *) repo : (RakPrefsRepoView *) responder;
- (void) updateContent : (BOOL) isRoot : (void *) repo : (BOOL) animated;

- (void) statusTriggered : (id) responder : (REPO_DATA *) repoData;

@end

@interface RakPrefsRepoView : RakView
{
	ROOT_REPO_DATA ** root;
	REPO_DATA ** repo;
	
	uint nbRoot, nbRepo;
	uint activeElementInRoot, activeElementInSubRepo;
	BOOL _selectedIsRoot;
	
	RakPrefsRepoList * list;
	RakPrefsRepoDetails * details;
	
	RakSwitchButton * radioSwitch;
	RakText * switchMessage, * placeholder;	
}

- (void *) dataForMode : (BOOL) rootMode index : (uint) index;
- (uint) sizeForMode : (BOOL) rootMode;

- (NSString *) nameOfParent : (uint) parentID;
- (uint) posOfParent : (uint) parentID;

- (void) selectionUpdate : (BOOL) isRoot : (uint) index;

@end
