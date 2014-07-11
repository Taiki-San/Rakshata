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

@interface RakReaderBottomBar : NSView
{
	bool readerMode;
	
	/** Images of different icons **/
	RakButton *favorite;
	RakButton *fullscreen;
	
	RakButton *prevChapter;
	RakButton *prevPage;
	RakButton *nextPage;
	RakButton *nextChapter;
	
	RakButton *trash;
	
	RakPageCounter * pageCount;
	
	CGContextRef contextBorder;
	
	bool isFaved;
}

- (id)init: (BOOL) displayed : (Reader*) parent;
- (void) setupPath;

- (void) updatePage : (uint) newCurrentPage : (uint) newPageMax;

- (void) leaveReaderMode;
- (void) startReaderMode;

- (void) releaseIcons;

- (short) numberIconsInBar;
- (void) loadIcons : (Reader*) superView;
- (void) favsUpdated : (BOOL) isNewStatedFaved;
- (CGFloat) getPosXElement : (uint) IDButton : (CGFloat) width;

- (void) resizeAnimation : (NSRect) frameRect;
- (void) recalculateElementsPosition : (BOOL) isAnimated : (CGFloat) newWidth;

- (NSColor*) getColorFront;

@end
