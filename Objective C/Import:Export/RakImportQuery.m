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

	CGFloat bordersY[3];	//Y pos of borders to draw
}

@end

enum
{
	META_TOP_BORDER = 8,
	META_BORDER_WIDTH = 8,
	META_TOP_FORM_BORDER = 10,
	META_INTERLINE_BORDER = 12
};

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
	return [[self initWithFrame:NSMakeRect(0, 0, 350, 500)] _autoInitWithMetadata:project];
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

	RakText * header = [[RakText alloc] initWithText:NSLocalizedString(_item.isTome ? @"IMPORT-VOL-OVERRIDE" : @"IMPORT-CHAP-OVERRIDE" , nil) :[self titleColor]];
	if(header != nil)
	{
		if(selfSize.width < header.bounds.size.width)
		{
			[self setFrameSize:NSMakeSize(header.bounds.size.width + 20, selfSize.height)];
			selfSize.width = _bounds.size.width;
		}

		[header setFrameOrigin:NSMakePoint(selfSize.width / 2 - header.bounds.size.width / 2, (currentY -= header.bounds.size.height + 8))];
		[self addSubview:header];
	}

	RakButton * discard = [RakButton allocWithText:NSLocalizedString(@"IMPORT-KEEP", nil)], * replace = [RakButton allocWithText:NSLocalizedString(@"IMPORT-REPLACE", nil)], * replaceAll = [RakButton allocWithText:NSLocalizedString(@"IMPORT-REPLACE-ALL", nil)];

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
	NSSize selfSize = self.frame.size, titleSize, inputSize;
	CGFloat maxWidthTitles = selfSize.width / 5, maxWidthContent = selfSize.width * 7 / 10, currentHeight;

	RakText * header = [[RakText alloc] initWithText:NSLocalizedString(@"IMPORT-META-HEAD", nil) :[self titleColor]], * title;
	if(header != nil)
	{
		header.cell.wraps = YES;
		header.fixedWidth = selfSize.width - 20;
		header.alignment = NSTextAlignmentCenter;

		[header setFrameOrigin:NSMakePoint(selfSize.width / 2 - header.bounds.size.width / 2, (selfSize.height -= header.bounds.size.height + META_TOP_BORDER))];
		[self addSubview:header];
	}

	bordersY[0] = (selfSize.height -= META_TOP_FORM_BORDER);

	selfSize.height = [self insertTopThreeTextOnly : selfSize.height - META_TOP_FORM_BORDER + META_INTERLINE_BORDER : maxWidthTitles : maxWidthContent];

	bordersY[1] = (selfSize.height -= META_TOP_FORM_BORDER);

	//Right to left or left to right?
	title = [self getTextForLocalizationString:@"IMPORT-META-READ-ORDER" :maxWidthTitles];
	RakSegmentedControl * switchButton = [[RakSegmentedControl alloc] initWithFrame:NSZeroRect :@[@"A → B", @"B ← A"]];
	if(title != nil && switchButton != nil)
	{
		[switchButton setEnabled:YES forSegment:0];
		[switchButton setEnabled:YES forSegment:1];

		[switchButton setSelected:YES forSegment:_project.rightToLeft];

		titleSize = title.bounds.size;
		inputSize = switchButton.bounds.size;

		currentHeight = MAX(titleSize.height, inputSize.height);
		selfSize.height -= currentHeight + META_TOP_FORM_BORDER;

		[title setFrameOrigin:NSMakePoint(META_BORDER_WIDTH + maxWidthTitles - title.bounds.size.width, selfSize.height + currentHeight / 2 - titleSize.height / 2)];
		[switchButton setFrameOrigin:NSMakePoint(maxWidthTitles + 2 * META_BORDER_WIDTH + (maxWidthContent / 2 - switchButton.bounds.size.width / 2), selfSize.height + currentHeight / 2 - inputSize.height / 2)];

		[self addSubview:title];
		[self addSubview:switchButton];
	}

	//Status of the project
	title = [self getTextForLocalizationString:@"IMPORT-META-STATUS" :maxWidthTitles];
	if(title != nil)
	{
		titleSize = title.bounds.size;
		inputSize = NSZeroSize;

		currentHeight = MAX(titleSize.height, inputSize.height);
		selfSize.height -= currentHeight + META_INTERLINE_BORDER;

		[title setFrameOrigin:NSMakePoint(META_BORDER_WIDTH + maxWidthTitles - title.bounds.size.width, selfSize.height + currentHeight / 2 - titleSize.height / 2)];
		[self addSubview:title];
	}

	//Tagging of the project
	title = [self getTextForLocalizationString:@"IMPORT-META-TAG" :maxWidthTitles];
	if(title != nil)
	{
		titleSize = title.bounds.size;
		inputSize = NSZeroSize;

		currentHeight = MAX(titleSize.height, inputSize.height);
		selfSize.height -= currentHeight + META_INTERLINE_BORDER;

		[title setFrameOrigin:NSMakePoint(META_BORDER_WIDTH + maxWidthTitles - title.bounds.size.width, selfSize.height + currentHeight / 2 - titleSize.height / 2)];
		[self addSubview:title];
	}

	bordersY[2] = (selfSize.height -= META_TOP_FORM_BORDER);

	//Image for the grid of the project
	title = [self getTextForLocalizationString:@"IMPORT-META-IMG-GRID" :maxWidthTitles];
	if(title != nil)
	{
		titleSize = title.bounds.size;
		inputSize = NSZeroSize;

		currentHeight = MAX(titleSize.height, inputSize.height);
		selfSize.height -= currentHeight + META_TOP_FORM_BORDER;

		[title setFrameOrigin:NSMakePoint(META_BORDER_WIDTH + maxWidthTitles - title.bounds.size.width, selfSize.height + currentHeight / 2 - titleSize.height / 2)];
		[self addSubview:title];
	}

	//Image for D&D of the project
	title = [self getTextForLocalizationString:@"IMPORT-META-IMG-DD" :maxWidthTitles];
	if(title != nil)
	{
		titleSize = title.bounds.size;
		inputSize = NSZeroSize;

		currentHeight = MAX(titleSize.height, inputSize.height);
		selfSize.height -= currentHeight + META_INTERLINE_BORDER;

		[title setFrameOrigin:NSMakePoint(META_BORDER_WIDTH + maxWidthTitles - title.bounds.size.width, selfSize.height + currentHeight / 2 - titleSize.height / 2)];
		[self addSubview:title];
	}

	//Image for CT of the project
	title = [self getTextForLocalizationString:@"IMPORT-META-IMG-CT" :maxWidthTitles];
	if(title != nil)
	{
		titleSize = title.bounds.size;
		inputSize = NSZeroSize;

		currentHeight = MAX(titleSize.height, inputSize.height);
		selfSize.height -= currentHeight + META_INTERLINE_BORDER;

		[title setFrameOrigin:NSMakePoint(META_BORDER_WIDTH + maxWidthTitles - title.bounds.size.width, selfSize.height + currentHeight / 2 - titleSize.height / 2)];
		[self addSubview:title];
	}

	selfSize.height -= 2 * META_TOP_FORM_BORDER;

	RakButton * button = [RakButton allocWithText:NSLocalizedString(@"CLOSE", nil)];
	if(button != nil)
	{
		button.target = self;
		button.action = @selector(close);

		titleSize = button.bounds.size;

		[button setFrameOrigin:NSMakePoint(selfSize.width / 4 - titleSize.width / 2, selfSize.height - titleSize.height)];

		[self addSubview:button];
	}

	button = [RakButton allocWithText:NSLocalizedString(@"CONFIRM", nil)];
	if(button != nil)
	{
		button.target = self;
		button.action = @selector(validateField);

		titleSize = button.bounds.size;

		[button setFrameOrigin:NSMakePoint(3 * selfSize.width / 4 - titleSize.width / 2, selfSize.height - titleSize.height)];

		[self addSubview:button];
	}
}

- (CGFloat) insertTopThreeTextOnly : (CGFloat) currentY : (CGFloat) maxWidthTitles : (CGFloat) maxWidthContent
{
	NSArray * titleNames = @[@"IMPORT-META-TITLE", @"IMPORT-META-AUTHOR", @"IMPORT-META-DESCRIPTION"], * placeholderNames = @[@"IMPORT-META-TITLE-PH", @"IMPORT-META-AUTHOR-PH", @"IMPORT-META-DESCRIPTION-PH"];
	charType * collector[3] = {(void*) &_project.projectName, (void*) &_project.authorName, (void*) &_project.description};

	for(byte i = 0; i < 3; i++)
	{
		RakText * title = [self getTextForLocalizationString:titleNames[i] :maxWidthTitles];
		RakText * inputField = [self getInputFieldWithPlaceholder:placeholderNames[i] : maxWidthContent];
		if(title != nil && inputField != nil)
		{
			NSSize titleSize = title.bounds.size, inputSize = inputField.bounds.size;

			if(collector[i][0])
				inputField.stringValue = getStringForWchar(collector[i]);

			//Synopsis, this one is slightly different
			if(i == 2)
			{
				inputField.cell.wraps = YES;
				inputField.fixedWidth = inputSize.width;
				inputField.enableMultiLine = YES;

				[inputField setFrameSize: [inputField intrinsicContentSize]];

				inputSize.height *= 5;

				RakListScrollView * view = [[RakListScrollView alloc] initWithFrame:NSMakeRect(0, 0, inputSize.width + [RakScroller width], inputSize.height)];

				view.drawsBackground = YES;
				view.backgroundColor = inputField.backgroundColor;
				view.documentView = inputField;

				((RakScroller *) view.verticalScroller).hideScroller = YES;
				((RakScroller *) view.verticalScroller).backgroundColorToReplicate = [self backgroundColor];

				inputField = (id) view;
			}

			CGFloat currentHeight = MAX(titleSize.height, inputSize.height);
			currentY -= currentHeight + META_INTERLINE_BORDER;

			if(i == 2)
				[title setFrameOrigin:NSMakePoint(META_BORDER_WIDTH + maxWidthTitles - title.bounds.size.width, currentY + currentHeight - titleSize.height)];
			else
				[title setFrameOrigin:NSMakePoint(META_BORDER_WIDTH + maxWidthTitles - title.bounds.size.width, currentY + currentHeight / 2 - titleSize.height / 2)];

			[inputField setFrameOrigin:NSMakePoint(maxWidthTitles + 2 * META_BORDER_WIDTH, currentY + currentHeight / 2 - inputSize.height / 2)];

			[self addSubview:title];
			[self addSubview:inputField];
		}
	}

	return currentY;
}

#pragma mark - Color management & UI generation

- (NSColor *) titleColor
{
	return [Prefs getSystemColor:COLOR_CLICKABLE_TEXT :nil];
}

- (RakText *) getTextForLocalizationString : (NSString *) string : (CGFloat) maxWidth
{
	RakText * text = [[RakText alloc] initWithText:NSLocalizedString(string, nil) :[self titleColor]];
	if(text != nil)
	{
		text.cell.wraps = YES;
		text.fixedWidth = maxWidth;
		text.alignment = NSTextAlignmentRight;
	}

	return text;
}

- (RakText *) getInputFieldWithPlaceholder : (NSString *) string : (CGFloat) maxWidth
{
	RakText * text = [[RakText alloc] initWithText:nil :[self titleColor]];
	if(text != nil)
	{
		((RakTextCell*) text.cell).customizedInjectionPoint = YES;
		((RakTextCell*) text.cell).centered = YES;

		text.drawsBackground = YES;
		text.backgroundColor = [Prefs getSystemColor:COLOR_BACKGROUND_TEXTFIELD :nil];

		[text setFrameSize:NSMakeSize(maxWidth, text.bounds.size.height)];
		[text setPlaceholderString:NSLocalizedString(string, nil)];
		[text setEditable:YES];
	}

	return text;
}

- (void) feedAnimationController : (RakCTAnimationController *) animationController
{
}

- (void) drawRect:(NSRect)dirtyRect
{
	[super drawRect:dirtyRect];

	if(!requestingMetadata)
		return;

	[[[NSColor whiteColor] colorWithAlphaComponent:0.3] setFill];

	dirtyRect.origin.x = dirtyRect.size.width / 6;
	dirtyRect.size.width = 4 * dirtyRect.origin.x;
	dirtyRect.size.height = 1;

	for(byte i = 0; i < 3; ++i)
	{
		if(bordersY[i] == 0)
			continue;

		dirtyRect.origin.y = bordersY[i];
		NSRectFill(dirtyRect);
	}
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

- (void) validateField
{

}

- (void) close
{

}

@end
