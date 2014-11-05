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
 *********************************************************************************************/

@interface RakListScrollView : NSScrollView
{
	NSView* documentViewToResize;
}

@property BOOL horizontalScrollingEnabled;
@property BOOL scrollingDisabled;

- (void) resizeAnimation : (NSRect) frameRect;
- (void) updateScrollerState : (NSRect) frame;

@end

@interface RakTableView : NSTableView

- (NSColor *) _dropHighlightColor;

@end

typedef struct smartReload_data
{
	uint data;
	BOOL installed;
	
} SR_DATA;

@interface RakList : RakDragResponder <NSTableViewDelegate, NSTableViewDataSource>
{
	void* _data;
	uint _nbData;
	RakListScrollView * scrollView;
	RakTableView * _tableView;
	
	NSInteger selectedIndex;
	
	//Color cache
	NSColor * normal;
	NSColor * highlight;
	
	NSString * _identifier;
}

@property (getter=isHidden, setter=setHidden:)				BOOL hidden;
@property (getter=frame, setter=setFrame:)					NSRect frame;
@property (weak, getter=superview, setter=setSuperview:)	NSView * superview;

- (void) applyContext : (NSRect) frame : (int) activeRow : (long) scrollerPosition;
- (bool) didInitWentWell;
- (void) failure;

- (void) setFrameOrigin : (NSPoint) origin;
- (void) setAlphaValue : (CGFloat) alphaValue : (BOOL) animated;

- (NSScrollView*) getContent;
- (void) resizeAnimation : (NSRect) frameRect;
- (void) reloadSize;

//Overwrite only
- (void) additionalResizing : (NSSize) newSize;

- (NSRect) getFrameFromParent : (NSRect) superviewFrame;

- (void) enableDrop;

- (NSInteger) getSelectedElement;
- (NSInteger) getIndexOfElement : (NSInteger) element;
- (float) getSliderPos;
- (NSInteger) selectedRow;

- (NSColor *) getTextColor;
- (NSColor *) getTextColor : (uint) column : (uint) row;
- (NSColor *) getTextHighlightColor;
- (NSColor *) getTextHighlightColor : (uint) column : (uint) row;
- (NSColor *) getBackgroundHighlightColor;

- (void) selectRow : (int) row;
- (void) resetSelection : (NSTableView *) tableView;

- (void) smartReload : (SR_DATA*) oldData : (uint) nbElemOld : (SR_DATA*) newData : (uint) nbElemNew;
- (void) fullAnimatedReload : (uint) oldElem : (uint) newElem;

- (void) fillDragItemWithData : (RakDragItem*) data : (uint) row;
- (BOOL) acceptDrop : (id < NSDraggingInfo >)info row:(NSInteger)row dropOperation:(NSTableViewDropOperation)operation source:(uint) source;
- (void) cleanupDrag;
- (BOOL) receiveDrop : (PROJECT_DATA) project : (bool) isTome : (int) element : (uint) sender : (NSInteger)row : (NSTableViewDropOperation)operation;

+ (void) propagateDragAndDropChangeState : (NSView*) view : (BOOL) started : (BOOL) canDL;

@end