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

@interface RakCTSelectionListContainer : NSView
{
	RakMenuText * _title;
	RakCTSelectionList * _content;
	
	BOOL _isCompact;
	BOOL _wasHidden;
}

@property BOOL compactMode;

- (instancetype) initWithFrame : (NSRect) parentFrame : (BOOL) isCompact : (RakCTSelectionList*) content;

- (void) resizeAnimation : (NSRect) parentFrame;

//Proxy
- (NSScrollView*) getContent;

- (NSInteger) getSelectedElement;
- (float) getSliderPos;

- (BOOL) reloadData : (PROJECT_DATA) project : (int) nbElem : (void *) newData : (BOOL) resetScroller;

- (NSInteger) getIndexOfElement : (NSInteger) element;
- (void) selectRow : (int) row;
- (void) jumpScrollerToRow : (int) row;

- (void) resetSelection : (NSTableView *) tableView;

@end
