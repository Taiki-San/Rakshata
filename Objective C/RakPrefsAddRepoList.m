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
		
		rootItems = [NSMutableArray array];
		
		[self initializeMain:frame];
		[content setHeaderView:nil];
	}
	
	return self;
}

- (void) setColumnWidth : (NSTableColumn *) _column : (uint) index : (CGFloat) fullWidth
{
	[super setColumnWidth:_column :index :fullWidth - [content indentationPerLevel]];
}

- (void) setFrame:(NSRect)frame
{
	[super setFrame:frame];
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

- (id) createViewWithItem : (RakPrefsAddRepoItem *) item
{
	RakPrefsRepoListItem* output = [RakPrefsRepoListItem alloc];
	
	output.wantActivationState = YES;
	
	return [output initWithRepo:YES :NO :[item isRootItem] :[item getRepo] :nil];
}

- (void) updateViewWithItem : (RakPrefsRepoListItem*) view : (RakPrefsAddRepoItem *) item
{
	[view updateContent:YES :NO :[item isRootItem] :[item getRepo] :nil];
}

- (NSView *)outlineView:(NSOutlineView *)outlineView viewForTableColumn:(NSTableColumn *)tableColumn item:(RakPrefsAddRepoItem *)item
{
	RakPrefsRepoListItem * rowView;
	
	rowView = [outlineView makeViewWithIdentifier:@"WillIEverReleasingThisSoftware?" owner:self];
	if(rowView == nil)
	{
		rowView = [self createViewWithItem : item];
		rowView.identifier = @"WillIEverReleasingThisSoftware?";
	}
	else
		[self updateViewWithItem:rowView :item];
	
	return rowView;
}

- (NSColor *) headerTextColor
{
	return [Prefs getSystemColor:GET_COLOR_ACTIVE :nil];
}

@end
