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

@implementation RakSerieQAItem

- (id) init : (void*) data : (BOOL) isRootItem
{
	self = [super init];
	
	if(self != nil)
	{
		_isRootItem = isRootItem;
		
		if(_isRootItem)
		{
			dataRoot = data;
			dataChild = NULL;
		}
		else
		{
			dataRoot = NULL;
			dataChild = data;
		}
	}
	
	return self;
}

- (BOOL) isRootItem
{
	return _isRootItem;
}

- (NSString*) getData
{
	if(_isRootItem && dataRoot != NULL)
		return dataRoot;
	else if(!_isRootItem && dataChild != NULL)
		return [NSString stringWithCString:dataChild->mangaName encoding:NSASCIIStringEncoding];
	else
		return @"Internal error :(";
}

@end

@implementation RakSerieQA

- (id) init : (NSRect) frame : (BOOL) isRecentDownload
{
	self = [super init];
	
	if(self != nil)
	{
		initializationOver = NO;
		_isRecentDownload = isRecentDownload;
		[self loadContent];
		
		if(_data != nil)
		{
			content = [[RakTreeView alloc] initWithFrame:frame];
			NSTableColumn * column = [[NSTableColumn alloc] initWithIdentifier:@"The Solar Empire shall fall!"];
			[column setWidth:content.frame.size.width];
			
			//Customisation
			[content setIntercellSpacing:NSMakeSize(0, 1)];
			[content setBackgroundColor:[NSColor clearColor]];
			[content setSelectionHighlightStyle:NSTableViewSelectionHighlightStyleNone];
			[content setFocusRingType:NSFocusRingTypeNone];
			
			//End of setup
			[content setDelegate:self];
			[content setDataSource:self];
			[content addTableColumn:column];
			[content setOutlineTableColumn:column];
			[content expandItem:nil expandChildren:YES];
			initializationOver = YES;
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
		
		changeTo(_cache[_sizeCache - 1].mangaName, '_', ' ');
		[_data addPointer:&_cache[_sizeCache - 1]];
		
		changeTo(_cache[_sizeCache - 2].mangaName, '_', ' ');
		[_data addPointer:&_cache[_sizeCache - 2]];
		
		changeTo(_cache[_sizeCache - 3].mangaName, '_', ' ');
		[_data addPointer:&_cache[_sizeCache - 3]];
		
		_nbElemDisplayed = 3;
	}
}

#pragma mark - Data source to the view

- (NSInteger)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item
{
    return (item == nil) ? 1 : ([item isRootItem] ? _nbElemDisplayed : 0);
}


- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item
{
	return item == nil ? YES : [item isRootItem];
}


- (id)outlineView:(NSOutlineView *)outlineView child:(NSInteger)index ofItem:(id)item
{
	if(item == nil)
		return [[RakSerieQAItem alloc] init : (_isRecentDownload ? @"Téléchargé récemment" : @"Consulté récemment") : YES];
	else
		return [[RakSerieQAItem alloc] init : [_data pointerAtIndex:index] : NO];
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
	return [item isRootItem];
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
	RakText *rowView = [outlineView makeViewWithIdentifier:@"StandardLine" owner:nil];
	if (!rowView)
	{
		rowView = [[RakText alloc] init];
		rowView.identifier = @"StandardLine";
		[rowView setTextColor:[self getFontColor]];
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
	
	if (row > 0)
	{
        return NSMakeRect(SR_READERMODE_MARGIN_ELEMENT_OUTLINE, superFrame.origin.y, [self bounds].size.width, superFrame.size.height);
    }
    return superFrame;
}

@end
