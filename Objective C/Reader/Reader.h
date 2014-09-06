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
	
	//Page management
	RakPageScrollView * _prevScrollView;
	RakPageScrollView * _scrollView;
	RakPageScrollView * _nextScrollView;
	
	NSPageController * mainScroller;
	NSImage * loadingPlaceholder;
	NSImage * loadingFailedPlaceholder;
	
	MUTEX_VAR cacheMutex;
	uint cacheSession;
	byte previousMove;
	
	BOOL distractionFree;
	
	//Context data
@private
	
	bool _readerMode;
	bool _alreadyRefreshed;
	bool _dontGiveACrapAboutCTPosUpdate;
	bool _flushingCache;
	
	bool _cacheBeingBuilt;
	
	PROJECT_DATA _project;
	DATA_LECTURE _data;
	
	int _currentElem;
	int _posElemInStructure;
}

@property BOOL isTome;

- (id)init : (NSView*)contentView : (NSString *) state;
- (void) initReaderMainView : (NSString *) state;

- (void) startReading : (PROJECT_DATA) project : (int) elemToRead : (bool) isTome : (uint) startPage;
- (void) willLeaveReader;
- (void) willOpenReader;

- (void) collapseAllTabs : (bool) forced;
- (void) hideBothTab;
- (void) unhideBothTab;
- (void) hideCursor;

- (void) switchDistractionFree;
- (void) shouldLeaveDistractionFreeMode;

- (void) switchFavs;
- (void) triggerFullscreen;
- (void) updatePage : (uint) newCurrentPage : (uint) newPageMax;

@end

#import "RakReaderControllerUIQuery.h"
#import "RakPageCounter.h"
#import "RakFavsInfo.h"
#import "RakDeleteConfirm.h"
#import "RakReaderBottomBar.h"
#import "RakPage.h"