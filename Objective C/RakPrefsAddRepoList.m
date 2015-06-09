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

@interface RakAddRepoList()
{
	ROOT_REPO_DATA ** _rootCache;
	
	NSMutableArray * rootItems;
	
	CGFloat givenWidth, contentWidth;
}

@end

enum
{
	ACTIVATION_STATE_ON,
	ACTIVATION_STATE_MIXED,
	ACTIVATION_STATE_OFF,
	
	BUTTON_WIDTH = 40,
	ROW_HEIGHT = 30,
	GIVEN_WIDTH_OFFSET = 5
};

@implementation RakAddRepoList

- (instancetype) initWithRepo : (ROOT_REPO_DATA **) root : (uint) nbRoot : (NSRect) frame
{
	if(root == NULL || !nbRoot)
		return nil;
	
	self = [self init];
	
	if (self != nil)
	{
		_rootCache = root;
		_nbRoot = nbRoot;
		givenWidth = frame.size.width - GIVEN_WIDTH_OFFSET;
		
		rootItems = [NSMutableArray array];
		
		[self initializeMain:frame];
		
		content.wantUpdateScrollview = YES;
		content.headerView = nil;
	}
	
	return self;
}

- (void) setColumnWidth : (NSTableColumn *) _column : (uint) index : (CGFloat) fullWidth
{
	[super setColumnWidth:_column :index :fullWidth - [content indentationPerLevel]];
}

#pragma mark - Data source stuffs

- (id) outlineView:(NSOutlineView *)outlineView child:(NSInteger)index ofItem:(id)item
{
	if(index < 0)
		return nil;
	
	id output;
	
	if(item == nil)
	{
		if(index >= _nbRoot)
			return nil;
		
		if([rootItems count] >= (NSUInteger) index  || (output = [rootItems objectAtIndex: (NSUInteger) index]) == nil)
		{
			output = [[RakAddRepoItem alloc] initWithRepo:_rootCache[index] :YES];
			
			if(output != nil)
				[rootItems insertObject:output atIndex:(NSUInteger) index];
		}
		else
			output = [rootItems objectAtIndex:(NSUInteger) index];
	}
	else
		output = [item getChildAtIndex:(NSUInteger) index];
	
	return output;
}

- (CGFloat)outlineView:(NSOutlineView *)outlineView heightOfRowByItem:(id)item
{
	return item == nil ? 0 : ROW_HEIGHT;
}

- (NSTableRowView *) outlineView:(NSOutlineView *)outlineView rowViewForItem : (RakAddRepoItem*) item
{
	if (![self outlineView:outlineView isGroupItem:item] && ([item class] != [RakAddRepoItem class] || ![item isRootItem]))
		return nil;
	
	RakTableRowView *rowView = [outlineView makeViewWithIdentifier:@"HeaderRowView" owner:self];
	if(rowView == nil)
	{
		rowView = [[RakTableRowView alloc] init];
		rowView.forcedWidth = givenWidth;
		rowView.identifier = @"HeaderRowView";
	}
	
	return rowView;
}

- (id) createViewWithItem : (RakAddRepoItem *) item
{
	RakAddRepoListItemView* output = [RakAddRepoListItemView alloc];
	
	output.wantActivationState = YES;
	
	output = [output initWithRepo:YES :NO :[item isRootItem] :[item getRepo] :nil];
	
	if(!contentWidth)
		contentWidth = givenWidth + GIVEN_WIDTH_OFFSET - [RakScroller width] - [content indentationPerLevel];
	
	output.responder = self;
	output.fixedWidth = contentWidth - ([item isRootItem] ? 0 : 5);
	
	return output;
}

- (void) updateViewWithItem : (RakAddRepoListItemView*) view : (RakAddRepoItem *) item
{
	[view updateContent:YES :NO :[item isRootItem] :[item getRepo] :nil];
	
	view.fixedWidth = contentWidth - ([item isRootItem] ? 0 : 5);
}

- (NSView *)outlineView:(NSOutlineView *)outlineView viewForTableColumn:(NSTableColumn *)tableColumn item:(RakAddRepoItem *)item
{
	RakAddRepoListItemView * rowView;
	
	rowView = [outlineView makeViewWithIdentifier:@"WillIEverReleaseThisSoftware?" owner:self];
	if(rowView == nil)
	{
		rowView = [self createViewWithItem : item];
		rowView.identifier = @"WillIEverReleaseThisSoftware?";
	}
	else
		[self updateViewWithItem:rowView :item];
	
	return rowView;
}

- (void) outlineView:(NSOutlineView *)outlineView didRemoveRowView:(NSTableRowView *)rowView forRow:(NSInteger)row
{
	[rowView.subviews enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop)
	 {
		 if([obj class] == [RakAddRepoListItemView class])
		 {
			 ((RakAddRepoListItemView *) obj).unloaded = YES;
			 *stop = YES;
		 }
	 }];
}

- (NSColor *) headerTextColor
{
	return [Prefs getSystemColor:COLOR_ACTIVE :nil];
}

#pragma mark - React to clicks

//We first update the flag, then we look for rows to refresh
- (void) statusTriggered : (RakPrefsRepoListItemView *) responder : (void *) repoData
{
	BOOL state = [responder getButtonState];
	if(responder.isRoot)
	{
		ROOT_REPO_DATA * repo = repoData;
		
		for(uint i = 0; i < repo->nombreSubrepo; i++)
			repo->subRepo[i].active = state;
		
		//We have to refresh all the children
		uint pos = 0;
		
		for(; pos < _nbRoot && _rootCache[pos] != repo; pos++);
		
		if(pos == _nbRoot)
			return;
	}
	else
	{
		REPO_DATA * repo = repoData;
		repo->active = state;
		
		//We have to update the parent
		uint pos = 0;
		
		for(; pos < _nbRoot && _rootCache[pos]->repoID != repo->parentRepoID; pos++);
		
		if(pos == _nbRoot)
			return;
	}
	
	[[NSNotificationCenter defaultCenter] postNotificationName:REFRESH_ADD_REPO_ITEM_STATUS object:nil];
}

@end
