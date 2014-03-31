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
 *********************************************************************************************/

@interface RakCTCoreContentView : NSObject <NSTableViewDelegate, NSTableViewDataSource>
{
	bool isTome;
	void* data;
	uint amountData;
	NSScrollView * scrollView;
	NSTableView * _tableView;
	
	//Color cache
	NSColor * normal;
	NSColor * highlight;
}

- (id) init : (NSRect) frame : (MANGAS_DATA) project : (bool) isTomeRequest : (long) elemSelected : (long) scrollerPosition;
- (void) failure;
- (void) setSuperView : (NSView *) superview;
- (bool) reloadData : (int) nbElem : (void *) newData;
- (void) setHidden : (bool) state;
- (void) setFrame : (NSRect) frameRect;

- (NSRect) getTableViewFrame : (NSRect) superViewFrame;

- (NSInteger) getSelectedElement;
- (float) getSliderPos;

- (NSColor *) getTextColor;
- (NSColor *) getTextHighlightColor;
- (NSColor *) getBackgroundHighlightColor;

@end

@interface RakCTScrollView : NSScrollView

@end