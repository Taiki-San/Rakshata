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
	RakCTDragableTitle * _title;
	RakCTSelectionList * _content;
	RakText * _placeholder;
	
	BOOL _placeholderActive;
	BOOL _wasHidden;
}

@property (nonatomic) BOOL compactMode;
@property (readonly) uint nbElem;

- (instancetype) initWithFrame : (NSRect) parentFrame : (BOOL) isCompact : (RakCTSelectionList*) content;

- (void) resizeAnimation : (NSRect) parentFrame;

//Proxy
- (NSScrollView*) getContent;

- (uint) getSelectedElement;
- (float) getSliderPos;

- (BOOL) reloadData : (PROJECT_DATA) project : (BOOL) resetScroller;

- (NSInteger) getIndexOfElement : (uint) element;
- (void) selectIndex : (uint) index;
- (void) jumpScrollerToIndex : (uint) index;

- (void) resetSelection;

- (void) setWantIsTome : (BOOL) isTome;

@end
