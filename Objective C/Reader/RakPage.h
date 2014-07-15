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
{
	PROJECT_DATA * data;
	uint nbElem;
}

- (void) setData : (PROJECT_DATA *) newData;
- (PROJECT_DATA *) data;
- (void) setNbElem : (uint) newData;
- (uint) nbElem;

@end

@interface Reader (PageManagement)

- (BOOL) initPage : (PROJECT_DATA) dataRequest : (int) elemRequest : (BOOL) isTomeRequest : (int) startPage;
- (NSString *) getContextToGTFO;

- (void) initialPositionning;
- (void) setFrameInternal : (NSRect) frameRect : (BOOL) isAnimated;

- (void) leaveReaderMode;
- (void) startReaderMode;

- (void) failure;

- (void) nextPage;
- (void) prevPage;
- (void) nextChapter;
- (void) prevChapter;
- (void) moveSliderX : (int) move;
- (void) moveSliderY : (int) move;
- (void) setSliderPos : (NSPoint) newPos;

- (BOOL) initialLoading : (PROJECT_DATA) dataRequest : (int) elemRequest : (BOOL) isTomeRequest : (int) startPage;
- (void) buildCache;
- (void) changePage : (byte) switchType;
- (void) jumpToPage : (uint) newPage;
- (void) changeChapter : (bool) goToNext;
- (void) changeProject : (PROJECT_DATA) projectRequest : (int) elemRequest : (bool) isTomeRequest : (int) startPage;
- (void) updateCT : (uint) request;
- (void) updateContext;
- (BOOL) craftPageAndSetupEnv : (byte) switchType;
- (void) deleteElement;
- (void) addPageToView;
- (void) updateScrollerAfterResize;

- (void) checkIfNewElements;
- (void) promptToGetNewElems : (RakArgumentToRefreshAlert *) arguments;

- (void) flushCache;
- (void) getTheFuckOut;

@end
