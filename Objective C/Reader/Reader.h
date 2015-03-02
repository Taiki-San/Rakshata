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
@class RakReaderBottomBar;

@interface Reader : RakTabView
{
	bool initialized;
	bool pageInitialized;
	
	uint gonnaReduceTabs;
	NSView * container;
	RakReaderBottomBar * bottomBar;
	RakReaderControllerUIQuery * newStuffsQuery;
	
	//Page management
	RakPageScrollView * _scrollView;
	
	NSPageController * mainScroller;
	NSImage * loadingPlaceholder;
	NSImage * loadingFailedPlaceholder;
	
	MUTEX_VAR cacheMutex;
	uint cacheSession;
	byte previousMove;
	
	BOOL bottomBarHidden;
	NSTimer * delaySinceLastMove;
	NSPoint cursorPosBeforeLastMove;
	
	//Context data
	bool _readerMode;
	bool _alreadyRefreshed;
	bool _dontGiveACrapAboutCTPosUpdate;
	bool _flushingCache;
	
	bool _cacheBeingBuilt;
	
	PROJECT_DATA _project;
	BOOL dataLoaded;
	DATA_LECTURE _data;
	
	int _currentElem;
	int _posElemInStructure;
	
	//Cache data
	BOOL previousDataLoaded;
	DATA_LECTURE _previousData;
	BOOL nextDataLoaded;
	DATA_LECTURE _nextData;
}

@property BOOL distractionFree;
@property BOOL isTome;
@property BOOL preventRecursion;

- (id)init : (NSView*)contentView : (NSString *) state;
- (void) initReaderMainView : (NSString *) state;

- (void) startReading : (PROJECT_DATA) project : (int) elemToRead : (bool) isTome : (uint) startPage;
- (void) willLeaveReader;
- (void) willOpenReader;

- (void) collapseAllTabs : (bool) forced;
- (void) hideBothTab;
- (void) unhideBothTab;

- (void) switchDistractionFree;
- (void) shouldLeaveDistractionFreeMode;
- (void) startFadeTimer : (NSPoint) cursorPosition;
- (void) abordFadeTimer;
- (void) cursorShouldFadeAway;
- (void) fadeBottomBar : (CGFloat) alpha;

- (void) switchFavs;
- (void) triggerFullscreen;
- (void) updatePage : (uint) newCurrentPage : (uint) newPageMax;
- (void) updateTitleBar : (PROJECT_DATA) project : (BOOL) isTome : (uint) position;

@end

#import "RakReaderControllerUIQuery.h"
#import "RakPageCounter.h"
#import "RakFavsInfo.h"
#import "RakDeleteConfirm.h"
#import "RakReaderBottomBar.h"
#import "RakPage.h"

BOOL preventWindowCaptureForWindow(NSWindow *window);