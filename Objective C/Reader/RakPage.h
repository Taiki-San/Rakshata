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

@interface Reader (PageManagement) <NSPageControllerDelegate>

- (BOOL) initPage : (PROJECT_DATA) dataRequest : (int) elemRequest : (BOOL) isTomeRequest : (int) startPage;
- (NSString *) getContextToGTFO;

- (void) initialPositionning : (RakPageScrollView *) scrollView;
- (void) setFrameInternal : (NSRect) frameRect : (BOOL) isAnimated;

- (void) failure;

- (void) nextPage;
- (void) prevPage;
- (void) nextChapter;
- (void) prevChapter;
- (BOOL) moveSliderX : (int) move;
- (BOOL) moveSliderY : (int) move;
- (void) setSliderPos : (NSPoint) newPos;

- (BOOL) initialLoading : (PROJECT_DATA) dataRequest : (int) elemRequest : (BOOL) isTomeRequest : (int) startPage;
- (void) changePage : (byte) switchType;
- (void) jumpToPage : (uint) newPage;
- (void) changeChapter : (bool) goToNext;
- (void) changeProject : (PROJECT_DATA) projectRequest : (int) elemRequest : (bool) isTomeRequest : (int) startPage;
- (void) updateCT : (uint) request;
- (void) updateContext : (BOOL) dataAlreadyLoaded;
- (void) updateEvnt;
- (void) deleteElement;

- (RakPageScrollView *) getScrollView : (uint) page : (DATA_LECTURE*) data;
- (void) addPageToView : (NSImage *) page : (RakPageScrollView *) scrollView;
- (void) buildCache : (NSNumber *) session;
- (void) updatePCState : (NSMutableArray **) data : (uint) page : (NSView *) view;

- (void) updateScrollerAfterResize : (RakPageScrollView *) scrollView : (NSSize) previousSize;

- (void) checkIfNewElements;
- (void) promptToGetNewElems : (RakArgumentToRefreshAlert *) arguments;

- (void) flushCache;
- (void) deallocInternal;

@end
