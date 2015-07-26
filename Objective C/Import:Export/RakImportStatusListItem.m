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

@implementation RakImportStatusListItem

- (instancetype) initWithProject : (PROJECT_DATA) project
{
	self = [super init];

	if(self != nil)
	{
		_isRootItem = YES;
		nullifyCTPointers(&project);
		_projectData = project;

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
		_projectData = _itemForChild.projectData.data.project;
		_status = self.itemForChild.issue == IMPORT_PROBLEM_NONE ? STATUS_BUTTON_OK : STATUS_BUTTON_ERROR;
		_metadataProblem = _status == IMPORT_PROBLEM_METADATA;

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
	_metadataProblem = NO;

	for(RakImportStatusListItem * item in children)
	{
		BOOL itemStatus = item.itemForChild.issue != IMPORT_PROBLEM_NONE;

		anythingWrong |= itemStatus;
		everythingWrong &= itemStatus;

		if(itemStatus && item.itemForChild.issue == IMPORT_PROBLEM_METADATA)
			_metadataProblem = YES;
	}

	return anythingWrong ? (everythingWrong ? STATUS_BUTTON_ERROR : STATUS_BUTTON_WARN) : STATUS_BUTTON_OK;
}

#pragma mark - Content update

- (void) checkRefreshStatusChild
{
	_metadataProblem = _itemForChild.issue == IMPORT_PROBLEM_METADATA;
	_status = _itemForChild.issue == IMPORT_PROBLEM_NONE ? STATUS_BUTTON_OK : STATUS_BUTTON_ERROR;
}

- (void) checkRefreshStatusRoot
{
	_status = [self checkStatusFromChildren];
}

@end