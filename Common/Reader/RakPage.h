/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
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

@interface RakArgumentToRefreshAlert : NSObject

@property PROJECT_DATA * data;
@property uint nbElem;

@end

@interface Reader (PageManagement) <NSPageControllerDelegate>

- (BOOL) initPage : (PROJECT_DATA) dataRequest : (uint) elemRequest : (BOOL) isTomeRequest : (uint) startPage;
- (STATE_DUMP) exportContext;

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
- (void) addPageToView : (RakImageView *) page : (RakPageScrollView *) scrollView;
- (void) buildCache : (NSNumber *) session;
- (void) updatePCState : (uint) page : (uint) currentCacheSession : (RakView *) view;

- (void) updateScrollerAfterResize : (RakPageScrollView *) scrollView : (NSSize) previousSize;
- (void) updateProjectReadingOrder;

- (void) checkIfNewElements;
- (void) promptToGetNewElems : (RakArgumentToRefreshAlert *) arguments;

@end
