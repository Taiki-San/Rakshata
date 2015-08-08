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
		return getStringForWchar(item.projectData.projectName);

	if(_item.isTome)
	{
		META_TOME metadata = _item.projectData.data.tomeLocal[0];

		if(metadata.readingID != INVALID_SIGNED_VALUE && metadata.readingName[0])
		{
			return [NSString stringWithFormat:@"%@ - %@", [NSString localizedStringWithFormat:NSLocalizedString(@"VOLUME-%d", nil), metadata.readingID], getStringForWchar(metadata.readingName)];
		}
		else if(metadata.readingID != INVALID_SIGNED_VALUE)
			return [NSString localizedStringWithFormat:NSLocalizedString(@"VOLUME-%d", nil), metadata.readingID];
		else
			return _item.path;

		return getStringForWchar(metadata.readingName);
	}

	int content = _item.contentID;

	if(content == INVALID_SIGNED_VALUE)
		return _item.path;

	if(content % 10)
		return [NSString localizedStringWithFormat:NSLocalizedString(@"CHAPTER-%d.%d", nil), content / 10, content % 10];

	return [NSString localizedStringWithFormat:NSLocalizedString(@"CHAPTER-%d", nil), content / 10];
}

- (NSString *) determineMessageForStatus : (byte) status andItem : (RakImportStatusListItem *) item
{
	if(status == STATUS_BUTTON_OK)
		return @"Tout est bon 😊";

	if(item.isRootItem && item.metadataProblem)
		return @"Données incomplètes 😱";

	else if(item.isRootItem || status == STATUS_BUTTON_WARN)
		return @"Problèmes detectés 😕";

	//Ok, error
	else if(item.itemForChild.issue == IMPORT_PROBLEM_DUPLICATE)
		return @"Duplicat detecté 😱";

	else if(item.itemForChild.issue == IMPORT_PROBLEM_INSTALL_ERROR)
		return [NSString stringWithFormat:@"%@ corrompu 😡", NSLocalizedString(_item.isTome ? @"VOLUME" : @"CHAPTER", nil)];

	else if(item.itemForChild.issue == IMPORT_PROBLEM_METADATA_DETAILS)
		return @"Détails manquants 😔";

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
	if(isRoot && !listItem.metadataProblem)
		return;

	if(listItem.metadataProblem)
	{
		_list.query = alert = [[RakImportQuery alloc] autoInitWithMetadata:listItem.projectData];
		if(alert != nil)
		{
			if(isRoot)
			{
				RakImportStatusListItem * child = [listItem getChildAtIndex:0];
				if(child != nil)
					alert.itemOfQueryForMetadata = child.itemForChild;
			}
			else
				alert.itemOfQueryForMetadata = _item;
		}
	}
	else if(_item.issue == IMPORT_PROBLEM_DUPLICATE)
		_list.query = alert = [[RakImportQuery alloc] autoInitWithDuplicate:_item];

	else if(_item.issue == IMPORT_PROBLEM_METADATA_DETAILS)
	{

	}

	[alert launchPopover:button :self];
}

@end

