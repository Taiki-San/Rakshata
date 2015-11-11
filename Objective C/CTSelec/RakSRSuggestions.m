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
		if(![self initModel])
			return nil;
		
		[self applyContext:frame : selectedRowIndex : -1];
		
		scrollView.verticalScroller.alphaValue = 0;
	}
	
	return self;
}

- (BOOL) initModel
{
	ID = malloc(10 * sizeof(uint));
	if(ID == NULL)
		return NO;
	
	cache = getCopyCache(SORT_NAME, &nbElem);
	if(cache == NULL || nbElem == 0)
	{
		free(ID);
		return NO;
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
	
	return YES;
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

@end
