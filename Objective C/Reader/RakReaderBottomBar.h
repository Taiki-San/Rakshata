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

@interface RakReaderBBButton : RakButton

@property BOOL popoverOpened;

- (void) removePopover;

@end

#define READER_DELAY_CURSOR_FADE	2
#define READER_BB_ALPHA_DF			0.5f
#define READER_BB_ALPHA_DF_STATIC	0.05f

@interface RakReaderBottomBar : NSView
{
	/** Images of different icons **/
	RakButton *favorite;
	RakButton *fullscreen;
	
	RakButton *prevChapter;
	RakButton *prevPage;
	RakButton *nextPage;
	RakButton *nextChapter;
	
	RakReaderBBButton *trash;
	
	RakPageCounter * pageCount;
	
	CGContextRef contextBorder;
	
	bool isFaved;
	
	//Used for distraction free mode
	Reader * _parent;
	NSTrackingArea * trackingArea;
}

@property BOOL readerMode;
@property BOOL highjackedMouseEvents;

- (id)init: (BOOL) displayed : (Reader*) parent;
- (void) setupPath;

- (void) updatePage : (uint) newCurrentPage : (uint) newPageMax;

- (void) releaseIcons;

- (short) numberIconsInBar;
- (void) loadIcons : (Reader*) superview;
- (void) favsUpdated : (BOOL) isNewStatedFaved;
- (CGFloat) getPosXElement : (uint) IDButton : (CGFloat) width;

- (void) resizeAnimation : (NSRect) frameRect;
- (void) recalculateElementsPosition : (BOOL) isAnimated : (CGFloat) newWidth;

- (NSColor*) getColorFront;

@end
