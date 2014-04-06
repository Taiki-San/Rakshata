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
 ********************************************************************************************/

@class RakReaderBottomBar;
@class RakPage;

@interface Reader : RakTabView
{
	bool initialized;
	
	uint gonnaReduceTabs;
	RakReaderBottomBar * bottomBar;
	RakPage * mainImage;
}

- (id)init : (NSView*)contentView : (NSString *) state;
- (void) initReaderMainView : (NSString *) state;

- (void) startReading : (MANGAS_DATA) project : (int) elemToRead : (bool) isTome : (uint) startPage;
- (void) willLeaveReader;
- (void) willOpenReader;

- (void) collapseAllTabs : (bool) forced;
- (void) hideBothTab;
- (void) unhideBothTab;
- (void) hideCursor;

- (void) prevPage;
- (void) nextPage;
- (void) prevChapter;
- (void) nextChapter;
- (void) triggerFullscreen;
- (void) deleteElement;

@end

#import "RakPageCounter.h"
#import "RakReaderBottomBar.h"
#import "RakPage.h"