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
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 ********************************************************************************************/

#if !TARGET_OS_IPHONE
#import "RakPageController.h"

@class RakReaderBottomBar;
#endif

@interface Reader : RakReaderCore
{
	uint gonnaReduceTabs;
	RakView * container;
#if !TARGET_OS_IPHONE
	RakReaderBottomBar * bottomBar;
	RakReaderControllerUIQuery * newStuffsQuery;
#endif
	
	BOOL queryHidden;
	uint * _queryArrayData;
	size_t _queryArraySize;
	
	//Page management
	BOOL saveMagnification, overrideDirection;
	CGFloat lastKnownMagnification;

	RakPageScrollView * _scrollView;
	
#if !TARGET_OS_IPHONE
	RakPageController * mainScroller;
#endif
	RakImage * loadingPlaceholder;
	RakImage * loadingFailedPlaceholder;
	
	uint workingCacheSession;
	byte previousMove;
	
	BOOL bottomBarHidden, oldDFState;
	NSTimer * delaySinceLastMove;
	NSPoint cursorPosBeforeLastMove;
	
	//Context data
	BOOL _readerMode;
	BOOL _alreadyRefreshed;
	BOOL _dontGiveACrapAboutCTPosUpdate;
	BOOL _flushingCache;
	
	BOOL _haveScrollerPosToCommit;
	NSPoint _scrollerPosToCommit;
	BOOL _endingTransition;
	
	BOOL dataLoaded;
	DATA_LECTURE _data;
		
	//Cache data
	BOOL previousDataLoaded;
	DATA_LECTURE _previousData;
	BOOL nextDataLoaded;
	DATA_LECTURE _nextData;
}

@property BOOL distractionFree;
@property BOOL preventRecursion;

- (instancetype) init : (RakView*)contentView : (NSString *) state;
- (void) initReaderMainView : (NSString *) state;

- (void) resetReader;

- (void) willLeaveReader;
- (void) willOpenReader;

- (void) collapseAllTabs : (BOOL) forced;
- (void) hideBothTab;
- (void) unhideBothTab;

- (void) switchDistractionFree;
- (void) shouldLeaveDistractionFreeMode;
- (void) startFadeTimer : (NSPoint) cursorPosition;
- (void) abortFadeTimer;
- (void) cursorShouldFadeAway;
- (void) fadeBottomBar : (CGFloat) alpha;

- (void) switchFavs;
- (void) triggerFullscreen;

- (void) updatePage : (uint) newCurrentPage : (uint) newPageMax;
- (void) updateTitleBar : (PROJECT_DATA) project : (BOOL) isTome : (uint) position;

@end

#if !TARGET_OS_IPHONE
#import "RakReaderSuggestions.h"
#import "RakReaderControllerUIQuery.h"
#import "RakPageCounter.h"
#import "RakFavsInfo.h"
#import "RakDeleteConfirm.h"
#import "RakReaderBottomBar.h"
#import "RakPage.h"

BOOL preventWindowCaptureForWindow(NSWindow *window);
#endif


enum
{
	RD_CONTROLBAR_HEIGHT = 26,
	RD_CONTROLBAR_WIDHT_PERC = 75,
	RD_CONTROLBAR_WIDHT_MIN = 500,
	RD_CONTROLBAR_WIDHT_MAX	= 1000,
	RD_CONTROLBAR_POSY = 20,
	
	READER_BORDURE_VERT_PAGE = 10,
	READER_PAGE_BOTTOM_BORDER = (RD_CONTROLBAR_HEIGHT + RD_CONTROLBAR_POSY + 10),
	READER_PAGE_BORDERS_HIGH = (READER_PAGE_TOP_BORDER + READER_PAGE_BOTTOM_BORDER),
};
