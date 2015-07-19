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

@interface RakImportStatusListRowView()
{
	RakImportStatusListItem * listItem;

	RakImportItem * _item;
	BOOL isRoot;

	RakText * projectName;
	RakStatusButton * button;

	__weak RakImportQuery * alert;
}

@property RakImportStatusList * list;

- (void) updateWithItem : (RakImportStatusListItem *) item;
- (byte) status;

@end

@interface RakImportStatusList()

@end

enum
{
	LIST_WIDTH = 350,
	ROW_HEIGHT = 30
};

#define NOTIFICATION_CHILD	@"RakImportStatusUpdateChild"
#define NOTIFICATION_ROOT	@"RakImportStatusUpdateRoot"

@implementation RakImportStatusListRowView

- (instancetype) initWithFrame : (NSRect) frame
{
	self = [super initWithFrame : frame];

	if(self != nil)
	{
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(checkRefreshStatus) name:NOTIFICATION_IMPORT_STATUS_UI object:nil];
	}

	return self;
}

- (void) dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void) updateWithItem : (RakImportStatusListItem *) item
{
	listItem = item;
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
		button = [[RakStatusButton alloc] initWithStatus:item.status];
		if(button != nil)
		{
			button.target = self;
			button.action = @selector(getDetails);
			[self addSubview:button];
		}
	}
	else
		button.status = item.status;

	button.stringValue = [self determineMessageForStatus : button.status andItem:item];

	//Refresh everything's position
	if(!NSEqualSizes(_bounds.size, NSZeroSize))
		[self setFrameSize:_bounds.size];
}

- (byte) status
{
	return button.status;
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

- (NSString *) determineMessageForStatus : (byte) status andItem : (RakImportStatusListItem *) item
{
	if(status == STATUS_BUTTON_OK)
		return @"Tout est bon 😊";

	else if(status == STATUS_BUTTON_WARN || (item.isRootItem && item.itemForChild.issue != IMPORT_PROBLEM_METADATA))
		return @"Problèmes detectés 😕";

	//Ok, error
	else if(item.itemForChild.issue == IMPORT_PROBLEM_DUPLICATE)
		return @"Duplicat detecté 😱";

	else if(item.itemForChild.issue == IMPORT_PROBLEM_INSTALL_ERROR)
		return [NSString stringWithFormat:@"%@ corrompu 😡", NSLocalizedString(_item.isTome ? @"VOLUME" : @"CHAPTER", nil)];

	return @"Données incomplètes 😱";
}

- (void) checkRefreshStatus
{
	byte oldStatus = button.status;
	button.status = listItem.status;

	button.stringValue = [self determineMessageForStatus : button.status andItem:listItem];

	if(oldStatus != button.status)
		[self setNeedsDisplay:YES];

	if(button.status != STATUS_BUTTON_ERROR && _list.query == alert)
		_list.query = alert = nil;
}

- (void) getDetails
{
	if(isRoot)
		return;

	_list.query = alert = [[RakImportQuery alloc] autoInitWithItem:_item];
	[alert launchPopover:button :self];
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

			_haveDuplicate |= item.issue == IMPORT_PROBLEM_DUPLICATE;
		}

		[rootCollector enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL * stop) {	[obj commitFinalList];	}];
		rootItems = [NSArray arrayWithArray:rootCollector];

		_nbRoot = [rootItems count];

		[self initializeMain:NSMakeRect(0, 0, LIST_WIDTH, 200)];
		[content setHeaderView:nil];

		//Expand what have to be
		[rootCollector enumerateObjectsUsingBlock:^(RakImportStatusListItem * obj, NSUInteger idx, BOOL * stop) {

			if(obj.status != STATUS_BUTTON_OK && [obj getNbChildren] < 10)
				[content expandItem:obj];

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

- (void) setQuery : (RakImportQuery *) newQuery
{
	if(_query != nil && _query != newQuery)
		[_query closePopover];

	_query = newQuery;
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

		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(checkRefreshStatusRoot) name:NOTIFICATION_ROOT object:nil];
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
		_status = self.itemForChild.issue == IMPORT_PROBLEM_NONE ? STATUS_BUTTON_OK : STATUS_BUTTON_ERROR;

		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(checkRefreshStatusChild) name:NOTIFICATION_CHILD object:nil];
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
	_status = [self checkStatusFromChildren];
}

- (byte) checkStatusFromChildren
{
	BOOL anythingWrong = NO, everythingWrong = YES;
	for(RakImportStatusListItem * item in children)
	{
		BOOL itemStatus = item.itemForChild.issue != IMPORT_PROBLEM_NONE;

		anythingWrong |= itemStatus;
		everythingWrong &= itemStatus;
	}

	return anythingWrong ? (everythingWrong ? STATUS_BUTTON_ERROR : STATUS_BUTTON_WARN) : STATUS_BUTTON_OK;
}

#pragma mark - Content update

- (void) checkRefreshStatusChild
{
	_status = _itemForChild.issue == IMPORT_PROBLEM_NONE ? STATUS_BUTTON_OK : STATUS_BUTTON_ERROR;
}

- (void) checkRefreshStatusRoot
{
	_status = [self checkStatusFromChildren];
}

@end