/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite  **
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
}

- (id)init: (BOOL) displayed : (Reader*) parent;
- (void) setupPath;

- (void) leaveReaderMode;
- (void) startReaderMode;

- (short) numberIconsInBar;
- (void) loadIcons : (Reader*) superView;
- (CGFloat) getPosXElement : (uint) IDButton;
- (void) recalculateElementsPosition;

- (NSColor*) getColorFront;

@end
