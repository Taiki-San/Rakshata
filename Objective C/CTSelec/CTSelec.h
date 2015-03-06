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

#import "RakCTDragableTitle.h"
#import "RakCTSelectionList.h"
#import "RakStarView.h"

#import "RakCTSynopsis.h"
#import "RakCTTools.h"
#import "RakCTCoreViewButtons.h"
#import "RakCTHeader.h"

#import "RakCTSerieHeader.h"
#import "RakSRSuggestions.h"
#import "RakSRDetails.h"
#import "RakCTFocusSRItem.h"

#import "RakChapterView.h"

#define CT_TRANSITION_ANIMATION 0.3f
#define CT_HALF_TRANSITION_ANIMATION 0.15f

@interface CTSelec : RakTabView
{
	//SR mode only
	RakCTSerieHeader * SRHeader;
	
	RakBackButton *backButton;
	
	RakChapterView * coreView;
}

- (id)init : (NSView*)contentView : (NSString *) state;

- (void) updateProject : (PROJECT_DATA) project : (BOOL)isTome : (int) element;
- (void) resetTabContent;

- (void) selectElem : (uint) projectID : (BOOL) isTome : (int) element;
- (PROJECT_DATA) activeProject;

@end