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
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

@interface RakDragResponder : NSObject
{
	IBOutlet RakDragView * draggedView;
}

#define REORDER_SERIE @"ReorderProjects"
#define REORDER_MDL @"ReorderMDL"

+ (uint) getOwnerOfTV : (RakView *) view;
- (BOOL) supportReorder;
- (uint) getSelfCode;
- (PROJECT_DATA) getProjectDataForDrag : (uint) row;
- (NSString *) contentNameForDrag : (uint) row;

+ (void) registerToPasteboard : (NSPasteboard *) pboard;
- (NSDragOperation) operationForContext : (id < NSDraggingInfo >) item : (uint) sourceTab : (NSInteger) suggestedRow : (NSTableViewDropOperation) operation;
- (BOOL) grantDropAuthorization : (RakDragItem *) item;
- (NSDragOperation) defineDropAuthorizations :(id < NSDraggingInfo >)info sender : (uint) sender proposedRow:(NSInteger)row  operation: (NSTableViewDropOperation) operation;
- (void) beginDraggingSession : (NSDraggingSession *)session willBeginAtPoint:(NSPoint)screenPoint forRowIndexes:(NSIndexSet *)rowIndexes withParent : (RakView*) view;
- (RakImage*) initializeImageForItem : (PROJECT_DATA) project : (NSString *) name : (uint) rowForAdditionalDrawing;

+ (void) patchPasteboardForFiledrop : (NSPasteboard *) pBoard forType : (NSString *) type;

//Customisation
- (void) additionalDrawing : (RakDragView *) draggedView : (uint) row;
- (NSRect) updateFrameBeforeDrag : (NSRect) earlyFrame;

@end
