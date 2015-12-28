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

@interface RakArgumentToRefreshAlert : NSObject

@property PROJECT_DATA * data;
@property uint nbElem;

@end

#if TARGET_OS_IPHONE
@interface Reader (PageManagement)
#else
@interface Reader (PageManagement) <NSPageControllerDelegate>
#endif

- (BOOL) initPage : (PROJECT_DATA) dataRequest : (uint) elemRequest : (BOOL) isTomeRequest : (uint) startPage;
- (NSString *) getContextToGTFO;
- (STATE_DUMP) exportContext;

- (void) initialPositionning : (RakPageScrollView *) scrollView;
#if !TARGET_OS_IPHONE
- (void) setFrameInternal : (NSRect) frameRect : (BOOL) isAnimated;
#endif

- (void) failure;

- (void) DBUpdated : (NSNotification*) notification;

- (void) nextPage;
- (void) prevPage;
- (void) nextChapter;
- (void) prevChapter;
- (BOOL) moveSliderX : (int) move;
- (BOOL) moveSliderY : (int) move;
- (void) setSliderPos : (NSPoint) newPos;
- (void) commitSliderPosIfNeeded;

- (BOOL) initialLoading : (PROJECT_DATA) dataRequest : (uint) elemRequest : (BOOL) isTomeRequest : (uint) startPage;
- (BOOL) changePage : (byte) switchType;
- (BOOL) changePage : (byte) switchType : (BOOL) animated;
- (void) jumpToPage : (uint) newPage;
- (BOOL) changeChapter : (BOOL) goToNext : (BOOL) byChangingPage;
- (void) changeProject : (PROJECT_DATA) projectRequest : (uint) elemRequest : (BOOL) isTomeRequest : (uint) startPage;
- (void) updateCTTab : (BOOL) shouldOverwriteActiveProject;
- (void) updateContext : (BOOL) dataAlreadyLoaded;
- (void) updateEvnt;
- (void) deleteElement;

- (RakPageScrollView *) getScrollView : (uint) page : (DATA_LECTURE*) data;
- (void) addPageToView : (RakImage *) page : (RakPageScrollView *) scrollView;
- (void) buildCache : (NSNumber *) session;
- (void) updatePCState : (uint) page : (uint) currentCacheSession : (RakView *) view;

#if !TARGET_OS_IPHONE
- (void) updateScrollerAfterResize : (RakPageScrollView *) scrollView : (NSSize) previousSize;
- (void) updateProjectReadingOrder;
#endif

#if !TARGET_OS_IPHONE
- (void) checkIfNewElements;
- (void) promptToGetNewElems : (RakArgumentToRefreshAlert *) arguments;
#endif

- (void) flushCache;
- (void) deallocInternal;

@end
