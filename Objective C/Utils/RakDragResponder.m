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

- (uint) getOwnerOfTV : (NSTableView *) tableView
{
	NSView * view = tableView.superview;
	
	while(view != nil && [view superclass] != [RakTabView class])
		view = view.superview;
	
	if(view != nil)
	{
		if([view class] == [Series class])
			return GUI_THREAD_SERIES;
		else if([view class] == [CTSelec class])
			return GUI_THREAD_CT;
		else if([view class] == [MDL class])
			return GUI_THREAD_MDL;
	}
	
	return 0;
}

- (BOOL) supportReorder
{
	return NO;
}

- (uint) getSelfCode
{
	NSLog(@"Default implementation shouldn't be used!");
	return GUI_THREAD_MASK;
}

- (MANGAS_DATA) getProjectDataForDrag : (uint) row
{
	MANGAS_DATA returnData;
	
	memset(&returnData, 0, sizeof(returnData));
	
	return returnData;
}

- (NSString *) contentNameForDrag : (uint) row
{
	return nil;
}

- (NSString *) reorderCode
{
	return nil;
}

#pragma mark - Internal code

- (void) registerToPasteboard : (NSPasteboard *) pboard
{
	[pboard declareTypes:[NSArray arrayWithObjects:NSStringPboardType,nil] owner:self];
}

- (NSDragOperation) operationForContext : (id < NSDraggingInfo >) item : (uint) sourceTab : (NSInteger) suggestedRow
{
	return NSDragOperationNone;
}

- (NSDragOperation) defineDropAuthorizations :(id < NSDraggingInfo >)info proposedRow:(NSInteger)row
{
	uint tab = [self getOwnerOfTV:[info draggingSource]];
	
	if(tab == [self getSelfCode])
	{
		if([self supportReorder])
			return NSDragOperationMove;
		
		return NSDragOperationNone;
	}
	return [self operationForContext:info :tab :row];
}

- (void) beginDraggingSession : (NSDraggingSession *)session willBeginAtPoint:(NSPoint)screenPoint forRowIndexes:(NSIndexSet *)rowIndexes withParent : (NSView*) view
{
	if ([rowIndexes count] > 1)
		return;
	
	NSUInteger row = [rowIndexes firstIndex];
	
	[session enumerateDraggingItemsWithOptions:NSDraggingItemEnumerationConcurrent
									   forView:view
									   classes:[NSArray arrayWithObjects:[NSPasteboardItem class], [NSStringPboardType class], nil]
								 searchOptions:nil
									usingBlock:^(NSDraggingItem *draggingItem, NSInteger index, BOOL *stop)
	 {
		 [[NSBundle mainBundle] loadNibNamed:@"dragView" owner:self topLevelObjects:nil];
		 
		 if(draggedView != nil)
		 {
			 [draggedView setupContent:[self getProjectDataForDrag : row] :[self contentNameForDrag : row]];
			 
			 NSImage * image = [draggedView initImage];
			 NSRect frame = NSMakeRect(draggingItem.draggingFrame.origin.x + draggingItem.draggingFrame.size.width / 2 - image.size.width / 2,
									   draggingItem.draggingFrame.origin.y + draggingItem.draggingFrame.size.height / 2 - image.size.height / 2,
									   image.size.width, image.size.height);
			 
			 [draggingItem setDraggingFrame:frame contents:image];
			 *stop = NO;
		 }
	 }];
	
	NSString * type = [self reorderCode];
	if(type != nil)
		[session.draggingPasteboard setData:[NSData data] forType:type];
}

@end
