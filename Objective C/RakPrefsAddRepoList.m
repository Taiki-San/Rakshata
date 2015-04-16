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

@interface RakPrefsAddRepoList()
{
	ROOT_REPO_DATA ** _rootCache;

	NSMutableArray * rootItems;
}

@end

enum
{
	ACTIVATION_STATE_ON,
	ACTIVATION_STATE_MIXED,
	ACTIVATION_STATE_OFF,
	
	BUTTON_WIDTH = 40,
	ROW_HEIGHT = 30
};

@implementation RakPrefsAddRepoList

- (instancetype) initWithRepo : (ROOT_REPO_DATA **) root : (uint) nbRoot : (NSRect) frame
{
	if(root == NULL || !nbRoot)
		return nil;
	
	self = [self init];
	
	if (self != nil)
	{
		_rootCache = root;
		_nbRoot = nbRoot;
		nbColumn = 2;
		
		rootItems = [NSMutableArray array];
		
		[self initializeMain:frame];
		
		uint pos = 0;
		NSArray * names = @[NSLocalizedString(@"PREFS-ADD-COLUMN-NAME-REPO_NAME", nil), NSLocalizedString(@"PREFS-ADD-COLUMN-NAME-ACTIVE", nil)];
		for (NSTableColumn * column in content.tableColumns)
		{
			if(pos >= [names count])
				break;
			
			NSTableHeaderCell * cell = [[NSTableHeaderCell alloc] initTextCell:[names objectAtIndex:pos++]];
			
			if (cell != nil)
			{
				[column setHeaderCell:cell];
			}
		}
	}
	
	return self;
}

- (void) setColumnWidth : (NSTableColumn *) _column : (uint) index : (CGFloat) fullWidth
{
	[super setColumnWidth:_column :index :index ? BUTTON_WIDTH : (fullWidth - BUTTON_WIDTH)];
}

#pragma mark - Data source stuffs

- (id)outlineView:(NSOutlineView *)outlineView child:(NSInteger)index ofItem:(id)item
{
	id output;
	
	if(item == nil)
	{
		if(index >= _nbRoot)
			return nil;
		
		if([rootItems count] >= index  || (output = [rootItems objectAtIndex:index]) == nil)
		{
			output = [[RakPrefsAddRepoItem alloc] initWithRepo:_rootCache[index] :YES];

			if(output != nil)
				[rootItems insertObject:output atIndex:index];
		}
		else
			output = [rootItems objectAtIndex:index];
	}
	else
		output = [item getChildAtIndex:index];
	
	return output;
}

- (CGFloat)outlineView:(NSOutlineView *)outlineView heightOfRowByItem:(id)item
{
	return item == nil ? 0 : ROW_HEIGHT;
}

- (NSTableRowView *) outlineView:(NSOutlineView *)outlineView rowViewForItem : (RakPrefsAddRepoItem*) item
{
	if (![self outlineView:outlineView isGroupItem:item] && ([item class] != [RakPrefsAddRepoItem class] || ![item isRootItem]))
		return nil;
	
	NSTableRowView *rowView = [outlineView makeViewWithIdentifier:@"HeaderRowView" owner:self];
	if (!rowView)
	{
		rowView = [[RakTableRowView alloc] init];
		rowView.identifier = @"HeaderRowView";
	}
	
	return rowView;
}

- (id) createViewWithItem : (RakPrefsAddRepoItem *) item isFirstColumn : (BOOL) isFirstColumn
{
	return [[RakPrefsRepoListItem alloc] initWithRepo:YES :!isFirstColumn :[item isRootItem] :[item getRepo] :nil];
}

- (void) updateViewWithItem : (RakPrefsRepoListItem*) view : (RakPrefsAddRepoItem *) item isFirstColumn : (BOOL) isFirstColumn
{
	[view updateContent:YES :!isFirstColumn :[item isRootItem] :[item getRepo] :nil];
}


- (NSView *)outlineView:(NSOutlineView *)outlineView viewForTableColumn:(NSTableColumn *)tableColumn item:(RakPrefsAddRepoItem *)item
{
	RakPrefsRepoListItem * rowView;
	
	if(tableColumn == nil || tableColumn == firstColumn)
	{
		rowView = [outlineView makeViewWithIdentifier:@"WillIEverReleasingThisSoftware?" owner:self];
		if(rowView == nil)
		{
			rowView = [self createViewWithItem : item isFirstColumn: YES];
			rowView.identifier = @"WillIEverReleasingThisSoftware?";
		}
		else
			[self updateViewWithItem:rowView :item isFirstColumn:YES];
	}
	else
	{
		rowView = [outlineView makeViewWithIdentifier:@"IfYou'reReadingThisProbably" owner:self];
		if(rowView == nil)
		{
			rowView = [self createViewWithItem : item isFirstColumn: NO];
			rowView.identifier = @"IfYou'reReadingThisProbably";
		}
		else
			[self updateViewWithItem:rowView :item isFirstColumn:NO];
	}
	
	return rowView;
}

@end
