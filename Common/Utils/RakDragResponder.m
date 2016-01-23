/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
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

@implementation RakDragResponder

#pragma mark - Drag'n drop configuration

+ (uint) getOwnerOfTV : (RakView *) view
{
	while(view != nil && [view superclass] != [RakTabView class])
		view = view.superview;
	
	if(view != nil)
	{
		if([view class] == [Series class])
			return TAB_SERIES;
		else if([view class] == [CTSelec class])
			return TAB_CT;
		else if([view class] == [MDL class])
			return TAB_MDL;
	}
	
	return TAB_UNKNOWN;
}

- (BOOL) supportReorder
{
	return NO;
}

- (uint) getSelfCode
{
#ifdef EXTENSIVE_LOGGING
	NSLog(@"Default implementation shouldn't be used!");
#endif
	return TAB_MASK;
}

- (PROJECT_DATA) getProjectDataForDrag : (uint) row
{
	PROJECT_DATA returnData;
	
	memset(&returnData, 0, sizeof(returnData));
	
	return returnData;
}

- (NSString *) contentNameForDrag : (uint) row
{
	return nil;
}

#pragma mark - Internal code

+ (void) registerToPasteboard : (NSPasteboard *) pboard
{
	[pboard declareTypes:@[PROJECT_PASTEBOARD_TYPE] owner:self];
}

- (NSDragOperation) operationForContext : (id < NSDraggingInfo >) item : (uint) sourceTab : (NSInteger) suggestedRow : (NSTableViewDropOperation) operation
{
	return NSDragOperationNone;
}

- (BOOL) grantDropAuthorization : (RakDragItem *) item
{
	return YES;
}

- (NSDragOperation) defineDropAuthorizations :(id < NSDraggingInfo >)info sender : (uint) sender proposedRow:(NSInteger)row  operation: (NSTableViewDropOperation) operation
{
	NSPasteboard * pasteboard = [info draggingPasteboard];
	if(pasteboard != nil)
	{
		RakDragItem * item = [[RakDragItem alloc] initWithData: [pasteboard dataForType:PROJECT_PASTEBOARD_TYPE]];
		if(item != nil)
		{
			if(![self grantDropAuthorization : item])
				return NSDragOperationNone;
		}
	}
	
	if(sender == [self getSelfCode])
	{
		if([self supportReorder])
			return NSDragOperationMove;
		
		return NSDragOperationNone;
	}
	return [self operationForContext:info :sender :row :NSTableViewDropOn];
}

- (void) beginDraggingSession : (NSDraggingSession *)session willBeginAtPoint:(NSPoint)screenPoint forRowIndexes:(NSIndexSet *)rowIndexes withParent : (RakView*) view
{
	if([rowIndexes count] > 1)
		return;
	
	NSUInteger row = [rowIndexes firstIndex];
	
	[session enumerateDraggingItemsWithOptions:NSDraggingItemEnumerationConcurrent
									   forView:view
									   classes:@[[NSPasteboardItem class]]
								 searchOptions:@{}
									usingBlock:^(NSDraggingItem *draggingItem, NSInteger index, BOOL *stop)
	 {
		 RakImage * image = [self initializeImageForItem : [self getProjectDataForDrag : row] : [self contentNameForDrag : row] : row];
		 if(image != nil)
		 {
			 NSRect frame = NSMakeRect(draggingItem.draggingFrame.origin.x - image.size.width / 3,
									   draggingItem.draggingFrame.origin.y - 3 * image.size.height / 4,
									   image.size.width, image.size.height);
			 
			 [draggingItem setDraggingFrame:[self updateFrameBeforeDrag : frame] contents:image];
		 }
		 else
			 *stop = YES;
	 }];
}

- (RakImage *) initializeImageForItem : (PROJECT_DATA) project : (NSString *) name : (uint) rowForAdditionalDrawing
{
	if(project.isInitialized)
	{
		[[NSBundle mainBundle] loadNibNamed:@"dragView" owner:self topLevelObjects:nil];
		if(draggedView != nil)
		{
			[draggedView setupContent: project : name];
			[self additionalDrawing : draggedView : rowForAdditionalDrawing];
			
			return [draggedView createImage];
		}
	}
	
	return nil;
}

- (void) additionalDrawing : (RakDragView *) draggedView : (uint) row
{
	
}

- (NSRect) updateFrameBeforeDrag : (NSRect) earlyFrame
{
	return earlyFrame;
}

#pragma mark - Helper API

+ (void) patchPasteboardForFiledrop : (NSPasteboard *) pBoard forType : (NSString *) type
{
	//On y croit \o/
	NSMutableArray * array = [NSMutableArray arrayWithObject:NSFilesPromisePboardType];
	[array addObjectsFromArray:[pBoard types]];

	[pBoard declareTypes:[NSArray arrayWithArray:array] owner:self];
	[pBoard setPropertyList:@[type] forType:NSFilesPromisePboardType];
}

@end
