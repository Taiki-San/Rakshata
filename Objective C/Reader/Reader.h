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

@class RakReaderBottomBar;

@interface Reader : RakTabView
{
	bool initialized;
	bool pageInitialized;
	
	uint gonnaReduceTabs;
	RakReaderBottomBar * bottomBar;
	
	//Page management
	NSData *_prevPage;
	NSData *_pageData;
	NSImage* _page;
	NSData *_nextPage;
	
	NSImageView * _pageView;
	NSScrollView * _scrollView;
	
	NSRect _scrollViewFrame;
	NSRect _contentFrame;
	
	BOOL _pageTooLarge;
	BOOL _pageTooHigh;
	BOOL _areSlidersHidden;
	
	/*context data*/
@private
	bool _readerMode;
	bool _alreadyRefreshed;
	bool _dontGiveACrapAboutCTPosUpdate;
	
	bool _cacheBeingBuilt;
	
	PROJECT_DATA _project;
	DATA_LECTURE _data;
	
	int _currentElem;
	int _posElemInStructure;
	bool _isTome;
}

- (id)init : (NSView*)contentView : (NSString *) state;
- (void) initReaderMainView : (NSString *) state;

- (void) startReading : (PROJECT_DATA) project : (int) elemToRead : (bool) isTome : (uint) startPage;
- (void) willLeaveReader;
- (void) willOpenReader;

- (void) collapseAllTabs : (bool) forced;
- (void) hideBothTab;
- (void) unhideBothTab;
- (void) hideCursor;

- (void) switchFavs;
- (void) triggerFullscreen;
- (void) updatePage : (uint) newCurrentPage : (uint) newPageMax;

@end

#import "RakReaderControllerUIQuery.h"
#import "RakPageCounter.h"
#import "RakReaderBottomBar.h"
#import "RakPage.h"