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

+ (uint) getOwnerOfTV : (NSView *) view;
- (BOOL) supportReorder;
- (uint) getSelfCode;
- (PROJECT_DATA) getProjectDataForDrag : (uint) row;
- (NSString *) contentNameForDrag : (uint) row;

+ (void) registerToPasteboard : (NSPasteboard *) pboard;
- (NSDragOperation) operationForContext : (id < NSDraggingInfo >) item : (uint) sourceTab : (NSInteger) suggestedRow : (NSTableViewDropOperation) operation;
- (BOOL) grantDropAuthorization : (BOOL) canDL;
- (NSDragOperation) defineDropAuthorizations :(id < NSDraggingInfo >)info sender : (uint) sender proposedRow:(NSInteger)row  operation: (NSTableViewDropOperation) operation;
- (void) beginDraggingSession : (NSDraggingSession *)session willBeginAtPoint:(NSPoint)screenPoint forRowIndexes:(NSIndexSet *)rowIndexes withParent : (NSView*) view;
- (NSImage*) initializeImageForItem : (PROJECT_DATA) project : (NSString *) name : (uint) rowForAdditionalDrawing;

//Customisation
- (void) additionalDrawing : (RakDragView *) draggedView : (uint) row;
- (NSRect) updateFrameBeforeDrag : (NSRect) earlyFrame;

@end
