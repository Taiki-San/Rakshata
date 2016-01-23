/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

#import "RakCTDragableTitle.h"
#import "RakCTSelectionList.h"
#import "RakStarView.h"

#import "RakSynopsis.h"
#import "RakCTTools.h"
#import "RakCTHeader.h"

#import "RakCTSerieHeader.h"
#import "RakSRSuggestions.h"
#import "RakSRDetails.h"
#import "RakCTFocusSRItem.h"

#import "RakChapterView.h"

#define CT_TRANSITION_ANIMATION 0.3f
#define CT_HALF_TRANSITION_ANIMATION 0.15f

#define BORDER_CT_FOCUS 10

@interface CTSelec : RakTabView
{
	//SR mode only
	RakCTSerieHeader * SRHeader;
	
	RakBackButton *backButton;
	
	RakChapterView * coreView;
}

- (instancetype) init : (RakView *) contentView : (NSString *) state;

- (void) updateProject : (uint) cacheDBID : (BOOL)isTome : (uint) element;
- (void) resetTabContent;

- (void) selectElem : (uint) projectID : (BOOL) isTome : (uint) element;
- (PROJECT_DATA) activeProject;

@end

enum
{
	TITLE_BALANCING_OFFSET = 10
};