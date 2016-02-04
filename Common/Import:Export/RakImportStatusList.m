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

#define IMPORT_ENTRY_PB_TYPE @"UghHopefullyThisWillWorkAtSomePoint"

enum
{
	LIST_WIDTH = 350,
	ROW_HEIGHT = 30
};

@implementation RakImportStatusList

- (instancetype) initWithImportList : (NSArray *) dataset
{
	self = [super init];

	if(self != nil)
	{
		_dataSet = dataset;

		//We want to create our roots with projects, something that would be super inefficient with the standard system
		NSMutableArray * projectHashCollector = [NSMutableArray new], * rootCollector = [NSMutableArray new];
		for(RakImportItem * item in dataset)
		{
			//We assume the path is unique for a given project
			char * pathToProject = getPathForProject(item.projectData.data.project);

			if(pathToProject == NULL)
				continue;

			NSString * string = [NSString stringWithUTF8String:pathToProject];
			free(pathToProject);

			NSUInteger index = [projectHashCollector indexOfObject:string];
			RakImportStatusListItem * currentRoot;

			if(index != NSNotFound)
				currentRoot = [rootCollector objectAtIndex:index];
			else
			{
				currentRoot = [[RakImportStatusListItem alloc] initWithProject:item.projectData.data.project];
				[rootCollector addObject:currentRoot];
				[projectHashCollector addObject:string];
			}

			[currentRoot addItemAsChild:item];
		}

		[rootCollector enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL * stop) {	[obj commitFinalList];	}];
		rootItems = [NSArray arrayWithArray:rootCollector];

		_nbRoot = [rootItems count];

		[self initializeMain:NSMakeRect(0, 0, LIST_WIDTH, 200)];
		[content setHeaderView:nil];
		[content setAllowsMultipleSelection:YES];
		
		content.wantUpdateScrollview = YES;

		//Expand what have to be
		[rootCollector enumerateObjectsUsingBlock:^(RakImportStatusListItem * obj, NSUInteger idx, BOOL * stop) {

			if(obj.status != STATUS_BUTTON_OK && !obj.metadataProblem && [obj getNbChildren] < 10)
			{
				[content expandItem:obj];
			}

		}];
		
		//We register the internal D&D
		[content registerForDraggedTypes: @[IMPORT_ENTRY_PB_TYPE]];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(openChildNotif:) name:NOTIFICATION_IMPORT_OPEN_ITEM object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(senpaiNoticedMe:) name:NOTIFICATION_IMPORT_NEED_CALLBACK object:nil];
	}

	return self;
}

- (id) outlineView:(NSOutlineView *)outlineView child:(NSInteger)index ofItem:(id)item
{
	if(index < 0)
		return nil;

	id output = nil;

	if(item == nil)
	{
		if([rootItems count] > (NSUInteger) index)
			output = [rootItems objectAtIndex:(NSUInteger) index];
	}
	else
		output = [item getChildAtIndex:(NSUInteger) index];

	return output;
}

#pragma mark Outline view

- (CGFloat)outlineView:(NSOutlineView *)outlineView heightOfRowByItem:(id)item
{
	return item == nil ? 0 : ROW_HEIGHT;
}

- (NSTableRowView *) outlineView:(NSOutlineView *)outlineView rowViewForItem : (id) item
{
	RakTableRowView *rowView = [outlineView makeViewWithIdentifier:@"HeaderRowView" owner:self];
	if(rowView == nil)
	{
		rowView = [[RakTableRowView alloc] init];
		rowView.drawBackground = YES;
		rowView.identifier = @"HeaderRowView";
	}

	return rowView;
}

- (RakView *)outlineView:(NSOutlineView *)outlineView viewForTableColumn:(NSTableColumn *)tableColumn item:(RakImportStatusListItem *)item
{
	RakImportStatusListRowView * rowView = [outlineView makeViewWithIdentifier:@"Itis37COutsideImDying" owner:self];
	if(rowView == nil)
	{
		rowView = [[RakImportStatusListRowView alloc] initWithFrame:NSZeroRect];
		rowView.identifier = @"Itis37COutsideImDying";
		rowView.list = self;
	}

	[rowView updateWithItem :item];

	return rowView;
}

- (NSIndexSet *)outlineView:(NSOutlineView *)outlineView selectionIndexesForProposedSelection:(NSIndexSet *)proposedSelectionIndexes
{
	return proposedSelectionIndexes;
}

#pragma mark Drag & Drop

+ (NSArray <NSDictionary *> *) linearizeItem : (RakImportStatusListItem *) item ofParent : (RakImportStatusListItem *) parent
{
	if(!item.rootItem)
		return @[@{@"item" : item.itemForChild, @"root" : parent}];
	
	else if([item getNbChildren] == 0)
		return nil;
	
	NSMutableArray <RakImportItem *> * collector = [NSMutableArray new];
	
	for(uint i = 0; i < [item getNbChildren]; ++i)
	{
		NSArray * new = [[self class] linearizeItem:[item getChildAtIndex:i] ofParent:item];
		if(new != nil)
			[collector addObjectsFromArray:new];
	}
	
	return [NSArray arrayWithArray:collector];
}

- (void) updateDraggingProject : (NSArray <NSDictionary *> *) array
{
	RakImportItem * item = [[array objectAtIndex:0] objectForKey:@"item"];
	if([array count] > 1)
	{
		//We check if it is not all the same project
		bool wasFirstLocal = item.projectData.data.project.locale;
		uint projectID = item.projectData.data.project.projectID, cacheID = item.projectData.data.project.cacheDBID;
		
		for(uint i = 1, length = [array count]; i < length; ++i)
		{
			item = [[array objectAtIndex:i] objectForKey:@"item"];
			
			if(wasFirstLocal != item.projectData.data.project.locale || projectID != item.projectData.data.project.projectID || cacheID != item.projectData.data.project.cacheDBID)
			{
				//We will craft a fake PROJECT_DATA if needed not to have to re-create the UI from scratch
				_draggedProject = getEmptyProject();
				_draggedProject.isInitialized = true;
				wstrncpy(_draggedProject.projectName, LENGTH_PROJECT_NAME, (charType*) [NSLocalizedString(@"IMPORT-DRAG-VARIOUS-SERIES", nil) cStringUsingEncoding:NSUTF32StringEncoding]);
				return;
			}
		}
		
		item = [[array objectAtIndex:0] objectForKey:@"item"];
	}
	
	_draggedProject = item.projectData.data.project;
	nullifyCTPointers(&_draggedProject);
	
	//Not commited imported project are flagged as initialized. However, they contain the
	if(!_draggedProject.isInitialized)
		_draggedProject.isInitialized = true;
}

- (BOOL)outlineView:(NSOutlineView *)outlineView writeItems:(NSArray *)items toPasteboard:(NSPasteboard *)pboard
{
	NSMutableArray <NSDictionary *> * collector = [NSMutableArray new];
	
	//We only drag
	for(RakImportStatusListItem * item in items)
	{
		NSArray * new = [[self class] linearizeItem : item ofParent:[outlineView parentForItem:item]];
		if(new != nil)
			[collector addObjectsFromArray:new];
	}
	
	[pboard declareTypes:@[IMPORT_ENTRY_PB_TYPE] owner:self];

	[self updateDraggingProject:collector];
	
	return [pboard setData:[[NSArray arrayWithArray:collector] convertToData] forType:IMPORT_ENTRY_PB_TYPE];
}

- (PROJECT_DATA) getProjectDataForDrag : (uint) row
{
	return _draggedProject;
}

- (void)outlineView:(NSOutlineView *)outlineView draggingSession:(NSDraggingSession *)session willBeginAtPoint:(NSPoint)screenPoint forItems:(NSArray *)draggedItems
{
	[self beginDraggingSession:session willBeginAtPoint:screenPoint forRowIndexes:[NSIndexSet indexSetWithIndex:42] withParent:outlineView];
}

- (NSDragOperation)outlineView:(NSOutlineView *)outlineView validateDrop:(id < NSDraggingInfo >)info proposedItem:(id)targetItem proposedChildIndex:(NSInteger)index
{
	if([targetItem isKindOfClass:[RakImportStatusListItem class]] && ((RakImportStatusListItem *)  targetItem).isRootItem)
		return NSDragOperationMove;
	
	return NSDragOperationNone;
}

- (BOOL)outlineView:(NSOutlineView *)outlineView acceptDrop:(id<NSDraggingInfo>)info item : (RakImportStatusListItem *) finalItem childIndex:(NSInteger)finalIndex
{
	NSPasteboard * pasteboard = [info draggingPasteboard];
	if(pasteboard == nil || finalItem == nil || !finalItem.rootItem || [finalItem getNbChildren] == 0)
		return NO;
	
	NSArray * array = [NSArray arrayWithData:[pasteboard dataForType:IMPORT_ENTRY_PB_TYPE]];
	if(array == nil)
		return NO;

	[self moveItems:array toParent:finalItem atIndex : finalIndex];
	
	return YES;
}

#pragma mark Internal Utils & Notification

+ (void) refreshAfterPass
{
	[[NSNotificationCenter defaultCenter] postNotificationName:NOTIFICATION_CHILD object:nil];
	[[NSNotificationCenter defaultCenter] postNotificationName:NOTIFICATION_ROOT object:nil];
	[[NSNotificationCenter defaultCenter] postNotificationName:NOTIFICATION_IMPORT_STATUS_UI object:nil];
}

- (BOOL) haveDuplicate
{
	for(RakImportItem * item in _dataSet)
	{
		if(item.issue == IMPORT_PROBLEM_DUPLICATE)
			return YES;
	}

	return NO;
}

- (void) setQuery : (RakImportQuery *) newQuery
{
	if(_query != nil && _query != newQuery)
		[_query closePopover];

	_query = newQuery;

	if(newQuery != nil)
		newQuery.controller = _controller;
}

- (BOOL) collapseIfRelevant
{
	for(RakImportItem * item in _dataSet)
	{
		if(item.issue != IMPORT_PROBLEM_NONE)
			return NO;
	}
	
	[_controller close];
	return YES;
}

- (void) moveItems : (NSArray *) items toParent : (RakImportStatusListItem *) newRoot atIndex : (NSInteger) newIndex
{
	BOOL needImportRoot = NO;
	NSMutableArray * itemsNeedingUpdate = [NSMutableArray new];
	NSMutableIndexSet * indexToRemove = [NSMutableIndexSet new];
	
	//We process each entries
	uint count = 0;
	PROJECT_DATA_EXTRA sampleProject, oldProject;
	
	if(newRoot != nil)	//Move to an existing item
	{
		sampleProject = ((RakImportStatusListItem *) [newRoot getChildAtIndex:0]).itemForChild.projectData;
	}
	else				//Oh boy...
	{
		if([items count] == 0)	//Nope nope nope nope
			return;
		
		NSDictionary * dict = [items firstObject];
		RakImportItem * item = [dict objectForKey:@"item"];
		if(item == nil)
			return;
		
		sampleProject = getEmptyExtraProject();

		sampleProject.data.project.locale = true;
		sampleProject.data.project.cacheDBID = getEmptyLocalSlot(sampleProject.data.project);

		swprintf(sampleProject.data.project.projectName, LENGTH_PROJECT_NAME, L"(ex %ls)", item.projectData.data.project.projectName);
		sampleProject.data.project.projectName[LENGTH_PROJECT_NAME - 1] = 0;
		
		newRoot = [[RakImportStatusListItem alloc] initWithProject:sampleProject.data.project];

		NSMutableArray * newRoots = [NSMutableArray arrayWithArray:rootItems];
		if(newRoots == nil)
			return;

		NSUInteger indexOfOld = [rootItems indexOfObject:[dict objectForKey:@"root"]];
		if(indexOfOld != NSNotFound)
			[newRoots insertObject:newRoot atIndex:indexOfOld + 1];
		else
			[newRoots addObject:newRoot];
		
		rootItems = [NSArray arrayWithArray:newRoots];
		_nbRoot = [rootItems count];
		
		needImportRoot = YES;
		newIndex = NSOutlineViewDropOnItemIndex;
	}
	
	nullifyCTPointers(&sampleProject.data.project);
	
	for (NSDictionary * entry in items)
	{
		RakImportItem * item = [entry objectForKey:@"item"];
		RakImportStatusListItem * root = [entry objectForKey:@"root"];
		
		if(item == nil || root == nil || root == newRoot)
			continue;
		
		//We remove the item from its root and add it to the new item
		if(![root removeChild:item])
		{
#ifdef EXTENSIVE_LOGGING
			NSLog(@"Uh, we misidentified the root?!");
#endif
		}
		
		//We update the item
		if(newIndex == NSOutlineViewDropOnItemIndex)
			[newRoot addItemAsChild:item];
		else
			[newRoot insertItemAsChild:item atIndex: newIndex + count++];
		
		NSDictionary * entryID = @{@"index": @(root.indexOfLastRemoved), @"root": root};
		
		//Remove the old root if needed
		if([root getNbChildren] == 0)
		{
			uint index = [rootItems indexOfObject:root];
			
			if(index != NSNotFound)
				[indexToRemove addIndex:index];
			
			//We remove any item of this root from the itemsNeedingUpdate
			NSMutableIndexSet * indexes = [NSMutableIndexSet new];
			
			[itemsNeedingUpdate enumerateObjectsUsingBlock:^(id  _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
				
				if([obj objectForKey:@"root"] == root)
					[indexes addIndex:idx];
			}];
			
			if([indexes count] > 0)
				[itemsNeedingUpdate removeObjectsAtIndexes:indexes];
		}
		else if([itemsNeedingUpdate indexOfObject:entryID] == NSNotFound)
			[itemsNeedingUpdate addObject:entryID];
		
		//Update the project
		oldProject = item.projectData;
		
		releaseCTData(oldProject.data.project);
		nullifyCTPointers(&oldProject.data.project);
		oldProject.data.project = sampleProject.data.project;
		
		generateCTUsable(&oldProject.data);
		
		item.projectData = oldProject;
	}
	
	[NSAnimationContext beginGrouping];
	
	[newRoot checkRefreshStatusRoot];

	if(needImportRoot)
		[content insertItemsAtIndexes:[NSIndexSet indexSetWithIndex:[rootItems indexOfObject:newRoot]] inParent:nil withAnimation:NSTableViewAnimationSlideLeft];
	
	//Remove independant entries
	for(NSDictionary * dict in itemsNeedingUpdate)
	{
		[content removeItemsAtIndexes:[NSIndexSet indexSetWithIndex:[(NSNumber *) [dict objectForKey:@"index"] unsignedIntegerValue]] inParent:[dict objectForKey:@"root"] withAnimation:NSTableViewAnimationSlideLeft];
	}
	
	//Remove empty root
	NSMutableArray * mutable = [NSMutableArray arrayWithArray:rootItems];
	if(mutable != nil)
	{
		[mutable removeObjectsAtIndexes:indexToRemove];
		id new = [NSArray arrayWithArray:mutable];
		if(new != nil)
			rootItems = new;
		
		[content removeItemsAtIndexes:indexToRemove inParent:nil withAnimation:NSTableViewAnimationSlideLeft];
	}
	
	//Reload the receiver
	[content reloadItem:newRoot reloadChildren:YES];
	
	[NSAnimationContext endGrouping];
}

- (void) removeItems : (NSArray *) array
{
	NSMutableArray * itemsToRemove = [NSMutableArray array];
	NSMutableDictionary * rootToRemove = [NSMutableDictionary dictionary];
	NSMutableIndexSet * indexRootToRemove = [NSMutableIndexSet indexSet];

	for(RakImportStatusListItem * item in array)
	{
		BOOL isRoot = item.isRootItem;
		if(!isRoot)	//If a standard item
		{
			RakImportStatusListItem * root = [content parentForItem:item];
			if(root == nil)
				continue;
			
			//We check the root wasn't already discarded
			if([rootToRemove count] > 0 && [rootToRemove objectForKey:@((uint64_t) root)] != nil)
				continue;

			//We delete the root if empty
			else if ([root getNbChildren] <= 1)
			{
				NSUInteger index = [rootItems indexOfObject:root];

				[rootToRemove setObject:@(0) forKey:@((uint64_t) root)];
				[indexRootToRemove addIndex:index];
			}

			//We just remove the child
			else
			{
				[root removeChild:item.itemForChild];
			
				//We new add it to the list to be removed from the array
				[itemsToRemove addObject:@{@"index":@(root.indexOfLastRemoved), @"root": root}];
			}
			
			item.itemForChild.issue = IMPORT_PROBLEM_NONE;
		}
		else
		{
			NSUInteger index = [rootItems indexOfObject:item];
			
			[rootToRemove setObject:@(0) forKey:@((uint64_t) item)];
			[indexRootToRemove addIndex:index];
			
			[item enumerateChildrenWithBlock:^(RakImportStatusListItem * child, uint indexChild, BOOL *stop)
			{
				child.itemForChild.issue = IMPORT_PROBLEM_NONE;
			}];
		}
	}
	
	[NSAnimationContext beginGrouping];
	
	//Remove independant entries
	for(NSDictionary * dict in itemsToRemove)
	{
		[content removeItemsAtIndexes:[NSIndexSet indexSetWithIndex:[(NSNumber *) [dict objectForKey:@"index"] unsignedIntegerValue]] inParent:[dict objectForKey:@"root"] withAnimation:NSTableViewAnimationSlideLeft];
	}
	
	//Remove empty root
	if([indexRootToRemove count] > 0)
	{
		NSMutableArray * mutable = [NSMutableArray arrayWithArray:rootItems];
		if(mutable != nil)
		{
			[mutable removeObjectsAtIndexes:indexRootToRemove];
			
			id new = [NSArray arrayWithArray:mutable];
			if(new != nil)
				rootItems = new;
			
			[content removeItemsAtIndexes:indexRootToRemove inParent:nil withAnimation:NSTableViewAnimationSlideLeft];
		}
	}
	
	[NSAnimationContext endGrouping];
}

- (void) openChildNotif : (NSNotification *) notification
{
	RakOutlineListItem * item = notification.object;
	
	if(item == nil || ![item isKindOfClass:[RakOutlineListItem class]])
		return;
	
	NSUInteger pos = [rootItems indexOfObject:item];
	if(pos == NSNotFound)
		return;
	
	[content expandItem:item expandChildren:YES];
}

- (void) senpaiNoticedMe : (NSNotification *) notification
{
	NSDictionary * userInfo;
	NSArray * itemList;
	NSNumber * request;
	id singleItem;
	
	if(notification == nil || (userInfo = notification.userInfo) == nil)
		return;
	
	else if((singleItem =	[userInfo objectForKey:@"item"]) == nil || ![singleItem isKindOfClass:[RakImportStatusListItem class]])
		return;
	
	else if((request = [userInfo objectForKey:@"payload"]) == nil || ![request isKindOfClass:[NSNumber class]])
		return;
	
	//If items were already selected, they overtake the one we received
	NSIndexSet * selectedRowIndexes = content.selectedRowIndexes;
	if(selectedRowIndexes != nil && [selectedRowIndexes count] > 0)
	{
		NSMutableArray * newItemList = [NSMutableArray array];
		
		[selectedRowIndexes enumerateIndexesUsingBlock:^(NSUInteger idx, BOOL * _Nonnull stop)
		{
			RakImportStatusListItem * current = [content itemAtRow:(NSInteger) idx];
			if(current != nil)
				[newItemList addObject:current];
		}];
		
		if([newItemList count] > 0)
			itemList = [NSArray arrayWithArray:newItemList];
		else
			itemList = @[singleItem];
	}
	else
		itemList = @[singleItem];

	//We process the request
	switch ([request intValue])
	{
		//moveToIndependentNode
		case 1:
		{
			NSMutableArray * array = [NSMutableArray array];
			for(RakImportStatusListItem * item in itemList)
			{
				RakImportStatusListItem * root = [content parentForItem:item];
				if(root != nil)
				{
					[array addObject:@{@"item":item.itemForChild, @"root":root}];
				}
				else
				{
					root = item;
					for(uint i = 0, length = [root getNbChildren]; i < length; ++i)
					{
						[array addObject:@{@"item":((RakImportStatusListItem *) [root getChildAtIndex:i]).itemForChild, @"root":root}];
					}
				}
			}
			
			if([array count])
				[self moveItems:array toParent:nil atIndex:0];
			
			break;
		}
			
		//removeItem
		case 2:
		{
			[self removeItems : itemList];
			break;
		}
			
		default:
			return;
	}
	
	if(![self collapseIfRelevant])
		[[self class] refreshAfterPass];
}

@end
