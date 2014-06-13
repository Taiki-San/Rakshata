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

@interface RakDragResponder : NSObject
{
	IBOutlet RakDragView * draggedView;
}

#define REORDER_SERIE @"ReorderProjects"
#define REORDER_MDL @"ReorderMDL"

+ (uint) getOwnerOfTV : (NSTableView *) tableView;
- (BOOL) supportReorder;
- (uint) getSelfCode;
- (MANGAS_DATA) getProjectDataForDrag : (uint) row;
- (NSString *) contentNameForDrag : (uint) row;
- (NSString *) reorderCode;

+ (void) registerToPasteboard : (NSPasteboard *) pboard;
- (NSDragOperation) operationForContext : (id < NSDraggingInfo >) item : (uint) sourceTab : (NSInteger) suggestedRow;
- (NSDragOperation) defineDropAuthorizations :(id < NSDraggingInfo >)info proposedRow:(NSInteger)row;
- (void) beginDraggingSession : (NSDraggingSession *)session willBeginAtPoint:(NSPoint)screenPoint forRowIndexes:(NSIndexSet *)rowIndexes withParent : (NSView*) view;


@end
