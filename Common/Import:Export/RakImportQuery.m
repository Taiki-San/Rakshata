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

@interface RakImportQuery()
{
	RakImportItem * _item;
	byte issue;

	//Model
	CATEGORY_VERBOSE * cats;
	TAG_VERBOSE * tags;
	uint nbCats, nbTags;

	CGFloat bordersY[4];	//Y pos of borders to draw

	//Fields to recover metadata data
	RakText * name, * author, * description;
	RakSegmentedControl * rightToLeft;
	RakPopUpButton * status, * tagList, * catList;
	RakImageDropArea * dropSR, * dropCT, * dropDD;

	//Fields to recover details
	RakText * detailHeader, * contentIDTitle, * contentID, * contentSeparator, * contentName;
	RakButton * detailConfirm;
	RakSegmentedControl * isTome;
	CGFloat maxContentIDWidth;
	
	bool originalIsArchived;
	uint guessedCacheID;
	PROJECT_DATA archivedProject;
	NSArray * archivedThumbs;
}

@end

@interface RakCTFormatter : NSNumberFormatter

@end

enum
{
	META_TOP_BORDER = 8,
	META_BORDER_WIDTH = 8,
	META_TOP_FORM_BORDER = 10,
	META_LARGE_INTERLINE_BORDER = 10,
	META_INTERLINE_BORDER = 8,
	META_SMALL_INTERLINE_BORDER = 6,
	BORDER = 4
};

@implementation RakImportQuery

#pragma mark Duplicate management

- (instancetype) autoInitWithDuplicate : (RakImportItem *) item
{
	if(item == nil || item.issue != IMPORT_PROBLEM_DUPLICATE)
		return nil;

	return [[self initWithFrame:NSMakeRect(0, 0, 300, 64)] _autoInitWithDuplicate:item];
}

- (instancetype) _autoInitWithDuplicate : (RakImportItem *) item
{
	_item = item;
	issue = IMPORT_PROBLEM_DUPLICATE;
	return self;
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

		[header setFrameOrigin:NSMakePoint(selfSize.width / 2 - header.bounds.size.width / 2, (currentY -= header.bounds.size.height + META_TOP_BORDER))];
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

#pragma mark Details management

- (instancetype) autoInitWithDetails : (RakImportItem *) item
{
	if(item == nil)
		return nil;

	return [[self initWithFrame:NSMakeRect(0, 0, 300, 143)] _autoInitWithDetails:item];
}

- (instancetype) _autoInitWithDetails : (RakImportItem *) item
{
	_item = item;
	issue = IMPORT_PROBLEM_METADATA_DETAILS;
	return self;
}

- (void) setupUIDetails
{
	NSSize selfSize = self.frame.size, workingSize;
	CGFloat currentY = selfSize.height;

	//Header
	detailHeader = [[RakText alloc] initWithText:NSLocalizedString(_item.isTome ? @"IMPORT-DET-HEAD-VOL" : @"IMPORT-DET-HEAD-CHAP", nil) :[self titleColor]];
	if(detailHeader != nil)
	{
		workingSize = detailHeader.bounds.size;

		if(selfSize.width < workingSize.width)
		{
			[self setFrameSize:NSMakeSize(workingSize.width + 20, selfSize.height)];
			selfSize.width = _bounds.size.width;
		}

		[detailHeader setFrameOrigin:NSMakePoint(selfSize.width / 2 - workingSize.width / 2, (currentY -= workingSize.height + META_TOP_BORDER))];
		[self addSubview:detailHeader];
	}

	//Border over input fields
	bordersY[0] = (currentY -= META_LARGE_INTERLINE_BORDER);

	//C/T toggle
	isTome = [[RakSegmentedControl alloc] initWithFrame:NSZeroRect :@[NSLocalizedString(@"CHAPTER", nil), NSLocalizedString(@"VOLUME", nil)]];
	if(isTome != nil)
	{
		[isTome setEnabled:YES forSegment:0];
		[isTome setEnabled:YES forSegment:1];
		[isTome setSelectedSegment:_item.isTome];

		isTome.target = self;
		isTome.action = @selector(changedIsTome);
		
		workingSize = isTome.bounds.size;
		[isTome setFrameOrigin:NSMakePoint(selfSize.width / 2 - workingSize.width / 2, currentY -= workingSize.height + META_LARGE_INTERLINE_BORDER)];
		[self addSubview:isTome];
	}

	const CGFloat maxWidthTitles = selfSize.width / 5, maxWidthContent = selfSize.width * 13 / 20;

	//Field for the number of the CT
	contentIDTitle = [self getTextForLocalizationString:(_item.isTome ? @"VOLUME" : @"CHAPTER") :maxWidthTitles + META_BORDER_WIDTH];
	contentID = [self getInputFieldWithPlaceholder:@"IMPORT-DET-PH-NUMBER" : maxWidthContent];
	if(contentIDTitle != nil && contentID != nil)
	{
		contentIDTitle.stringValue = [contentIDTitle.stringValue stringByAppendingString:@" #"];
		contentID.formatter = [[RakCTFormatter alloc] init];

		NSSize titleSize = contentIDTitle.bounds.size;
		workingSize = contentID.bounds.size;

		maxContentIDWidth = workingSize.width;

		const CGFloat currentHeight = MAX(titleSize.height, workingSize.height);
		currentY -= currentHeight + META_INTERLINE_BORDER;

		[contentIDTitle setFrameOrigin:NSMakePoint(0, currentY + currentHeight / 2 - titleSize.height / 2)];
		[contentID setFrameOrigin:NSMakePoint(maxWidthTitles + META_BORDER_WIDTH + 4, currentY + currentHeight / 2 - workingSize.height / 2)];
		
		PROJECT_DATA_PARSED projectData = _item.projectData.data;
		if(_item.isTome)
		{
			if(projectData.nbVolumesLocal > 0 && projectData.tomeLocal[0].readingID != INVALID_SIGNED_VALUE)
				contentID.stringValue = getStringForCTID(projectData.tomeLocal[0].readingID);
		}
		else
		{
			if(projectData.nbChapterLocal > 0 && projectData.chaptersLocal[0] != INVALID_VALUE)
				contentID.stringValue = getStringForCTID((int) projectData.chaptersLocal[0]);
		}

		[self addSubview:contentID];
		[self addSubview:contentIDTitle];
	}

	//The field to receive the name of the volume, hidden to the right when in chap mode
	contentSeparator = [self getTextForLocalizationString:@"-" :0];
	if(contentSeparator != nil)
		[self addSubview:contentSeparator];

	contentName = [self getInputFieldWithPlaceholder:@"IMPORT-DET-PH-NAME" :127];
	if(contentName != nil)
	{
		contentName.maxLength = MAX_TOME_NAME_LENGTH;
		
		if(_item.isTome)
		{
			PROJECT_DATA_PARSED projectData = _item.projectData.data;
			if(projectData.nbVolumesLocal > 0 && projectData.tomeLocal[0].readingName[0])
				contentID.stringValue = getStringForWchar(projectData.tomeLocal[0].readingName);
		}
		
		contentName.nextKeyView = contentID;
		[self addSubview:contentName];
	}

	//Toggle the confirmation button when text is typed to lock it when fields are empty
	if(contentID != nil || contentName != nil)
	{
		__unsafe_unretained id weakSelf = self;
		contentName.callbackOnChange = contentID.callbackOnChange = ^(NSNotification * notification, BOOL didComplete) {
			[weakSelf updateButtonStateByFields];
		};
	}

	//Enforce the positions of the field for the name
	[self positionForIsTomeState:_item.isTome withAnimation:NO];

	//Border below the input field
	currentY -= META_LARGE_INTERLINE_BORDER;
	bordersY[1] = currentY;
	currentY -= META_LARGE_INTERLINE_BORDER;

	//Confirmation/discard buttons
	RakButton * button = [RakButton allocWithText:NSLocalizedString(@"CLOSE", nil)];
	if(button != nil)
	{
		button.target = self;
		button.action = @selector(close);

		workingSize = button.bounds.size;

		[button setFrameOrigin:NSMakePoint(selfSize.width / 3 - workingSize.width / 2, currentY - workingSize.height)];

		[self addSubview:button];
	}

	detailConfirm = [RakButton allocWithText:NSLocalizedString(@"CONFIRM", nil)];
	if(detailConfirm != nil)
	{
		[self updateButtonStateByFields];

		detailConfirm.target = self;
		detailConfirm.action = @selector(validateDetail);

		workingSize = detailConfirm.bounds.size;

		[detailConfirm setFrameOrigin:NSMakePoint(2 * selfSize.width / 3 - workingSize.width / 2, currentY - workingSize.height)];

		[self addSubview:detailConfirm];
	}
}

- (void) positionForIsTomeState : (BOOL) isTomeState withAnimation : (BOOL) animate
{
	NSRect workingFrame;

	//# field
	workingFrame = contentID.frame;
	const CGFloat widthContenID = isTomeState ? 50 : maxContentIDWidth, baseY = workingFrame.origin.y, elemHeight = workingFrame.size.height;

	if(workingFrame.size.width != widthContenID)
	{
		workingFrame.size.width = widthContenID;
		[animate ? contentID.animator: contentID setFrameSize:workingFrame.size];
	}

	CGFloat width = (isTomeState ? NSMaxX(workingFrame) : _bounds.size.width) + BORDER;

	//Separator
	workingFrame = contentSeparator.frame;
	if(workingFrame.origin.x != width)
	{
		if(workingFrame.origin.y == 0)
			workingFrame.origin.y = baseY + (elemHeight / 2 - workingFrame.size.height / 2);

		[animate ? contentSeparator.animator : contentSeparator setFrameOrigin:NSMakePoint(width, workingFrame.origin.y)];
	}

	width += workingFrame.size.width + BORDER;

	//Name field
	workingFrame = contentName.frame;
	if(workingFrame.origin.x != width)
	{
		if(workingFrame.origin.y == 0)
			workingFrame.origin.y = baseY + (elemHeight / 2 - workingFrame.size.height / 2);

		[animate ? contentName.animator : contentName setFrameOrigin:NSMakePoint(width, workingFrame.origin.y)];
	}

	[self bindFieldsDetails : isTomeState];
}

- (void) bindFieldsDetails : (BOOL) isTomeState
{
	contentID.nextKeyView = isTomeState ? contentName : contentID;
}

- (void) updateButtonStateByFields
{
	detailConfirm.enabled = [contentName.stringValue length] > 0 || [contentID.stringValue length] > 0;
}

#pragma mark Metadata management

- (instancetype) autoInitWithMetadata : (PROJECT_DATA) project
{
	return [[self initWithFrame:NSMakeRect(0, 0, 350, 608)] _autoInitWithMetadata:project];
}

- (instancetype) _autoInitWithMetadata : (PROJECT_DATA) project
{
	_project = project;
	issue = IMPORT_PROBLEM_METADATA;
	return self;
}

- (void) setupUIMetadata
{
	NSSize selfSize = self.frame.size, titleSize = NSZeroSize, inputSize;
	CGFloat maxWidthTitles = selfSize.width / 5, maxWidthContent = selfSize.width * 7 / 10, currentHeight = 0;

	RakText * header = [[RakText alloc] initWithText:[self stringTitleMetadata] :[self titleColor]], * title;
	if(header != nil)
	{
		header.font = [NSFont fontWithName:[Prefs getFontName:GET_FONT_PREFS_TITLE] size:[NSFont systemFontSize]];
		header.textColor = [self mainTitleColor];

		header.enableWraps = YES;
		header.fixedWidth = selfSize.width - 20;
		header.alignment = NSTextAlignmentCenter;

		[header setFrameOrigin:NSMakePoint(selfSize.width / 2 - header.bounds.size.width / 2, (selfSize.height -= header.bounds.size.height + META_TOP_BORDER))];
		[self addSubview:header];
	}

	bordersY[0] = (selfSize.height -= META_TOP_FORM_BORDER);

	selfSize.height = [self insertTopThreeTextOnly : selfSize.height - META_TOP_FORM_BORDER + META_LARGE_INTERLINE_BORDER : maxWidthTitles : maxWidthContent];

	bordersY[1] = (selfSize.height -= META_TOP_FORM_BORDER);

	//Right to left or left to right?
	title = [self getTextForLocalizationString:@"IMPORT-META-READ-ORDER" :maxWidthTitles];
	rightToLeft = [[RakSegmentedControl alloc] initWithFrame:NSZeroRect :@[@"A → B", @"B ← A"]];
	if(title != nil && rightToLeft != nil)
	{
		[rightToLeft setEnabled:YES forSegment:0];
		[rightToLeft setEnabled:YES forSegment:1];

		[rightToLeft setSelected:YES forSegment:_project.rightToLeft];

		titleSize = title.bounds.size;
		inputSize = rightToLeft.bounds.size;

		currentHeight = MAX(titleSize.height, inputSize.height);
		selfSize.height -= currentHeight + META_TOP_FORM_BORDER;

		[title setFrameOrigin:NSMakePoint(META_BORDER_WIDTH + maxWidthTitles - titleSize.width, selfSize.height + currentHeight / 2 - titleSize.height / 2)];
		[rightToLeft setFrameOrigin:NSMakePoint(maxWidthTitles + 2 * META_BORDER_WIDTH + (maxWidthContent / 2 - inputSize.width / 2), selfSize.height + currentHeight / 2 - inputSize.height / 2)];

		[self addSubview:title];
		[self addSubview:rightToLeft];
	}

	//Status of the project
	title = [self getTextForLocalizationString:@"IMPORT-META-STATUS" :maxWidthTitles];
	status = [[RakPopUpButton alloc] initWithFrame:NSMakeRect(0, 0, maxWidthContent, currentHeight) pullsDown:NO];
	if(title != nil)
	{
		titleSize = title.bounds.size;
		inputSize = status.bounds.size;

		[status addItemsWithTitles:@[@"––",
										  NSLocalizedString(@"CT-STATUS-OVER", nil),
										  NSLocalizedString(@"CT-STATUS-CANCELLED", nil),
										  NSLocalizedString(@"CT-STATUS-PAUSE", nil),
										  NSLocalizedString(@"CT-STATUS-WIP", nil),
										  NSLocalizedString(@"CT-STATUS-ANNOUNCED", nil)]];

		[status selectItemAtIndex:_project.status <= STATUS_MAX ? _project.status : 0];

		currentHeight = MAX(titleSize.height, inputSize.height);
		selfSize.height -= META_SMALL_INTERLINE_BORDER + currentHeight;

		[title setFrameOrigin:NSMakePoint(META_BORDER_WIDTH + maxWidthTitles - titleSize.width, selfSize.height + (currentHeight - titleSize.height) / 2)];
		[status setFrameOrigin:NSMakePoint(maxWidthTitles + 2 * META_BORDER_WIDTH, selfSize.height + (currentHeight - inputSize.height) / 2)];

		[self addSubview:title];
		[self addSubview:status];
	}

	//Tagging of the project
	title = [self getTextForLocalizationString:@"IMPORT-META-TAG" :maxWidthTitles];
	tagList = [[RakPopUpButton alloc] initWithFrame:NSMakeRect(0, 0, maxWidthContent, currentHeight) pullsDown:NO];
	if(title != nil && tagList != nil)
	{
		[tagList addItemsWithTitles:[self getTagArray]];

		//We select the current item
		if(_project.mainTag != CAT_NO_VALUE)
		{
			for(uint i = 0; i < nbTags; i++)
			{
				if(tags[i].ID == _project.mainTag)
				{
					[tagList selectItemAtIndex:i];
					break;
				}
			}
		}

		titleSize = title.bounds.size;
		inputSize = tagList.bounds.size;
		currentHeight = MAX(titleSize.height, inputSize.height);
		selfSize.height -= META_SMALL_INTERLINE_BORDER + currentHeight;

		[title setFrameOrigin:NSMakePoint(META_BORDER_WIDTH + maxWidthTitles - titleSize.width, selfSize.height + (currentHeight - titleSize.height) / 2)];
		[tagList setFrameOrigin:NSMakePoint(maxWidthTitles + 2 * META_BORDER_WIDTH, selfSize.height + (currentHeight - inputSize.height) / 2)];

		[self addSubview:title];
		[self addSubview:tagList];
	}

	//Category of the project
	title = [self getTextForLocalizationString:@"IMPORT-META-CAT" :maxWidthTitles];
	catList = [[RakPopUpButton alloc] initWithFrame:NSMakeRect(0, 0, maxWidthContent, currentHeight) pullsDown:NO];
	if(title != nil && tagList != nil)
	{
		[catList addItemsWithTitles:[self getCatArray]];

		//We select the current item
		if(_project.category != CAT_NO_VALUE)
		{
			for(uint i = 0; i < nbCats; i++)
			{
				if(cats[i].ID == _project.category)
				{
					[catList selectItemAtIndex:i];
					break;
				}
			}
		}

		titleSize = title.bounds.size;
		inputSize = catList.bounds.size;

		currentHeight = MAX(titleSize.height, inputSize.height);
		selfSize.height -= META_SMALL_INTERLINE_BORDER + currentHeight;

		[title setFrameOrigin:NSMakePoint(META_BORDER_WIDTH + maxWidthTitles - titleSize.width, selfSize.height + (currentHeight - titleSize.height) / 2)];
		[catList setFrameOrigin:NSMakePoint(maxWidthTitles + 2 * META_BORDER_WIDTH, selfSize.height + (currentHeight - inputSize.height) / 2)];

		[self addSubview:title];
		[self addSubview:catList];
	}

	bordersY[2] = (selfSize.height -= META_TOP_FORM_BORDER);

	header = [self getTextForLocalizationString:@"IMPORT-META-PLZ-DRAG" :selfSize.width];
	if(header != nil)
	{
		header.alignment = NSTextAlignmentCenter;
		header.font = [NSFont fontWithName:[Prefs getFontName:GET_FONT_PREFS_TITLE] size:[NSFont systemFontSize]];
		header.textColor = [self mainTitleColor];

		[header sizeToFit];

		titleSize = header.bounds.size;

		selfSize.height -= titleSize.height + 7;

		[header setFrameOrigin:NSMakePoint(selfSize.width / 2 - titleSize.width / 2, selfSize.height)];
		[self addSubview:header];

	}

	bordersY[3] = (selfSize.height -= META_TOP_FORM_BORDER);

	//Image for the grid of the project
	title = [self getTextForLocalizationString:@"IMPORT-META-IMG-GRID" :maxWidthTitles];
	dropSR = [[RakImageDropArea alloc] initWithContentString:[NSString localizedStringWithFormat:NSLocalizedString(@"IMPORT-META-DROP-PH-%d-%d", nil), 300, 300]];
	if(title != nil && dropSR != nil)
	{
		titleSize = title.bounds.size;
		inputSize = dropSR.bounds.size;

		[self setDefaultThumbFor:dropSR withID:THUMB_INDEX_SR2X fallBack : THUMB_INDEX_SR];

		currentHeight = MAX(titleSize.height, inputSize.height);
		selfSize.height -= currentHeight + META_LARGE_INTERLINE_BORDER;

		[title setFrameOrigin:NSMakePoint(META_BORDER_WIDTH + maxWidthTitles - titleSize.width, selfSize.height + (currentHeight - titleSize.height) / 2)];
		[dropSR setFrameOrigin:NSMakePoint(META_BORDER_WIDTH + maxWidthTitles + (maxWidthContent - inputSize.width) / 2, selfSize.height)];

		[self addSubview:title];
		[self addSubview:dropSR];
	}

	//Image for D&D of the project
	title = [self getTextForLocalizationString:@"IMPORT-META-IMG-DD" :maxWidthTitles];
	dropDD = [[RakImageDropArea alloc] initWithContentString:[NSString localizedStringWithFormat:NSLocalizedString(@"IMPORT-META-DROP-PH-%d-%d", nil), 100, 100]];
	if(title != nil && dropDD != nil)
	{
		titleSize = title.bounds.size;
		inputSize = dropDD.bounds.size;

		[self setDefaultThumbFor:dropDD withID:THUMB_INDEX_DD2X fallBack : THUMBID_DD];

		currentHeight = MAX(titleSize.height, inputSize.height);
		selfSize.height -= currentHeight + META_LARGE_INTERLINE_BORDER;

		[title setFrameOrigin:NSMakePoint(META_BORDER_WIDTH + maxWidthTitles - titleSize.width, selfSize.height + (currentHeight - titleSize.height) / 2)];
		[dropDD setFrameOrigin:NSMakePoint(META_BORDER_WIDTH + maxWidthTitles + (maxWidthContent - inputSize.width) / 2, selfSize.height)];

		[self addSubview:title];
		[self addSubview:dropDD];
	}

	//Image for CT of the project
	title = [self getTextForLocalizationString:@"IMPORT-META-IMG-CT" :maxWidthTitles];
	dropCT = [[RakImageDropArea alloc] initWithContentString:[NSString localizedStringWithFormat:NSLocalizedString(@"IMPORT-META-DROP-PH-%d-%d", nil), 1920, 1080]];
	if(title != nil && dropCT != nil)
	{
		titleSize = title.bounds.size;
		inputSize = dropCT.bounds.size;

		[self setDefaultThumbFor:dropCT withID:THUMB_INDEX_HEAD2X fallBack : THUMBID_HEAD];

		currentHeight = MAX(titleSize.height, inputSize.height);
		selfSize.height -= currentHeight + META_LARGE_INTERLINE_BORDER;

		[title setFrameOrigin:NSMakePoint(META_BORDER_WIDTH + maxWidthTitles - titleSize.width, selfSize.height + (currentHeight - titleSize.height) / 2)];
		[dropCT setFrameOrigin:NSMakePoint(META_BORDER_WIDTH + maxWidthTitles + (maxWidthContent - inputSize.width) / 2, selfSize.height)];

		[self addSubview:title];
		[self addSubview:dropCT];
	}

	[self bindFieldsMetadata];

	selfSize.height -= META_TOP_FORM_BORDER;

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
		button.action = @selector(validateMetadata);

		titleSize = button.bounds.size;

		[button setFrameOrigin:NSMakePoint(3 * selfSize.width / 4 - titleSize.width / 2, selfSize.height - titleSize.height)];

		[self addSubview:button];
	}

	selfSize.height -= titleSize.height + META_TOP_BORDER;

	if(selfSize.height < 0)
	{
		NSSize newSize = _bounds.size;

		newSize.height -= selfSize.height;

#ifdef EXTENSIVE_LOGGING
		NSLog(@"Correcting height to %lf", newSize.height);
#endif
		[self setFrameSize:newSize];
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

			if(i == 0)
			{
				name = inputField;
				name.wantCompletion = YES;
				name.wantCustomBorder = [name.stringValue isEqualToString:@""];
				name.customBorderColor = [Prefs getSystemColor:COLOR_BUTTON_STATUS_WARN];
				
				__block id cpSelf = self;
				__weak RakText * cpName = name;
				
				name.callbackOnChange = ^(NSNotification * notification, BOOL didComplete) {
					cpName.wantCustomBorder = [cpName.stringValue isEqualToString:@""];
					[cpSelf handleCompletion];
				};
			}
			else if(i == 1)
			{
				author = inputField;
				
				author.wantCustomBorder = [author.stringValue isEqualToString:@""];
				author.customBorderColor = [Prefs getSystemColor:COLOR_BUTTON_STATUS_WARN];
				
				__weak RakText * copyAuthor = author;
				
				author.callbackOnChange = ^(NSNotification * notification, BOOL didComplete){
					copyAuthor.wantCustomBorder = [copyAuthor.stringValue isEqualToString:@""];
				};
			}
			//Synopsis, this one is slightly different
			else if(i == 2)
			{
				description = inputField;

				inputField.enableWraps = YES;
				inputField.fixedWidth = inputSize.width;
				inputField.enableMultiLine = YES;

				[inputField setFrameSize: [inputField intrinsicContentSize]];

				inputSize.height *= 5;

				RakListScrollView * view = [[RakListScrollView alloc] initWithFrame:NSMakeRect(0, 0, inputSize.width + [RakScroller width], inputSize.height)];

				view.drawsBackground = YES;
				view.backgroundColor = inputField.backgroundColor;
				view.documentView = inputField;
				view.redirectClicToContentView = YES;

				((RakScroller *) view.verticalScroller).hideScroller = YES;
				((RakScroller *) view.verticalScroller).backgroundColorToReplicate = [self backgroundColor];

				inputField = (id) view;
			}

			CGFloat currentHeight = MAX(titleSize.height, inputSize.height);
			currentY -= currentHeight + META_LARGE_INTERLINE_BORDER;

			if(i == 2)
				[title setFrameOrigin:NSMakePoint(META_BORDER_WIDTH + maxWidthTitles - titleSize.width, currentY + currentHeight - titleSize.height)];
			else
				[title setFrameOrigin:NSMakePoint(META_BORDER_WIDTH + maxWidthTitles - titleSize.width, currentY + currentHeight / 2 - titleSize.height / 2)];

			[inputField setFrameOrigin:NSMakePoint(maxWidthTitles + 2 * META_BORDER_WIDTH, currentY + currentHeight / 2 - inputSize.height / 2)];

			[self addSubview:title];
			[self addSubview:inputField];
		}
	}

	name.maxLength = LENGTH_PROJECT_NAME - 1;
	author.maxLength = LENGTH_AUTHORS - 1;
	description.maxLength = LENGTH_DESCRIPTION - 1;

	return currentY;
}

- (void) bindFieldsMetadata
{
	name.nextKeyView = author;
	author.nextKeyView = description;
	description.nextKeyView = rightToLeft;
	rightToLeft.nextKeyView = status;
	status.nextKeyView = tagList;
	tagList.nextKeyView = catList;
	catList.nextKeyView = name;
}

- (NSString *) stringTitleMetadata
{
	return NSLocalizedString(@"IMPORT-META-HEAD", nil);
}

- (void) keyDown:(NSEvent *)theEvent
{
	if(([theEvent modifierFlags] & NSCommandKeyMask) != 0 && [[theEvent charactersIgnoringModifiers] characterAtIndex:0] == NSCarriageReturnCharacter)
	{
		if(issue == IMPORT_PROBLEM_METADATA)
			return [self validateMetadata];

		else if(issue == IMPORT_PROBLEM_METADATA_DETAILS)
			return [self validateDetail];
	}

	return [super keyDown:theEvent];
}

#pragma mark Generic initialization

- (BOOL) launchPopover : (RakView *) anchor : (RakImportStatusListRowView*) receiver
{
	[self internalInit:anchor :NSZeroRect :NO];
	return YES;
}

- (void) setupView
{
	if(issue == IMPORT_PROBLEM_METADATA)
		[self setupUIMetadata];

	else if(issue == IMPORT_PROBLEM_METADATA_DETAILS)
		[self setupUIDetails];

	else if(issue == IMPORT_PROBLEM_DUPLICATE)
		[self setupUIDuplicate];
	else
		NSLog(@"Unimplemented issue management");
}

#pragma mark - Color management & UI generation

- (RakColor *) mainTitleColor
{
	return [Prefs getSystemColor:COLOR_INACTIVE];
}

- (RakColor *) titleColor
{
	return [Prefs getSystemColor:COLOR_ACTIVE];
}

- (RakColor *) textColor
{
	return [Prefs getSystemColor:COLOR_CLICKABLE_TEXT];
}

- (RakText *) getTextForLocalizationString : (NSString *) string : (CGFloat) maxWidth
{
	RakText * text = [[RakText alloc] initWithText:NSLocalizedString(string, nil) :[self titleColor]];
	if(text != nil)
	{
		if(maxWidth)
		{
			text.enableWraps = YES;
			text.fixedWidth = maxWidth;
		}
		else
			[text sizeToFit];

		text.alignment = NSTextAlignmentRight;
	}

	return text;
}

- (RakText *) getInputFieldWithPlaceholder : (NSString *) string : (CGFloat) maxWidth
{
	RakText * text = [[RakText alloc] initWithText:nil :[self textColor]];
	if(text != nil)
	{
		((RakTextCell*) text.cell).customizedInjectionPoint = YES;
		((RakTextCell*) text.cell).centered = YES;

		text.drawsBackground = YES;
		text.backgroundColor = [Prefs getSystemColor:COLOR_TEXTFIELD_BACKGROUND];

		[text setPlaceholderString:NSLocalizedString(string, nil)];
		text.editable = YES;

		if (maxWidth != 0)
			[text setFrameSize:NSMakeSize(maxWidth, text.bounds.size.height)];
		else
			[text sizeToFit];
	}

	return text;
}

- (void) setDefaultThumbFor : (RakImageDropArea *) area withID : (uint) ID fallBack : (uint) fallbackID
{
	if(_itemOfQueryForMetadata != nil)
	{
		NSData * thumbData = [_controller queryThumbOf:_itemOfQueryForMetadata withIndex:ID];
		if(thumbData != nil)
		{
			RakImage * image = [[RakImage alloc] initWithData:thumbData];
			if(image != nil)
			{
				area.image = image;
				area.defaultImage = YES;
				return;
			}
		}

		if(fallbackID < NB_IMAGES)
			[self setDefaultThumbFor:area withID:fallbackID fallBack:NB_IMAGES];
	}
	else if(_project.isInitialized)
	{
		RakImage * image = nil;
		
		switch (ID)
		{
			case THUMB_INDEX_HEAD:
			case THUMB_INDEX_HEAD2X:
			{
				image = loadCTHeader(_project);
				break;
			}
				
			case THUMB_INDEX_SR:
			case THUMB_INDEX_SR2X:
			{
				image = loadImageGridWithoutDefault(_project);
				break;
			}
				
			case THUMB_INDEX_DD:
			case THUMB_INDEX_DD2X:
			{
				image = loadDDThumbnailWithoutDefault(_project);
				break;
			}
		}
		
		if(image != nil)
		{
			area.image = image;
			area.defaultImage = YES;
		}
	}
}

- (void) feedAnimationController : (RakCTAnimationController *) animationController
{
}

- (void) drawRect:(NSRect)dirtyRect
{
	[super drawRect:dirtyRect];

	if(bordersY[0] == 0)
		return;

	[[[RakColor whiteColor] colorWithAlphaComponent:0.3] setFill];

	dirtyRect.origin.x = dirtyRect.size.width / 6;
	dirtyRect.size.width = 4 * dirtyRect.origin.x;
	dirtyRect.size.height = 1;

	for(byte i = 0; i < 4; ++i)
	{
		if(bordersY[i] == 0)
			continue;

		dirtyRect.origin.y = bordersY[i];
		NSRectFill(dirtyRect);
	}
}

#pragma mark - Data manipulation helper

- (NSArray *) getTagArray
{
	NSMutableArray * array = [NSMutableArray arrayWithObject:@"––"];

	if(tags != NULL || getCopyOfTags(&tags, &nbTags))
	{
		for(uint i = 0; i < nbTags; i++)
			[array addObject:getStringForWchar(tags[i].name)];
	}

	return [NSArray arrayWithArray:array];
}

- (NSArray *) getCatArray
{
	NSMutableArray * array = [NSMutableArray arrayWithObject:@"––"];

	if(cats != NULL || getCopyOfCats(&cats, &nbCats))
	{
		for(uint i = 0; i < nbCats; i++)
			[array addObject:getStringForWchar(cats[i].name)];
	}

	return [NSArray arrayWithArray:array];
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

#pragma mark - Metadata completion helper

- (void) handleCompletion
{
	const char * projectName = [[name.stringValue stringByTrimmingCharactersInSet:[NSCharacterSet characterSetWithCharactersInString:@" "]] UTF8String];
	
	if(!haveOneOrLessMatchForNameStartingWith(projectName))
		return;
	
	uint cacheID = getProjectByName(projectName);
	if(cacheID == INVALID_VALUE)
		return [self restoreOldContext];
	else if(cacheID == guessedCacheID)
		return;
	
	PROJECT_DATA project = getProjectByID(cacheID);
	if(!project.isInitialized)
		return [self restoreOldContext];
	
	if(!originalIsArchived)
	{
		NSArray * tmp;
		archivedProject = [self exfilterProject:&tmp];
		archivedThumbs = tmp;
		originalIsArchived = true;
	}
	else
		releaseCTData(_project);
	
	_project = project;
	
	[self configureProject:project withImages:nil];
	[self setDefaultThumbFor:dropCT withID:THUMB_INDEX_HEAD2X fallBack : THUMBID_HEAD];
	[self setDefaultThumbFor:dropSR withID:THUMB_INDEX_SR2X fallBack : THUMBID_SRGRID];
	[self setDefaultThumbFor:dropDD withID:THUMB_INDEX_DD2X fallBack : THUMBID_DD];
}

- (void) restoreOldContext
{
	if(!originalIsArchived)
		return;

	releaseCTData(_project);
	guessedCacheID = INVALID_VALUE;
	originalIsArchived = false;
	
	_project = archivedProject;
	archivedProject = getEmptyProject();
	
	[self configureProject:_project withImages:archivedThumbs];
}

- (PROJECT_DATA) exfilterProject : (NSArray **) images
{
	PROJECT_DATA data = _project;
	
	//We linearize the data from all the fields
	wcsncpy(data.projectName, (charType*) [name.stringValue cStringUsingEncoding:NSUTF32StringEncoding], LENGTH_PROJECT_NAME);
	wcsncpy(data.authorName, (charType*) [author.stringValue cStringUsingEncoding:NSUTF32StringEncoding], LENGTH_AUTHORS);
	wcsncpy(data.description, (charType*) [description.stringValue cStringUsingEncoding:NSUTF32StringEncoding], LENGTH_DESCRIPTION);
	data.rightToLeft = rightToLeft.selectedSegment == 1;
	
	data.status = status.indexOfSelectedItem;
	data.mainTag = (tagList.indexOfSelectedItem == 0 || tagList.indexOfSelectedItem > nbTags) ? CAT_NO_VALUE : tags[tagList.indexOfSelectedItem - 1].ID;
	data.category = (catList.indexOfSelectedItem == 0 || catList.indexOfSelectedItem > nbCats) ? CAT_NO_VALUE : cats[catList.indexOfSelectedItem - 1].ID;
	
	data.tags = malloc(sizeof(TAG));
	if(data.tags != NULL)
	{
		data.nbTags = 1;
		data.tags[0].ID = data.mainTag;
	}
	else
		data.nbTags = 0;
	
	NSMutableArray * overridenImages = [NSMutableArray array];
	
	if(!dropCT.defaultImage && dropCT.image != nil)
		[overridenImages addObject:@{@"code" : @(THUMB_INDEX_HEAD2X), @"data" : dropCT.image}];
	
	if(!dropDD.defaultImage && dropDD.image != nil)
		[overridenImages addObject:@{@"code" : @(THUMB_INDEX_DD2X), @"data" : dropDD.image}];
	
	if(!dropSR.defaultImage && dropSR.image != nil)
		[overridenImages addObject:@{@"code" : @(THUMB_INDEX_SR2X), @"data" : dropSR.image}];
	
	*images = [NSArray arrayWithArray:overridenImages];
	
	return data;
}

- (void) configureProject : (PROJECT_DATA) project withImages : (NSArray *) array
{
	author.stringValue = getStringForWchar(project.authorName);
	description.stringValue = getStringForWchar(project.description);
	
	[rightToLeft setSelectedSegment:project.rightToLeft];
	[status selectItemAtIndex:project.status];
	
	if(project.mainTag != CAT_NO_VALUE)
	{
		for(uint i = 0; i < nbTags; i++)
		{
			if(tags[i].ID == project.mainTag)
			{
				[tagList selectItemAtIndex:i];
				break;
			}
		}
	}
	
	if(project.category)
	{
		for(uint i = 0; i < nbCats; i++)
		{
			if(cats[i].ID == project.category)
			{
				[catList selectItemAtIndex:i];
				break;
			}
		}
	}
	
	if(array == nil)
		return;
	
	for(NSDictionary * dict in array)
	{
		switch ([[dict objectForKey:@"code"] unsignedIntegerValue])
		{
			case THUMB_INDEX_HEAD2X:
				dropCT.image = [dict objectForKey:@"data"];
				break;
				
			case THUMB_INDEX_SR2X:
				dropSR.image = [dict objectForKey:@"data"];
				break;
				
			case THUMB_INDEX_DD2X:
				dropDD.image = [dict objectForKey:@"data"];
				break;
		}
	}
}

#pragma mark - Button responder

- (void) changedIsTome
{
	BOOL isTomeSelected = isTome.selectedSegment;

	//Top header
	detailHeader.stringValue = NSLocalizedString(isTomeSelected ? @"IMPORT-DET-HEAD-VOL" : @"IMPORT-DET-HEAD-CHAP", nil);

	NSRect frame = detailHeader.frame;

	[detailHeader sizeToFit];
	frame.size = detailHeader.frame.size;

	[detailHeader setFrameOrigin:NSMakePoint(_bounds.size.width / 2 - frame.size.width / 2, frame.origin.y)];

	//Input fields
	contentIDTitle.stringValue = [NSString stringWithFormat:@"%@ #", NSLocalizedString(isTomeSelected ? @"VOLUME" : @"CHAPTER", nil)];

	//We need to check that what was inputed in the field is valid
	if(!isTomeSelected)
	{
		NSString * string = contentID.stringValue;

		while(string.length > 0 && ![contentID.formatter isPartialStringValid:string newEditingString:nil errorDescription:nil])
		{
			string = [string substringToIndex:string.length - 1];
		}

		contentID.stringValue = string;
	}

	[self positionForIsTomeState:isTomeSelected withAnimation:YES];
}

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

- (void) validateMetadata
{
	NSArray * overridenImages;

	if([_controller reflectMetadataUpdate:[self exfilterProject:&overridenImages] withImages:overridenImages forItem:_itemOfQueryForMetadata])
		[self close];
}

- (void) validateDetail
{
	BOOL wantTome = isTome.selectedSegment != 0, validInput = NO;

	NSNumber * number = @([getNumberForString(contentID.stringValue) intValue] * 10);
	NSString * string = contentName.stringValue;

	uint checkedContentID = INVALID_VALUE;

	//Valid name and reading ID
	if(wantTome)
	{
		if(number != nil && [number longLongValue] <= INT_MAX && [number longLongValue] >= INT_MIN && [number intValue] != INVALID_SIGNED_VALUE)
		{
			checkedContentID = (uint) [number intValue];
			validInput = YES;
		}
		
		if(string != nil && [string length] > 0)
			validInput = YES;
		else
			string = nil;
	}

	//Valid number ID
	else if(number != nil && [number unsignedIntValue] != INVALID_VALUE)
	{
		checkedContentID = [number unsignedIntValue];
		validInput = YES;
	}

	//No valid data :(
	if(!validInput)
		return;

	PROJECT_DATA_EXTRA project = _item.projectData;
	_item.contentID = checkedContentID;

	if(wantTome)
	{
		META_TOME tomeData;
		
		//Somehow, the volume wasn't registered ¯\_(ツ)_/¯
		if(project.data.nbVolumesLocal == 0)
		{
			tomeData.ID = getVolumeIDForImport(project.data.project);
			if(tomeData.ID != INVALID_VALUE)
			{
				tomeData.readingID = (int) checkedContentID;
				wstrncpy(tomeData.readingName, MAX_TOME_NAME_LENGTH + 1, getStringFromUTF8((const byte *) [string UTF8String]));
				
				tomeData.details = malloc(sizeof(CONTENT_TOME));
				if(tomeData.details != NULL)
				{
					tomeData.details[0].ID = CHAPTER_FOR_IMPORTED_VOLUMES;
					tomeData.details[0].isPrivate = true;
					tomeData.lengthDetails = 1;
				}
				
				project.data.tomeLocal = malloc(sizeof(META_TOME));
				if(project.data.tomeLocal != NULL)
				{
					project.data.nbVolumesLocal = 1;
					project.data.tomeLocal[0] = tomeData;
					
					//If it migrated, we remove the other entry
					if(project.data.nbChapterLocal > 0)
					{
						project.data.nbChapterLocal = 0;
						free(project.data.chaptersLocal);
						project.data.chaptersLocal = NULL;
					}
				}
				else
					free(tomeData.details);
			}
			else
				logR("Couldn't grab a volume ID ಠ_ಠ\nYou should restart Rakshata and/or import smaller chuncks (and send us an email)");
		}
		else
		{
			project.data.tomeLocal[0].readingID = (int) checkedContentID;
			wstrncpy(project.data.tomeLocal[0].readingName, MAX_TOME_NAME_LENGTH + 1, getStringFromUTF8((const byte *) [string UTF8String]));			
		}
	}
	else if(_item.isTome != wantTome)
	{
		releaseParsedData(project.data);
		nullifyParsedPointers(&project.data);
		
		project.data.chaptersLocal = malloc(sizeof(uint));
		if(project.data.chaptersLocal != NULL)
		{
			project.data.nbChapterLocal = 1;
			project.data.chaptersLocal[0] = _item.contentID;
			generateCTUsable(&project.data);
		}
	}
	else
	{
		project.data.chaptersLocal[0] = _item.contentID;
	}

	_item.projectData = project;
	_item.isTome = wantTome;
	[self close];

	[_item refreshState];
	[_controller postProcessUpdate];
}

- (void) close
{
	[self closePopover];
}

@end

@implementation RakCTFormatter

- (BOOL)isPartialStringValid:(NSString*)partialString newEditingString:(NSString**)newString errorDescription:(NSString**)error
{
	if([partialString length] == 0)
		return YES;

	NSMutableCharacterSet * chSet = [NSMutableCharacterSet decimalDigitCharacterSet];
	[chSet addCharactersInString:@".,"];

	//Check for non-numerical chars
	if([partialString rangeOfCharacterFromSet:[chSet invertedSet]].location != NSNotFound)
		return NO;

	//Don't allow both , and .
	if([partialString rangeOfString:@","].location != NSNotFound && [partialString rangeOfString:@"."].location != NSNotFound)
		return NO;

	//Try to convert in a way that convert , but fails with .
	NSNumber * content = getNumberForString(partialString);

	if(content == nil)
	{
		//Try to convert in a way that convert . but fails with ,
		content = [[NSDecimalNumber alloc] initWithString:partialString];

		if(content == nil)
			return NO;
	}

	double numberWithNoDecimal = [content doubleValue] * 10;

	//We only want one number after ./,
	if((numberWithNoDecimal * 10) - (((int64_t) numberWithNoDecimal) * 10))
		return NO;

	return YES;
}

@end
