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

#import "RakCTCoreContentView.h"
#import "RakCTTools.h"
#import "RakCTCoreViewButtons.h"
#import "RakCTHeader.h"
#import "RakChapterView.h"

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