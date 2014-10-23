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

@implementation RakDragResponder

#pragma mark - Drag'n drop configuration

+ (uint) getOwnerOfTV : (NSView *) view
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
	
	return 0;
}

- (BOOL) supportReorder
{
	return NO;
}

- (uint) getSelfCode
{
#ifdef DEV_VERSION
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
	[pboard declareTypes:[NSArray arrayWithObject: PROJECT_PASTEBOARD_TYPE] owner:self];
}

- (NSDragOperation) operationForContext : (id < NSDraggingInfo >) item : (uint) sourceTab : (NSInteger) suggestedRow : (NSTableViewDropOperation) operation
{
	return NSDragOperationNone;
}

- (BOOL) grantDropAuthorization : (BOOL) canDL
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
			if(![self grantDropAuthorization : [item canDL]])
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

- (void) beginDraggingSession : (NSDraggingSession *)session willBeginAtPoint:(NSPoint)screenPoint forRowIndexes:(NSIndexSet *)rowIndexes withParent : (NSView*) view
{
	if ([rowIndexes count] > 1)
		return;
	
	NSUInteger row = [rowIndexes firstIndex];
	
	[session enumerateDraggingItemsWithOptions:NSDraggingItemEnumerationConcurrent
									   forView:view
									   classes:@[[NSPasteboardItem class], [NSStringPboardType class]]
								 searchOptions:nil
									usingBlock:^(NSDraggingItem *draggingItem, NSInteger index, BOOL *stop)
	 {
		 [[NSBundle mainBundle] loadNibNamed:@"dragView" owner:self topLevelObjects:nil];
		 
		 if(draggedView != nil)
		 {
			 [draggedView setupContent:[self getProjectDataForDrag : row] :[self contentNameForDrag : row]];
			 [self additionalDrawing : draggedView];
			 
			 NSImage * image = [draggedView createImage];
			 NSRect frame = NSMakeRect(draggingItem.draggingFrame.origin.x + draggingItem.draggingFrame.size.width / 2 - image.size.width / 2,
									   draggingItem.draggingFrame.origin.y + draggingItem.draggingFrame.size.height / 2 - image.size.height / 2,
									   image.size.width, image.size.height);
			 
			 [draggingItem setDraggingFrame:frame contents:image];
			 *stop = NO;
		 }
	 }];
}

- (void) additionalDrawing : (RakDragView *) draggedView
{
	
}

@end
