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
			dataChild	= NULL;
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

- (uint) getNbChildren
{
	if([self isRootItem])
		return _nbChildren;
	return 0;
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
			NSTableColumn * column = [[NSTableColumn alloc] initWithIdentifier:@"The Solar Empire shall fall!"];
			[column setWidth:content.frame.size.width];
			
			//Customisation
			[content setBackgroundColor:[NSColor clearColor]];
			[content setFocusRingType:NSFocusRingTypeNone];
			
			//End of setup
			[content setDelegate:self];
			[content setDataSource:self];
			[content addTableColumn:column];
			[content setOutlineTableColumn:column];
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
	
	[super dealloc];
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
			return 0;
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
	if(item == nil)
	{
		id output = [[RakSerieListItem alloc] init : NULL : YES : initializationStage : [self getChildrenByInitialisationStage]];
		
		if(initializationStage != INIT_OVER)
			initializationStage++;
		
		return output;
	}
	else if(![item isMainList])
	{
		return [[RakSerieListItem alloc] init : [_data pointerAtIndex: (index + ([item isDLList] ? 3 : 0))] : NO : initializationStage : 0];
	}
	else if(index < _sizeCache)
	{
		changeTo(_cache[index].mangaName, '_', ' ');
		return [[RakSerieListItem alloc] init : _cache[index].mangaName : NO : initializationStage : 0];
	}
	else
	{
		return [[RakSerieListItem alloc] init : @"Inconsistency :(" : NO : initializationStage : 0];
	}
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
	return item != nil && ![item isRootItem];
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
	if(item == nil)
		return 0;
	else if([item isRootItem])
	{
		if([item isMainList])
		{
			CGFloat output = content.frame.size.height - (_nbElemReadDisplayed != 0) * 21 - _nbElemReadDisplayed * 20 - (_nbElemDLDisplayed != 0) * 21 - _nbElemDLDisplayed * 20;
			return output;
		}
		else
			return 21;
	}
	else
		return 21;
}

- (void)outlineView:(NSOutlineView *)outlineView didAddRowView:(NSTableRowView *)rowView forRow:(NSInteger)row
{

}

- (void)outlineView:(NSOutlineView *)outlineView didRemoveRowView:(NSTableRowView *)rowView forRow:(NSInteger)row
{
	
}

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
		rowView = [outlineView makeViewWithIdentifier:@"StandardLine" owner:nil];
		if(rowView == nil)
		{
			_mainList = [[RakSerieMainList alloc] init: NSMakeRect(0, 0, outlineView.frame.size.height, outlineView.frame.size.height)];
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
		}
	}
	
	return rowView;
}

@end

@implementation RakTableRowView

- (void) drawBackgroundInRect:(NSRect)dirtyRect
{
	
}

@end

@implementation RakTreeView

- (NSRect)frameOfOutlineCellAtRow:(NSInteger)row
{
	NSRect superFrame = [super frameOfOutlineCellAtRow:row];
	
	if (![[self itemAtRow:row] isRootItem])
	{
        return NSMakeRect(SR_READERMODE_MARGIN_ELEMENT_OUTLINE, superFrame.origin.y, [self bounds].size.width, superFrame.size.height);
    }
    return superFrame;
}

@end
