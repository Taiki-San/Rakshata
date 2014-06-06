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

#import "RakListDragTextView.h"

@interface RakListScrollView : NSScrollView
{
	NSView* documentViewToResize;
}

- (void) resizeAnimation : (NSRect) frameRect;

@end

@interface RakTableView : NSTableView

- (NSColor *) _dropHighlightColor;

@end

@interface RakList : NSObject <NSTableViewDelegate, NSTableViewDataSource, NSDraggingDestination>
{
	void* data;
	uint amountData;
	RakListScrollView * scrollView;
	RakTableView * _tableView;
	
	//Color cache
	NSColor * normal;
	NSColor * highlight;
}

- (void) applyContext : (NSRect) frame : (int) activeRow : (long) scrollerPosition;
- (bool) didInitWentWell;
- (void) failure;

- (NSScrollView*) getContent;
- (void) setSuperView : (NSView *) superview;
- (void) setHidden : (bool) state;
- (BOOL) isHidden;
- (NSRect) frame;
- (void) setFrame : (NSRect) frameRect;
- (void) resizeAnimation : (NSRect) frameRect;

- (NSRect) getTableViewFrame : (NSRect) superViewFrame;

- (NSInteger) getSelectedElement;
- (float) getSliderPos;
- (NSInteger) selectedRow;

- (NSColor *) getTextColor;
- (NSColor *) getTextHighlightColor;
- (NSColor *) getBackgroundHighlightColor;

#define REORDER_SERIE @"ReorderProjects"
#define REORDER_MDL @"ReorderMDL"

- (NSString *) reorderCode;
- (uint) getOwnerOfTV : (NSTableView *) tableView;

- (BOOL) supportReorder;
- (uint) getSelfCode;
- (NSDragOperation) operationForContext : (id < NSDraggingInfo >) item : (uint) sourceTab : (NSInteger) suggestedRow;

@end