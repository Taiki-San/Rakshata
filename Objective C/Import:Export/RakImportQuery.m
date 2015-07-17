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

@interface RakImportQuery()
{
	RakImportItem * _item;
	BOOL requestingMetadata;

	RakText * header;
}

@end

@implementation RakImportQuery

- (instancetype) autoInitWithItem : (RakImportItem *) item
{
	if(item == nil || item.issue == IMPORT_PROBLEM_NONE)
		return nil;

	return [[self initWithFrame:NSMakeRect(0, 0, 300, 50)] _autoInitWithItem:item];
}

- (BOOL) launchPopover : (NSView *) anchor : (RakImportStatusListRowView*) receiver
{
	[self internalInit:anchor :NSZeroRect :YES];
	return YES;
}

- (void) setupView
{
	if(requestingMetadata)
		[self setupUIMetadata];
	else
		[self setupUIDuplicate];
}

- (instancetype) _autoInitWithItem : (RakImportItem *) item
{
	_item = item;
	requestingMetadata = _item.issue == IMPORT_PROBLEM_METADATA;
	return self;
}

- (void) setupUIDuplicate
{
	NSSize selfSize = _bounds.size;
	CGFloat currentY = selfSize.height;

	header = [[RakText alloc] initWithText:@"CT déjà existant, souhaitez vous le remplacer?" :[Prefs getSystemColor:COLOR_CLICKABLE_TEXT :nil]];
	if(header != nil)
	{
		[self addSubview:header];
		[header setFrameOrigin:NSMakePoint(selfSize.width / 2 - header.bounds.size.width / 2, (currentY -= header.bounds.size.height + 5))];
	}
}

- (void) setupUIMetadata
{

}

#pragma mark - Popover interaction

- (INPopoverArrowDirection) arrowDirection
{
	return INPopoverArrowDirectionLeft;
}

- (void) configurePopover:(INPopoverController *)internalPopover
{
	[super configurePopover:internalPopover];

	internalPopover.closesWhenApplicationBecomesInactive = NO;
	internalPopover.closesWhenPopoverResignsKey = NO;
}

- (void) additionalUpdateOnThemeChange
{
	[super additionalUpdateOnThemeChange];
}

@end
