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

		//Expand what have to be
		[rootCollector enumerateObjectsUsingBlock:^(RakImportStatusListItem * obj, NSUInteger idx, BOOL * stop) {

			if(obj.status != STATUS_BUTTON_OK && !obj.metadataProblem && [obj getNbChildren] < 10)
			{
				[content expandItem:obj];
			}

		}];
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

- (CGFloat)outlineView:(NSOutlineView *)outlineView heightOfRowByItem:(id)item
{
	return item == nil ? 0 : ROW_HEIGHT;
}

- (NSTableRowView *) outlineView:(NSOutlineView *)outlineView rowViewForItem : (RakAddRepoItem*) item
{
	RakTableRowView *rowView = [outlineView makeViewWithIdentifier:@"HeaderRowView" owner:self];
	if(rowView == nil)
	{
		rowView = [[RakTableRowView alloc] init];
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

- (BOOL)outlineView:(NSOutlineView *)outlineView shouldSelectItem:(id)item
{
	return NO;
}

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
