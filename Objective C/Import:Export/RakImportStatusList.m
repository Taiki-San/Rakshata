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

@interface RakImportStatusListRowView : NSView
{
	RakImportItem * _item;
	BOOL isRoot;

	RakText * projectName;
	RakStatusButton * button;
}

- (void) updateWithItem : (RakImportStatusListItem *) item;

@end

enum
{
	LIST_WIDTH = 350,
	ROW_HEIGHT = 30
};

@implementation RakImportStatusListRowView

- (void) updateWithItem : (RakImportStatusListItem *) item
{
	_item = item.itemForChild;
	isRoot = item.isRootItem;

	if(projectName == nil)
	{
		projectName = [[RakText alloc] initWithText:[self getLineName : item] :[NSColor whiteColor]];
		if(projectName != nil)
			[self addSubview:projectName];
	}
	else
	{
		projectName.stringValue = [self getLineName : item];
		[projectName sizeToFit];
	}

	if(button == nil)
	{
		button = [[RakStatusButton alloc] initWithStatus:[self determineStatus : item]];
		if(button != nil)
			[self addSubview:button];
	}
	else
		button.status = [self determineStatus : item];

	button.stringValue = [self determineMessageForStatus : button.status andItem:item];

	//Refresh everything's position
	if(!NSEqualSizes(_bounds.size, NSZeroSize))
		[self setFrameSize:_bounds.size];
}

- (void) setFrameSize:(NSSize)newSize
{
	[super setFrameSize:newSize];

	newSize = _bounds.size;

	[projectName setFrameOrigin:NSMakePoint(20, newSize.height / 2 - projectName.bounds.size.height / 2)];

	NSSize itemSize = button.bounds.size;
	[button setFrameOrigin:NSMakePoint(newSize.width - itemSize.width - (isRoot ? 18 : 20), newSize.height / 2 - itemSize.height / 2)];

}

#pragma mark - Logic

- (NSString *) getLineName : (RakImportStatusListItem *) item
{
	if(isRoot)
		return getStringForWchar(item.projectForRoot.projectName);

	if(_item.isTome)
	{
		META_TOME metadata = _item.projectData.data.tomeLocal[0];

		if(metadata.readingID != INVALID_SIGNED_VALUE && metadata.readingName[0])
		{
			return [NSString stringWithFormat:@"%@ - %@", [NSString localizedStringWithFormat:NSLocalizedString(@"VOLUME-%d", nil), metadata.readingID], getStringForWchar(metadata.readingName)];
		}
		else if(metadata.readingID != INVALID_SIGNED_VALUE)
			return [NSString localizedStringWithFormat:NSLocalizedString(@"VOLUME-%d", nil), metadata.readingID];

		return getStringForWchar(metadata.readingName);
	}

	int content = _item.contentID;

	if(content % 10)
		return [NSString localizedStringWithFormat:NSLocalizedString(@"CHAPTER-%d.%d", nil), content / 10, content % 10];

	return [NSString localizedStringWithFormat:NSLocalizedString(@"CHAPTER-%d", nil), content / 10];
}

- (byte) determineStatus : (RakImportStatusListItem *) item
{
	if(!isRoot)
		return _item.issue == IMPORT_PROBLEM_NONE ? STATUS_BUTTON_OK : STATUS_BUTTON_ERROR;

	return [item checkStatusFromChildren];
}

- (NSString *) determineMessageForStatus : (byte) status andItem : (RakImportStatusListItem *) item
{
	if(status == STATUS_BUTTON_OK || rand() % 2)
		return @"Tout est bon :)";

	return @"Meh";
}

@end

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
			{
				NSLog(@"WTF?");
				continue;
			}

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
	}

	[rowView updateWithItem :item];

	return rowView;
}

@end

@implementation RakImportStatusListItem

- (instancetype) initWithProject : (PROJECT_DATA) project
{
	self = [super init];

	if(self != nil)
	{
		_isRootItem = YES;
		nullifyCTPointers(&project);
		_projectForRoot = project;

		children = [NSMutableArray new];
	}

	return self;
}

- (instancetype) initWithItem : (RakImportItem *) item
{
	self = [super init];

	if(self != nil)
	{
		_isRootItem = NO;
		_itemForChild = item;
	}

	return self;
}

- (void) addItemAsChild : (RakImportItem *) item
{
	[children addObject:[[RakImportStatusListItem alloc] initWithItem:item]];
}

- (void) commitFinalList
{
	[children sortUsingComparator:^NSComparisonResult(RakImportStatusListItem * obj1, RakImportStatusListItem * obj2) {

		RakImportItem * first = obj1.itemForChild, * second = obj2.itemForChild;

		if(first.isTome ^ second.isTome)
			return first.isTome ? NSOrderedAscending : NSOrderedDescending;

		return first.contentID > second.contentID ? NSOrderedDescending : NSOrderedAscending;
	}];

	_nbChildren = [children count];
}

- (byte) checkStatusFromChildren
{
	BOOL anythingWrong = NO, everythingWrong = YES;
	for(RakImportStatusListItem * item in children)
	{
		BOOL status = item.itemForChild.issue != IMPORT_PROBLEM_NONE;

		anythingWrong |= status;
		everythingWrong &= status;
	}

	return anythingWrong ? (everythingWrong ? STATUS_BUTTON_ERROR : STATUS_BUTTON_WARN) : STATUS_BUTTON_OK;
}

@end