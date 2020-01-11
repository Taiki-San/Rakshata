/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at https://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

#import "RakListScrollView.h"
#import "RakOutlineList.h"
#import "RakListItemView.h"

#define RAKLIST_MAIN_COLUMN_ID @"For the New Lunar Republic!"
#define LIST_INVALID_SELECTION	UINT_MAX
#define LIST_WIDE_LINE_HEIGHT 45

@interface RakTableView : NSTableView
{
	BOOL _lastSelectionWasClic;
	
	uint _preCommitedLastClickedRow;
	
	uint _lastSelectedRow;
	uint _lastSelectedColumn;
}

@property BOOL wantVerboseClick;
@property uint lastClickedRow;
@property uint lastClickedColumn;
@property uint preCommitedLastClickedColumn;

- (RakColor *) _dropHighlightColor;

//Need to be called when the clicked row/column are validated, and we want to exploit the data
- (void) commitClic;

@end

typedef struct //smartReload_data
{
	uint64_t data;
	BOOL installed;
	
} SR_DATA;

@interface RakList : RakDragResponder <NSTableViewDelegate, NSTableViewDataSource, NSMenuDelegate>
{
	void* _data;
	uint _nbData;
	RakListScrollView * scrollView;
	RakTableView * _tableView;
	
	uint selectedRowIndex;
	uint selectedColumnIndex;
	uint _nbElemPerCouple;
	uint _nbCoupleColumn;

	BOOL isListOfRepo;
	CGFloat _scrollerWidth;
	
	RakProjectMenu * menuHandler;
	
	//Color cache
	RakColor * normal;
	RakColor * highlight;
	RakColor * _tmpColor;

	NSDraggingSession * draggingSession;
	NSString * _identifier;
}

@property (getter=isHidden, setter=setHidden:)				BOOL hidden;
@property (getter=frame, setter=setFrame:)					NSRect frame;
@property (weak, getter=superview, setter=setSuperview:)	RakView * superview;
@property BOOL _selectionChangeComeFromClic;

//Simplify the interface for simple use
@property NSArray * defaultDataField;
@property id defaultResponder;
@property SEL action;


- (void) applyContext : (NSRect) frame : (uint) activeRow : (long) scrollerPosition;
- (void) failure;

- (void) setFrameOrigin : (NSPoint) origin;
- (void) setAlphaValue : (CGFloat) alphaValue : (BOOL) animated;

- (RakListScrollView*) getContent;
- (void) resizeAnimation : (NSRect) frameRect;
- (void) reloadSize;

- (void) updateMultiColumn : (NSSize) scrollviewSize;
//Overwrite only
- (void) additionalResizing : (NSSize) newSize : (BOOL) animated;

- (NSRect) getFrameFromParent : (NSRect) superviewFrame;
- (void) needUpdateTableviewHeight;

- (void) enableDrop;

- (void) activateMenu;
- (void) configureMenu : (NSMenu *) menu forLine : (NSInteger) line column : (NSInteger) column;

- (uint) getSelectedElement;
- (uint) getIndexOfElement : (uint) element;
- (float) getSliderPos;
- (NSInteger) selectedRow;

- (RakColor *) getTextColor;
- (RakColor *) getTextColor : (uint) column : (uint) row;
- (RakColor *) getTextHighlightColor;
- (RakColor *) getTextHighlightColor : (uint) column : (uint) row;
- (RakColor *) getBackgroundHighlightColor;

- (void) graphicSelection : (RakView *) view : (BOOL) select;
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
- (BOOL) shouldPromiseFile : (RakDragItem *) item;
- (BOOL) receiveDrop : (PROJECT_DATA) project : (BOOL) isTome : (uint) element : (uint) sender : (NSInteger)row : (NSTableViewDropOperation)operation;

+ (void) propagateDragAndDropChangeState : (BOOL) started : (BOOL) canDL;

@end
