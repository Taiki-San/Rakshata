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
	PROJECT_DATA _project;
	BOOL requestingMetadata;
}

@end

@implementation RakImportQuery

- (instancetype) autoInitWithDuplicate : (RakImportItem *) item
{
	if(item == nil || item.issue != IMPORT_PROBLEM_METADATA)
		return nil;

	return [[self initWithFrame:NSMakeRect(0, 0, 300, 64)] _autoInitWithDuplicate:item];
}

- (instancetype) _autoInitWithDuplicate : (RakImportItem *) item
{
	_item = item;
	requestingMetadata = NO;
	return self;
}

- (instancetype) autoInitWithMetadata : (PROJECT_DATA) project
{
	return [[self initWithFrame:NSMakeRect(0, 0, 300, 400)] _autoInitWithMetadata:project];
}

- (instancetype) _autoInitWithMetadata : (PROJECT_DATA) project
{
	_project = project;
	requestingMetadata = YES;
	return self;
}

- (BOOL) launchPopover : (NSView *) anchor : (RakImportStatusListRowView*) receiver
{
	[self internalInit:anchor :NSZeroRect :NO];
	return YES;
}

- (void) setupView
{
	if(requestingMetadata)
		[self setupUIMetadata];
	else
		[self setupUIDuplicate];
}

- (void) setupUIDuplicate
{
	NSSize selfSize = self.frame.size;
	CGFloat currentY = selfSize.height;

	RakText * header = [[RakText alloc] initWithText:@"CT déjà existant, souhaitez vous le remplacer?" :[Prefs getSystemColor:COLOR_CLICKABLE_TEXT :nil]];
	if(header != nil)
	{
		[header setFrameOrigin:NSMakePoint(selfSize.width / 2 - header.bounds.size.width / 2, (currentY -= header.bounds.size.height + 8))];
		[self addSubview:header];
	}

	RakButton * discard = [RakButton allocWithText:@"Conserver"], * replace = [RakButton allocWithText:@"Remplacer"], * replaceAll = [RakButton allocWithText:@"Remplacer série"];

	if(discard != nil && replace != nil && replaceAll != nil)
	{
		CGFloat totalWidth = discard.bounds.size.width + 5 + replace.bounds.size.width + 5 + replaceAll.bounds.size.width, baseWidth = _bounds.size.width / 2 - totalWidth / 2, height = currentY / 2;

		discard.action = @selector(discard);
		replace.action = @selector(replace);
		replaceAll.action = @selector(replaceAll);

		for(RakButton * button in @[discard, replace, replaceAll])
		{
			[button setFrameOrigin:NSMakePoint(baseWidth, height - button.bounds.size.height / 2)];
			baseWidth += 5 + button.bounds.size.width;

			button.target = self;

			[self addSubview:button];
		}
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

- (void) additionalUpdateOnThemeChange
{
	[super additionalUpdateOnThemeChange];
}

#pragma mark - Button responder

- (void) replaceAll
{
	[[NSNotificationCenter defaultCenter] postNotificationName:NOTIFICATION_IMPORT_REPLACE_ALL object:_item];
}

- (void) replace
{
	[[NSNotificationCenter defaultCenter] postNotificationName:NOTIFICATION_IMPORT_REPLACE_ONE object:_item];
}

- (void) discard
{
	_item.issue = IMPORT_PROBLEM_NONE;
	[RakImportStatusList refreshAfterPass];
}

@end
