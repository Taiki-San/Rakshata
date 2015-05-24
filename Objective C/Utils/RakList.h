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

#import "RakListScrollView.h"
#import "RakOutlineList.h"

#define RAKLIST_MAIN_COLUMN_ID @"For the New Lunar Republic!"
#define LIST_INVALID_SELECTION	UINT_MAX
#define LIST_WIDE_LINE_HEIGHT 45

@interface RakTableView : NSTableView
{
	BOOL _lastSelectionWasClic;
	
	uint _preCommitedLastClickedRow;
	
	uint _lastClickedRow;
	uint _lastClickedColumn;
}

@property BOOL wantVerboseClick;
@property uint lastClickedRow;
@property uint lastClickedColumn;
@property uint preCommitedLastClickedColumn;

- (NSColor *) _dropHighlightColor;

//Need to be called when the clicked row/column are validated, and we want to exploit the data
- (void) commitClic;

@end

typedef struct smartReload_data
{
	uint64_t data;
	BOOL installed;
	
} SR_DATA;

@interface RakList : RakDragResponder <NSTableViewDelegate, NSTableViewDataSource>
{
	void* _data;
	uint _nbData;
	RakListScrollView * scrollView;
	RakTableView * _tableView;
	
	NSView * preloadedRow;
	
	uint selectedRowIndex;
	uint selectedColumnIndex;
	uint _nbElemPerCouple;
	uint _nbCoupleColumn;
	
	CGFloat _scrollerWidth;
	
	//Color cache
	NSColor * normal;
	NSColor * highlight;
	NSColor * _tmpColor;
	
	NSString * _identifier;
}

@property (getter=isHidden, setter=setHidden:)				BOOL hidden;
@property (getter=frame, setter=setFrame:)					NSRect frame;
@property (weak, getter=superview, setter=setSuperview:)	NSView * superview;
@property BOOL _selectionChangeComeFromClic;

- (void) applyContext : (NSRect) frame : (int) activeRow : (long) scrollerPosition;
- (void) failure;

- (void) setFrameOrigin : (NSPoint) origin;
- (void) setAlphaValue : (CGFloat) alphaValue : (BOOL) animated;

- (NSScrollView*) getContent;
- (void) resizeAnimation : (NSRect) frameRect;
- (void) reloadSize;

- (void) updateMultiColumn : (NSSize) scrollviewSize;
//Overwrite only
- (void) additionalResizing : (NSSize) newSize;

- (NSRect) getFrameFromParent : (NSRect) superviewFrame;
- (void) needUpdateTableviewHeight;

- (void) enableDrop;

- (uint) getSelectedElement;
- (uint) getIndexOfElement : (uint) element;
- (float) getSliderPos;
- (NSInteger) selectedRow;

- (NSColor *) getTextColor;
- (NSColor *) getTextColor : (uint) column : (uint) row;
- (NSColor *) getTextHighlightColor;
- (NSColor *) getTextHighlightColor : (uint) column : (uint) row;
- (NSColor *) getBackgroundHighlightColor;

- (void) graphicSelection : (NSView *) view : (BOOL) select;
- (void) postProcessingSelection : (uint) row;

- (void) selectElement : (uint) element;
- (void) selectIndex : (uint) index;
- (void) resetSelection : (NSTableView *) tableView;

- (Class) contentClass;

- (uint) rowFromCoordinates : (uint) row : (uint) column;
- (uint) coordinateForIndex : (uint) index : (uint *) column;

- (void) smartReload : (SR_DATA*) oldData : (uint) nbElemOld : (SR_DATA*) newData : (uint) nbElemNew;
- (void) fullAnimatedReload : (uint) oldElem : (uint) newElem;
- (void) fullAnimatedReload : (uint) oldElem : (uint) newElem : (BOOL) revertedOrder;

- (void) fillDragItemWithData : (RakDragItem*) data : (uint) row;
- (BOOL) acceptDrop : (id < NSDraggingInfo >)info row:(NSInteger)row dropOperation:(NSTableViewDropOperation)operation source:(uint) source;
- (void) cleanupDrag;
- (BOOL) receiveDrop : (PROJECT_DATA) project : (BOOL) isTome : (int) element : (uint) sender : (NSInteger)row : (NSTableViewDropOperation)operation;

+ (void) propagateDragAndDropChangeState : (BOOL) started : (BOOL) canDL;

@end