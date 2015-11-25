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

@implementation RakSRSearchList

- (instancetype) init : (NSRect) frame ofType : (byte) type withData: (charType **) dataList ofSize : (uint) nbDataList andIndexes : (uint64_t *) inputIndexes
{
	self = [self init];
	
	if(self != nil)
	{
		_type = type;
		_manualSelection = NO;
		
		if(inputIndexes == NULL || dataList == NULL)
		{
			inputIndexes = NULL;
			dataList = NULL;
			nbDataList = 0;
		}
		
		indexes = inputIndexes;
		_data = dataList;
		_nbData = nbDataList;
		
		[self applyContext:frame : selectedRowIndex : -1];
		
		selection = [NSMutableArray array];
		
		NSString * notificationName = [self getNotificationName];
		if(notificationName != nil)
			[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(triggerFired:) name:notificationName object:nil];
	}
	
	return self;
}

#pragma mark - Data manipulation

- (void) updateData : (charType **) names ofSize : (uint) nbData andIndexes : (uint64_t *) listIndexes
{
	NSMutableArray * array = [NSMutableArray arrayWithCapacity:nbData];

	for(uint pos = 0; pos < nbData; ++pos)
		[array addObject:@(listIndexes[pos])];
	
	if([selection count] > 0)
	{
		//Remove deleted items
		NSMutableArray * deletedItems = [selection copy];
		[deletedItems removeObjectsInArray:array];
		
		for(NSNumber * item in deletedItems)
			[self tableView:_tableView shouldSelectRow:[item longValue]];
	}
	
	indexes = listIndexes;
	_data = names;
	_nbData = nbData;
	
	[_tableView reloadData];
}

- (SR_DATA *) getSmartReloadData : (PROJECT_DATA*) data : (uint) nbElem : (BOOL *) installed
{
	if(!nbElem)
		return NULL;
	
	SR_DATA * output = calloc(nbElem, sizeof(SR_DATA));
	
	if(output != NULL)
	{
		for(uint i = 0; i < nbElem; i++)
		{
			output[i].data = indexes[i];
		}
	}
	
	return output;
}

#pragma mark - tableview

- (NSColor *) getTextHighlightColor
{
	return [Prefs getSystemColor:COLOR_SURVOL];
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView
{
	return _data == NULL ? 0 : _nbData;
}

- (CGFloat)tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row
{
	return 14;
}

- (NSView*) tableView : (RakTableView *) tableView viewForTableColumn : (NSTableColumn*) tableColumn row : (NSInteger) row
{
	RakText * view = (RakText *) [super tableView:tableView viewForTableColumn:tableColumn row:row];
	
	if(view != nil)
	{
		if([selection indexOfObject:@(row)] != NSNotFound)
		{
			view.textColor = (highlight != nil ? highlight : [self getTextHighlightColor:0 :row]);
			view.drawsBackground = YES;
		}
		
		view.font = [NSFont fontWithName:[Prefs getFontName:GET_FONT_STANDARD] size:11];
	}
	
	return view;
}

- (NSString*) tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
	if(rowIndex < _nbData)
	{
		return getStringForWchar(((charType **) _data)[rowIndex]);
	}
	else
		return @"Error D:";
}

//Selection

- (BOOL) tableView:(RakTableView *)tableView shouldSelectRow:(NSInteger)rowIndex
{
	[tableView commitClic];
	
	lastWasSelected = [selection indexOfObject:@(indexes[rowIndex])] != NSNotFound;

	RakText * view = [tableView viewAtColumn:0 row:rowIndex makeIfNecessary:NO];
	if(view != nil && [view class] == [RakText class])
	{
		if(lastWasSelected)
			view.textColor = (normal != nil ? normal : [self getTextColor:0 :rowIndex]);
		else
			view.textColor = (highlight != nil ? highlight : [self getTextHighlightColor:0 :rowIndex]);

		[view setNeedsDisplay];
	}

	if(lastWasSelected)
		[selection removeObject:@(indexes[rowIndex])];
	else
		[selection addObject:@(indexes[rowIndex])];
	
	selectedRowIndex = rowIndex;	//Signal the row being updated
	
	//If the UI was out of date, we don't update the backend
	BOOL isUIOutOfDate = lastWasSelected == haveRestriction(indexes[rowIndex], getRestrictionTypeForSBID(_type));

	lastWasSelected = !lastWasSelected;
	
	if(isUIOutOfDate)
		[self postProcessingSelection];
	
	if(!lastWasSelected)
		selectedRowIndex = LIST_INVALID_SELECTION;
	
	return NO;
}

- (void) postProcessingSelection
{
	if(selectedRowIndex != LIST_INVALID_SELECTION && !_manualSelection)
	{
		NSString * notificationName = [self getNotificationName];
		if(notificationName != nil)
		{
			_manualSelection = YES;
			[[NSNotificationCenter defaultCenter] postNotificationName:notificationName object:getStringForWchar(((charType **) _data)[selectedRowIndex]) userInfo:@{SR_NOTIF_CACHEID : @(indexes[selectedRowIndex]), SR_NOTIF_OPTYPE : @(lastWasSelected)}];
			_manualSelection = NO;
		}
	}
}

- (NSColor *) getBackgroundHighlightColor
{
	return [NSColor clearColor];
}

#pragma mark - Trigger

- (NSString *) getNotificationName
{
	return getNotificationNameForSBID(_type);
}

- (void) triggerFired : (NSNotification *) notification
{
	if(_manualSelection)
		return;
	
	NSNumber * ID;
	
	if(notification == nil || notification.userInfo == nil || (ID = [notification.userInfo objectForKey:SR_NOTIF_CACHEID]) == nil || ![ID isKindOfClass:[NSNumber class]])
		return;
	
	uint64_t code = [ID unsignedLongLongValue];
	for(uint i = 0; i < _nbData; i++)
	{
		if(indexes[i] == code)
		{
			//If we want to unselect, but the item wasn't selected
			if((ID = [notification.userInfo objectForKey:SR_NOTIF_UNSELECT]) != nil && [ID isKindOfClass:[NSNumber class]] && [ID boolValue])
			{
				if([selection indexOfObject:@(code)] == NSNotFound)
					return;
			}
			
			_manualSelection = YES;
			[self tableView:_tableView shouldSelectRow:i];
			_manualSelection = NO;
			return;
		}
	}
}

- (void) fullCleanup
{
	//We assume the system that posted the notification called setLockStatusNotifyRestrictionChanged(true)
	NSArray * oldSelection = [NSArray arrayWithArray:selection];
	
	for(NSNumber * number in oldSelection)
	{
		uint64_t code = [number unsignedLongLongValue];
		for(uint i = 0; i < _nbData; i++)
		{
			if(indexes[i] == code)
			{
				[self tableView:_tableView shouldSelectRow:i];
			}
		}
	}
	
	[selection removeAllObjects];
}

@end
