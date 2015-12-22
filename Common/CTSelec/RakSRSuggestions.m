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

		[self setEmptyState : [dataArray count] == 0];
	}
	
	return self;
}

- (void) initModel
{
	dataArray = [[RakSuggestionEngine getShared] getSuggestionForProject:INVALID_VALUE withNumber:10];
}

- (void) DBUpdated : (NSNotification*) notification
{
	[self initModel];
	[_tableView reloadData];
	[self setEmptyState : [dataArray count] == 0];
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
	return (NSInteger) [dataArray count];
}

- (CGFloat)tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row
{
	return [self tableView:tableView viewForTableColumn:nil row:row].bounds.size.height;
}

- (RakView*) tableView : (RakTableView *) tableView viewForTableColumn : (NSTableColumn*) tableColumn row : (NSInteger) row
{
	NSDictionary * data = dataArray[(NSUInteger) row];
	
	RakCTFocusSRItem * element = [tableView makeViewWithIdentifier : _identifier owner:self];
	if(element == nil)
	{
		element = [[RakCTFocusSRItem alloc] initWithProject : [[RakSuggestionEngine getShared] dataForIndex:[[data objectForKey:@"ID"] unsignedIntValue]] reason : [[data objectForKey:@"reason"] unsignedIntValue] insertionPoint : [data objectForKey:@"insertionPoint"]];

		element.identifier = _identifier;
		element.table = self;
		element.controller = self;
		element.clickValidation = @selector(receiveClick:forClick:);
		
		
		if(row == _nbData - 1)
			element.last = YES;
	}
	else
	{
		element.reason = [[data objectForKey:@"reason"] unsignedIntValue];
		[element updateProject:[[RakSuggestionEngine getShared] dataForIndex:[[data objectForKey:@"ID"] unsignedIntValue]] insertionPoint:[data objectForKey:@"insertionPoint"]];
	}
	
	return element;
}

- (BOOL) receiveClick : (RakThumbProjectView *) project forClick : (byte) selection
{
	return ![RakSuggestionEngine suggestionWasClicked:project.elementID withInsertionPoint:project.insertionPoint];
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
