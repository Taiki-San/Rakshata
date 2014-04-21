/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite  **
 **                                                                                         **
 *********************************************************************************************/

@implementation RakSerieListItem

- (id) init : (void*) data : (BOOL) isRootItem : (int) initStage : (uint) nbChildren
{
	self = [super init];
	
	if(self != nil)
	{
		_isRootItem = isRootItem;
		
		if(_isRootItem)
		{
			children = [[NSMutableArray alloc] init];
			dataChild	= NULL;
			_expanded = YES;
			_isRecentList = _isDLList = _isMainList = NO;
			_nbChildren = nbChildren;
			
			switch (initStage)
			{
				case INIT_FIRST_STAGE:
				{
					_isRecentList = YES;
					dataRoot = @"Consulté récemment";
					break;
				}
					
				case INIT_SECOND_STAGE:
				{
					_isDLList = YES;
					dataRoot = @"Téléchargé récemment";
					break;
				}
					
				case INIT_THIRD_STAGE:
				{
					_isMainList = YES;
					dataRoot = @"Liste complète";
					break;
				}
			}
		}
		else
		{
			dataRoot	= nil;
			dataChild	= data;
			
			if(dataChild == nil && initStage == INIT_FINAL_STAGE)
				_isMainList = YES;
			else
				_isMainList = NO;
		}
	}
	
	return self;
}

- (BOOL) isRootItem
{
	return _isRootItem;
}

- (BOOL) isRecentList
{
	return _isRecentList;
}

- (BOOL) isDLList
{
	return _isDLList;
}

- (BOOL) isMainList
{
	return _isMainList;
}

- (void) setMainListHeight : (CGFloat) height
{
	_mainListHeight = height;
}

- (void) resetMainListHeight
{
	if([self isRootItem] && [self isMainList])
	{
		if([children count] == 1)
		{
			RakSerieListItem *mainList = [children objectAtIndex:0];
			[mainList setMainListHeight:0];
		}
	}
	else
		NSLog(@"Invalid request, I'm not the item you're looking for...");
}

- (CGFloat) getHeight
{
	if([self isRootItem])
		return 25;
	else if([self isMainList] && _mainListHeight)
		return _mainListHeight;
	
	return 0;
}

- (void) setExpaded : (BOOL) expanded
{
	_expanded = expanded;
}

- (BOOL) isExpanded
{
	return _expanded;
}

- (uint) getNbChildren
{
	if([self isRootItem])
		return _nbChildren;
	return 0;
}

- (void) setChild : (id) child atIndex : (NSInteger) index
{
	if(children != nil)
	{
		[children insertObject:child atIndex:index];
	}
}

- (id) getChildAtIndex : (NSInteger) index
{
	if(children != nil && [children count] > index)
		return [children objectAtIndex:index];
	
	return nil;
}

- (MANGAS_DATA*) getRawDataChild
{
	if (_isRootItem || _isMainList)
		return NULL;
	else
		return dataChild;
}

- (NSString*) getData
{
	if(_isRootItem && dataRoot != NULL)
		return dataRoot;
	else if(!_isRootItem && dataChild != NULL)
		return [NSString stringWithCString:dataChild->mangaName encoding:NSUTF8StringEncoding];
	else
		return @"Internal error :(";
}

@end

@implementation RakSerieList

- (id) init : (NSRect) frame : (BOOL) isRecentDownload
{
	self = [super init];
	
	if(self != nil)
	{
		initializationStage = INIT_FIRST_STAGE;
		[self loadContent];
		
		if(_data != nil)
		{
			content = [[RakTreeView alloc] initWithFrame:frame];
			[content setFrame:frame];
			column = [[RakTableColumn alloc] initWithIdentifier:@"The Solar Empire shall fall!"];
			[column setWidth:content.frame.size.width];
			
			//Customisation
			[content setIndentationPerLevel:[content indentationPerLevel] / 2];
			[content setBackgroundColor:[NSColor clearColor]];
			[content setFocusRingType:NSFocusRingTypeNone];
			[content setAutoresizesOutlineColumn:NO];
			
			//End of setup
			[content setDelegate:self];
			[content setDataSource:self];
			[content addTableColumn:column];
			[content setOutlineTableColumn:column];
			[column release];
			
			//We need some tweaks to be sure everything is properly deployed
			if(rootItems[2] != nil)
				[rootItems[2] resetMainListHeight];

			[content expandItem:nil expandChildren:YES];
	
			initializationStage = INIT_OVER;
		}
		else
			[self release];
	}
	
	return self;
}

- (RakTreeView *) getContent
{
	return content;
}

- (id) retain
{
	[content retain];
	return [super retain];
}

- (oneway void) release
{
	[content release];
	[super release];
}

- (void) dealloc
{
	freeMangaData(_cache);
	[_data release];
	[content removeFromSuperview];
	
	for (char i = 0; i < 3; i++)
	{
		if(rootItems[i] != nil)
			[rootItems[i] release];
	}
	
	[super dealloc];
}

- (void) setFrame: (NSRect) frame
{
	[column setFixedWidth:frame.size.width];
	
	NSRect mainListFrame = [_mainList frame];

	mainListFrame.origin.x = mainListFrame.origin.y = 0;
	mainListFrame.size.width = frame.size.width;
	mainListFrame.size.height += frame.size.height - [content frame].size.height;
	
	[content setFrame:frame];

	[_mainList setFrame:[self getMainListFrame:content]];
	if(rootItems[2] != nil)
		[rootItems[2] resetMainListHeight];
	
	[content reloadData];
}

- (void) setFrameOrigin : (NSPoint) newOrigin
{
	[content setFrameOrigin:newOrigin];
}

#pragma mark - Color

- (NSColor *) getFontColor
{
	return [Prefs getSystemColor:GET_COLOR_INACTIVE];
}

#pragma mark - Loading routines

- (void) loadContent
{
	_cache = getCopyCache(RDB_CTXSERIES | SORT_TEAM, &_sizeCache);
	
	if(_cache != NULL)
	{
		_data = [[NSPointerArray alloc] initWithOptions:NSPointerFunctionsOpaqueMemory];
		
		//Recent read
		changeTo(_cache[_sizeCache - 1].mangaName, '_', ' ');
		[_data addPointer:&_cache[_sizeCache - 1]];
		
		changeTo(_cache[_sizeCache - 2].mangaName, '_', ' ');
		[_data addPointer:&_cache[_sizeCache - 2]];
		
		changeTo(_cache[_sizeCache - 3].mangaName, '_', ' ');
		[_data addPointer:&_cache[_sizeCache - 3]];
		
		_nbElemReadDisplayed = 3;
		
		//Recent DL
		changeTo(_cache[_sizeCache - 4].mangaName, '_', ' ');
		[_data addPointer:&_cache[_sizeCache - 4]];
		
		changeTo(_cache[_sizeCache - 5].mangaName, '_', ' ');
		[_data addPointer:&_cache[_sizeCache - 5]];
		
		changeTo(_cache[_sizeCache - 6].mangaName, '_', ' ');
		[_data addPointer:&_cache[_sizeCache - 6]];
		
		_nbElemDLDisplayed = 3;
	}
}

- (void) goToNextInitStage
{
	initializationStage++;
}

- (uint) getChildrenByInitialisationStage
{
	if(initializationStage == INIT_FIRST_STAGE)
	{
		if(_nbElemReadDisplayed)
			return _nbElemReadDisplayed;
		initializationStage++;
	}
	
	if(initializationStage == INIT_SECOND_STAGE)
	{
		if(_nbElemDLDisplayed)
			return _nbElemDLDisplayed;
		initializationStage++;
	}
	
	if(initializationStage == INIT_THIRD_STAGE)
	{
		if(_sizeCache)
			return 1;
		initializationStage++;
	}
	
	return 0;
}

#pragma mark - Data source to the view

- (NSInteger)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item
{
    if(item == nil)
	{
		return (_nbElemReadDisplayed != 0) + (_nbElemDLDisplayed != 0) + (_sizeCache != 0);
	}
	else if ([item isRootItem])
		return [item getNbChildren];
	
	return 0;
}


- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item
{
	return item == nil ? YES : [item isRootItem];
}


- (id)outlineView:(NSOutlineView *)outlineView child:(NSInteger)index ofItem:(id)item
{
	id output;
	
	if(item == nil)
	{
		if(index >= 3)
			return nil;
			
		if(rootItems[index] == nil)
		{
			rootItems[index] = [[RakSerieListItem alloc] init : NULL : YES : initializationStage : [self getChildrenByInitialisationStage]];
			
			if(initializationStage != INIT_OVER)
				initializationStage++;
		}
		
		output = rootItems[index];
	}
	else if(![item isMainList])
	{
		output = [item getChildAtIndex:index];
		if(output == nil)
		{
			output = [[RakSerieListItem alloc] init : [_data pointerAtIndex: (index + ([item isDLList] ? 3 : 0))] : NO : initializationStage : 0];
			[item setChild:output atIndex:index];
		}
	}
	else
	{
		output = [item getChildAtIndex:index];
		if(output == nil)
		{
			output = [[RakSerieListItem alloc] init : nil : NO : initializationStage : 0];
			[item setChild:output atIndex:index];
		}
	}
	
	return output;
}

- (id)outlineView:(NSOutlineView *)outlineView objectValueForTableColumn:(NSTableColumn *)tableColumn byItem:(id)item
{
	if(item == NULL)
		return @"Invalid data :(";
	else
		return [item getData];
}

#pragma mark - Delegate to the view

- (BOOL)outlineView:(NSOutlineView *)outlineView shouldSelectItem:(id)item
{
	if(item == nil || [item isRootItem] || [item isMainList])
		return NO;
	
	MANGAS_DATA *tmp = [item getRawDataChild];
	
	if(tmp == NULL)
		return NO;
	
	MANGAS_DATA dataToSend = *tmp;
	changeTo(dataToSend.mangaName, ' ', '_');
	
	[RakTabView broadcastUpdateContext: content : dataToSend : NO : VALEUR_FIN_STRUCTURE_CHAPITRE];
	
	return YES;
}

- (BOOL)outlineView:(NSOutlineView *)outlineView shouldExpandItem:(id)item
{
	[item setExpaded:YES];
	return YES;
}

- (BOOL) outlineView:(NSOutlineView *)outlineView shouldCollapseItem:(id)item
{
	[item setExpaded:NO];
	return YES;
}

- (BOOL)outlineView:(NSOutlineView *)outlineView isGroupItem:(id)item
{
	return item != nil && ![item isRootItem];
}

- (BOOL)outlineView:(NSOutlineView *)outlineView shouldEditTableColumn:(NSTableColumn *)tableColumn item:(id)item
{
	return NO;
}

- (BOOL)outlineView:(NSOutlineView *)outlineView shouldShowOutlineCellForItem:(id)item
{
	return [item isRootItem] && ![item isMainList];
}

- (CGFloat)outlineView:(NSOutlineView *)outlineView heightOfRowByItem:(id)item
{
	CGFloat output;
	
	if(item == nil)
		output = 0;

	else if((output = [item getHeight]) == 0)
	{
		if([item isMainList])
		{
			CGFloat p = [outlineView intercellSpacing].height;	//Padding
			
			output = content.frame.size.height - (_nbElemReadDisplayed != 0) * (25 + p) - _nbElemReadDisplayed * ([outlineView rowHeight] + p) - (_nbElemDLDisplayed != 0) * (25 + p) - _nbElemDLDisplayed * ([outlineView rowHeight] + p) - (25 + p) - p;
			
			if(output < 25)
				output = 25;

			[item setMainListHeight:output];
		}
		
		else
			output = [outlineView rowHeight];
	}
	
	return output;
}

///			Manipulation we view added/removed

- (void)outlineView:(NSOutlineView *)outlineView didAddRowView:(NSTableRowView *)rowView forRow:(NSInteger)row
{

}

- (void)outlineView:(NSOutlineView *)outlineView didRemoveRowView:(NSTableRowView *)rowView forRow:(NSInteger)row
{
	
}

///		Craft views

- (NSTableRowView *) outlineView:(NSOutlineView *)outlineView rowViewForItem:(id)item
{
	if (![self outlineView:outlineView isGroupItem:item])
		return nil;
	
	NSTableRowView *rowView = [outlineView makeViewWithIdentifier:@"HeaderRowView" owner:nil];
	if (!rowView)
	{
		rowView = [[RakTableRowView alloc] init];
		rowView.identifier = @"HeaderRowView";
	}

	return rowView;
}

- (NSView *)outlineView:(NSOutlineView *)outlineView viewForTableColumn:(NSTableColumn *)tableColumn item:(id)item
{
	NSView * rowView;
	
	if([item isMainList])
	{
		if([item isRootItem])
		{
			rowView = [[RakSRSubMenu alloc] initWithText:outlineView.bounds :@"" : nil];
		}
		else
		{
			if(_mainList == nil)
				_mainList = [[RakSerieMainList alloc] init: [self getMainListFrame:outlineView]];
			
			rowView = [_mainList getContent];
		}
	}
	else
	{
		rowView = [outlineView makeViewWithIdentifier:@"StandardLine" owner:nil];
		if (rowView == nil)
		{
			rowView = [[RakText alloc] init];
			rowView.identifier = @"StandardLine";
			[(RakText*) rowView setTextColor:[self getFontColor]];
			
			if([item isRootItem])
				[(RakText*) rowView setFont:[NSFont fontWithName:@"Helvetica-Bold" size:13]];
			else
				[(RakText*) rowView setFont:[NSFont fontWithName:@"Helvetica" size:13]];
		}
	}
	
	return rowView;
}

- (NSRect) getMainListFrame : (NSOutlineView*) outlineView
{
	NSRect frame = [outlineView bounds];
	
	frame.size.width -= 2 * [outlineView indentationPerLevel];
	
	return frame;
}

#pragma mark - Notifications

- (void)outlineViewItemDidExpand:(NSNotification *)notification
{
    if (notification.object == content && initializationStage == INIT_OVER)
	{
		[self updateMainListSizePadding];
	}
}

- (void)outlineViewItemDidCollapse:(NSNotification *)notification
{
    if (notification.object == content && initializationStage == INIT_OVER)
	{
		[self updateMainListSizePadding];
	}
}

- (void) updateMainListSizePadding
{
	BOOL currentPortionExpanded = NO;
	RakSerieListItem *mainList = nil, *item;
	NSInteger height = 0, positionMainList = -1, nbRow = [content numberOfRows];
	CGFloat rowHeight = [content rowHeight], padding = [content intercellSpacing].height;
	
	for(NSInteger row = 0; row < nbRow; row++)
	{
		item = [content itemAtRow:row];
		
		if(item == nil)
			continue;
		
		if([item isRootItem])
		{
			height += 25 + padding;
			currentPortionExpanded = [item isExpanded];
		}
		
		else if([item isMainList])
		{
			positionMainList = row;
			mainList = item;
		}
		
		else if(currentPortionExpanded)
		{
			height += rowHeight + padding;
		}
	}
	
	if(mainList != nil)
	{
		height = content.frame.size.height - height - padding;		//We got last row height
		[mainList setMainListHeight:height];
		[content noteHeightOfRowsWithIndexesChanged:[NSMutableIndexSet indexSetWithIndex:positionMainList]];
	}
}

@end

