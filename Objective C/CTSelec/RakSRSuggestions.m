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

@implementation RakSRSuggestions

- (instancetype) init : (NSRect) frame
{
	self = [self init];
	
	if(self != nil)
	{
		[self initModel];
		
		[RakDBUpdate registerForUpdate:self :@selector(DBUpdated:)];
		
		[self applyContext:frame : selectedRowIndex : -1];
		
		scrollView.verticalScroller.alphaValue = 0;

		[self setEmptyState : _nbData == 0];
	}
	
	return self;
}

- (void) initModel
{
	ID = malloc(10 * sizeof(uint));
	if(ID == NULL)
		return;
	
	cache = getCopyCache(SORT_NAME, &nbElem);
	if(cache == NULL && nbElem != 0)	// o_o
	{
		free(ID);
		ID = NULL;
		return;
	}
	
	_nbData = MIN(nbElem, 10);

	NSMutableArray * array = [NSMutableArray arrayWithCapacity:10], * usedID = [NSMutableArray arrayWithCapacity:10];
	
	for (uint i = 0, value; i < _nbData; i++)
	{
		//Prevent reusing IDs
		value = getRandom() % nbElem;
		while([usedID indexOfObject:@(value)] != NSNotFound)
		{
			++value;
			value %= nbElem;
		}
		
		[usedID addObject:@(value)];
		[array addObject:getStringForWchar(cache[(ID[i] = value)].projectName)];
	}
	
	names = [NSArray arrayWithArray:array];
}

- (void) DBUpdated : (NSNotification*) notification
{
	if(getDBCount() == nbElem && ![RakDBUpdate isPluralUpdate:notification.userInfo])
		return;

	freeProjectData(cache);
	free(ID);
	
	[self initModel];
	[_tableView reloadData];
	[self setEmptyState : _nbData == 0];
}

- (void) dealloc
{
	[RakDBUpdate unRegister:self];
}

- (NSRect) getFrameFromParent: (NSRect) bounds
{
	bounds.origin.x += _scrollerWidth / 5;
	bounds.size.width += _scrollerWidth / 2;
	bounds.size.height -= 3;
	bounds.origin.y -= 3;
	
	return bounds;
}

#pragma mark - Element generation

- (NSInteger) numberOfRowsInTableView : (RakTableView *) tableView
{
	return _nbData;
}

- (CGFloat)tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row
{
	return [self tableView:tableView viewForTableColumn:nil row:row].bounds.size.height;
}

- (NSView*) tableView : (RakTableView *) tableView viewForTableColumn : (NSTableColumn*) tableColumn row : (NSInteger) row
{
	RakCTFocusSRItem * element = [tableView makeViewWithIdentifier : _identifier owner:self];
	byte reason = row & 1 ? SUGGESTION_REASON_TAG : SUGGESTION_REASON_AUTHOR;
	
	if(element == nil)
	{
		element = [[RakCTFocusSRItem alloc] initWithProject : cache[ID[row]] reason : reason];
		element.identifier = _identifier;
		element.table = self;
		
		if(row == _nbData - 1)
			element.last = YES;
	}
	else
	{
		element.reason = reason;
		[element updateProject:cache[ID[row]]];
	}
	
	return element;
}

#pragma mark - Configuration

- (BOOL) tableView:(NSTableView *)aTableView writeRowsWithIndexes:(NSIndexSet *)rowIndexes toPasteboard:(NSPasteboard *)pboard
{
	return NO;
}

#pragma mark - Handle empty state

- (void) setEmptyState : (BOOL) isEmpty
{
	if(isEmpty)
	{
		if(placeholder == nil)
		{
			placeholder = [[RakText alloc] initWithText:NSLocalizedString(@"PROJ-SUGG-EMPTY", nil) :[Prefs getSystemColor:COLOR_INACTIVE]];
			if(placeholder != nil)
			{
				placeholder.font = [NSFont fontWithName:[placeholder.font fontName] size:15];
				placeholder.alignment = NSCenterTextAlignment;
				placeholder.enableWraps = YES;
				placeholder.fixedWidth = 200;
				placeholder.enableMultiLine = YES;
				[placeholder updateMultilineHeight];
				
				NSSize size = scrollView.bounds.size;
				size.width -= _scrollerWidth;
				
				[placeholder setFrameOrigin:NSCenterSize(size, placeholder.bounds.size)];
				
				[scrollView addSubview:placeholder];
			}
		}
		else
		{
			[placeholder setFrameOrigin:NSCenterSize(scrollView.bounds.size, placeholder.bounds.size)];
			placeholder.hidden = NO;
		}
	}
	else
	{
		if(placeholder != nil)
			placeholder.hidden = YES;
	}
}

- (void) additionalResizing : (NSSize) newSize : (BOOL) animated
{
	if(!placeholder.isHidden)
	{
		newSize.width -= _scrollerWidth;

		if(animated)
			[placeholder.animator setFrameOrigin:NSCenterSize(newSize, placeholder.bounds.size)];
		else
			[placeholder setFrameOrigin:NSCenterSize(newSize, placeholder.bounds.size)];
	}
	
	[super additionalResizing:newSize :animated];
}

@end
