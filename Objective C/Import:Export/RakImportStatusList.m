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

		//Expand what have to be
		[rootCollector enumerateObjectsUsingBlock:^(RakImportStatusListItem * obj, NSUInteger idx, BOOL * stop) {

			if(obj.status != STATUS_BUTTON_OK && !obj.metadataProblem && [obj getNbChildren] < 10)
			{
				[content expandItem:obj];
			}

		}];
		
		//We register the internal D&D
		[content registerForDraggedTypes: @[IMPORT_ENTRY_PB_TYPE]];
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

- (NSView *)outlineView:(NSOutlineView *)outlineView viewForTableColumn:(NSTableColumn *)tableColumn item:(RakImportStatusListItem *)item
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

+ (NSArray <RakImportItem *> *) linearizeItem : (RakImportStatusListItem *) item
{
	if(!item.rootItem)
		return @[item.itemForChild];
	
	else if([item getNbChildren] == 0)
		return nil;
	
	NSMutableArray <RakImportItem *> * collector = [NSMutableArray new];
	
	for(uint i = 0; i < [item getNbChildren]; ++i)
	{
		NSArray * new = [[self class] linearizeItem:[item getChildAtIndex:i]];
		if(new != nil)
			[collector addObjectsFromArray:new];
	}
	
	return [NSArray arrayWithArray:collector];
}

- (void) updateDraggingProject : (NSArray <RakImportItem *> *) array
{
	RakImportItem * item = [array objectAtIndex:0];
	if([array count] > 1)
	{
		//We check if it is not all the same project
		bool wasFirstLocal = item.projectData.data.project.locale;
		uint projectID = item.projectData.data.project.projectID, cacheID = item.projectData.data.project.cacheDBID;
		
		for(uint i = 1, length = [array count]; i < length; ++i)
		{
			item = [array objectAtIndex:i];
			
			if(wasFirstLocal != item.projectData.data.project.locale || projectID != item.projectData.data.project.projectID || cacheID != item.projectData.data.project.cacheDBID)
			{
				//We will craft a fake PROJECT_DATA if needed not to have to re-create the UI from scratch
				_draggedProject = getEmptyProject();
				_draggedProject.isInitialized = true;
				wstrncpy(_draggedProject.projectName, LENGTH_PROJECT_NAME, L"Divers séries");
				return;
			}
		}
		
		item = [array objectAtIndex:0];
	}
	
	_draggedProject = item.projectData.data.project;
	nullifyCTPointers(&_draggedProject);
	
	//Not commited imported project are yet flagged as initiliazed. However, they contain the
	if(!_draggedProject.isInitialized)
		_draggedProject.isInitialized = true;
}

- (BOOL)outlineView:(NSOutlineView *)outlineView writeItems:(NSArray *)items toPasteboard:(NSPasteboard *)pboard
{
	NSMutableArray <RakImportItem *> * collector = [NSMutableArray new];
	
	//We only drag
	for(RakImportStatusListItem * item in items)
	{
		NSArray * new = [[self class] linearizeItem : item];
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
	currentDraggedItem = [draggedItems objectAtIndex:0];
	
	[self beginDraggingSession:session willBeginAtPoint:screenPoint forRowIndexes:[NSIndexSet indexSetWithIndex:42] withParent:outlineView];
	
	currentDraggedItem = nil;
}

- (NSArray<NSString *> *)outlineView:(NSOutlineView *)outlineView namesOfPromisedFilesDroppedAtDestination:(NSURL *)dropDestination forDraggedItems:(NSArray *)items
{
	return nil;
}

- (void) outlineView:(NSOutlineView *)outlineView draggingSession:(NSDraggingSession *)session endedAtPoint:(NSPoint)screenPoint operation:(NSDragOperation)operation
{
	[RakList propagateDragAndDropChangeState :NO : [RakDragItem canDL:[session draggingPasteboard]]];
}

- (BOOL)outlineView:(NSOutlineView *)outlineView acceptDrop:(id<NSDraggingInfo>)info item:(id)item childIndex:(NSInteger)index
{
	return NO;
}

#pragma mark Internal Utils

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

@end
