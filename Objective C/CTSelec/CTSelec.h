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

#import "RakCTSelectionList.h"
#import "RakCTTools.h"
#import "RakCTCoreViewButtons.h"
#import "RakCTHeaderImage.h"
#import "RakCTProjectSynopsis.h"
#import "RakChapterView.h"

#define CT_TRANSITION_ANIMATION 0.2f
#define CT_HALF_TRANSITION_ANIMATION 0.1f

@interface CTSelec : RakTabView
{
	RakBackButton *backButton;
	RakChapterView * coreView;
}

- (id)init : (NSView*)contentView : (NSString *) state;

- (NSRect) calculateContentViewSize : (NSRect) frame : (CGFloat) backButtonLowestY;

- (void) updateProject : (PROJECT_DATA) project : (BOOL)isTome : (int) element;

- (BOOL) refreshCT : (BOOL) checkIfRequired : (uint) ID;
- (void) selectElem : (uint) projectID : (BOOL) isTome : (int) element;

@end