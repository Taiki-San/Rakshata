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

@implementation RakSerieList

- (id) init : (NSRect) frame : (NSString*) state
{
	self = [super init];
	
	if(self != nil)
	{
		initializationStage = INIT_FIRST_STAGE;
		[self loadContent];
		[self restoreState:state];
		
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
			[content expandItem:nil expandChildren:YES];
			
			initializationStage = INIT_OVER;
			
			uint8_t i = 0;
			for(; i < 2 && ![rootItems[i] isMainList]; i++)
			{
				if(rootItems[i] != nil && !stateSubLists[i])
					[content collapseItem:rootItems[i]];
			}
			
			if(rootItems[i] != nil)
				[rootItems[i] resetMainListHeight];
			
			[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(RakSeriesNeedUpdateContent:) name:@"RakSeriesNeedUpdateContent" object:nil];
		}
		else
			[self release];
	}
	
	return self;
}

- (void) restoreState : (NSString *) state
{
	//On va parser le context, ouééé
	NSArray *componentsWithSpaces = [state componentsSeparatedByCharactersInSet:[NSCharacterSet newlineCharacterSet]];
	NSArray *dataState = [componentsWithSpaces filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"length > 0"]];
	
	stateMainList[0] = -1;	//Selection
	
	if([dataState count] == 3 || [dataState count] == 5)
	{
		stateSubLists[0] = [[dataState objectAtIndex:0] intValue] != 0;		//Recent read
		stateSubLists[1] = [[dataState objectAtIndex:1] intValue] != 0;		//Recent DL
		
		stateMainList[1] = [[dataState objectAtIndex:[dataState count] - 1] intValue];
		
		do
		{
			if([dataState count] == 5)
			{
				int indexTeam = getIndexOfTeam((char*)[[dataState objectAtIndex:2] cStringUsingEncoding:NSASCIIStringEncoding]);
				if(indexTeam == -1)
				{
					NSLog(@"Couldn't find the repo to restore, abort :/");
					break;
				}
				
				const char * mangaNameCourt = [[dataState objectAtIndex:3] cStringUsingEncoding:NSASCIIStringEncoding];
				
				MANGAS_DATA * project = getDataFromSearch (indexTeam, mangaNameCourt, RDB_CTXCT, false);
				
				if(project == NULL || project->team == NULL)
				{
					free(project);
					NSLog(@"Couldn't find the project to restore, abort :/");
					break;
				}
				else
				{
					stateMainList[0] = project->cacheDBID;
					free(project);
				}
			}
		} while (0);
	}
	else
	{
		stateSubLists[0] = stateSubLists[1] = YES;
		stateMainList[1] = -1;
	}
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

	[content setFrame:frame];
	
	[_mainList setFrame:[self getMainListFrame : [content bounds] : content]];
	char i;
	for(i = 0; i < 3 && rootItems[i] != nil && ![rootItems[i] isMainList]; i++);

	if(i < 3)
		[rootItems[i] resetMainListHeight];
	
	[content reloadData];
}

- (void) resizeAnimation : (NSRect) frame
{
	[column setFixedWidth:frame.size.width];
	
	[content.animator setFrame:frame];
	[content setDefaultFrame:frame];
	
	frame.origin.x = frame.origin.y = 0;
	
	[_mainList resizeAnimation:[self getMainListFrame:frame:content]];

	char i;
	for(i = 0; i < 3 && rootItems[i] != nil && ![rootItems[i] isMainList]; i++);
	
	if(i < 3)
		[rootItems[i] resetMainListHeight];
	
	[content reloadData];
}

- (void) setFrameOrigin : (NSPoint) newOrigin
{
	[content setFrameOrigin:newOrigin];
}

#pragma mark - Color

- (NSColor *) getFontTopColor
{
	return [Prefs getSystemColor:GET_COLOR_INACTIVE];
}

- (NSColor *) getFontClickableColor
{
	return [Prefs getSystemColor:GET_COLOR_CLICKABLE_TEXT];
}

#pragma mark - Loading routines

- (void) loadContent
{
	_cache = getCopyCache(RDB_CTXSERIES | SORT_TEAM, &_sizeCache);
	
	if(_cache != NULL)
	{
		_data = [[NSPointerArray alloc] initWithOptions:NSPointerFunctionsOpaqueMemory];
		[self loadRecentFromDB];
	}
}

- (void) loadRecentFromDB
{
	if(_data == nil)
		return;
	
	//Recent read
	uint8_t i = 0;
	MANGAS_DATA ** recent = getRecentEntries (false, &_nbElemReadDisplayed);
	
	if(recent != NULL)
	{
		for (; i < _nbElemReadDisplayed; i++)
		{
			changeTo(recent[i]->mangaName, '_', ' ');
			[_data insertPointer:recent[i] atIndex:i];
		}
		
		free(recent);
	}
	
	for (; i < 3; i++)
		[_data insertPointer:NULL atIndex:i];
	
	//Recent DL
	recent = getRecentEntries (true, &_nbElemDLDisplayed);
	i = 0;
	
	if(recent != NULL)
	{
		for (; i < _nbElemDLDisplayed; i++)
		{
			changeTo(recent[i]->mangaName, '_', ' ');
			[_data insertPointer:recent[i] atIndex:i + 3];
		}
		
		free(recent);
	}
	
	for (; i < 3; i++)
		[_data insertPointer:NULL atIndex: i + 3];
}

- (void) reloadContent
{
	void * collector[6];
	for(int i = 0; i < 6; i++)
		collector[i] = [_data pointerAtIndex:i];
	
	RakSerieListItem *items[3] = {rootItems[0], rootItems[1], rootItems[2]};
	
	uint8_t posMainList = 0;
	for(; posMainList < 2 && rootItems[posMainList+1] != nil; rootItems[posMainList++] = nil);
	[rootItems[posMainList] retain];
	[_mainList retain];
	
	[self loadRecentFromDB];
	
	initializationStage = INIT_FIRST_STAGE;
	[content reloadData];
	
	for(uint8_t i = 0; i < posMainList; i++)
	{
		if(items[i] != nil && [items[i] isExpanded])
			[content expandItem:rootItems[i]];
		[items[i] release];
	}
	
	for(int i = 0; i < 6; i++)
		free(collector[i]);
	
	[rootItems[posMainList] release];
	[_mainList release];
}

- (void) reloadMainList
{
	id view = [_mainList getContent];
	
	if([view class] == [RakSerieMainList class])
		[(RakSerieMainList*) view reloadData];
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

//On sauvegarde l'état du tab read, du tab DL, l'élem sélectionné et la position du scroller
- (NSString*) getContextToGTFO
{
	BOOL isTabReadOpen = YES, isTabDLOpen = YES;
	
	float scrollerPosition = _mainList != nil ? [_mainList getSliderPos] : -1;
	
	//We grab superior tabs state
	for(NSInteger row = 0, nbRow = [content numberOfRows], tabExported = 0; row < nbRow && tabExported != 2; row++)
	{
		id item = [content itemAtRow:row];
		
		if(item == nil)
			continue;
		
		else if([item isRootItem] && ![item isMainList])
		{
			if(tabExported == 0)
				isTabReadOpen = [item isExpanded];
			else
				isTabDLOpen = [item isExpanded];
			
			tabExported++;
		}
	}
	//We don't really care if we can't find both tabs, variables were defined with the default value
	
	//Great, now, export the state of the main list
	NSString *currentSelection;
	MANGAS_DATA project = [_mainList getElementAtIndex:[_mainList selectedRow]];
	
	if(project.team != NULL)
		currentSelection = [NSString stringWithFormat:@"%s\n%s", project.team->URL_depot, project.mangaNameShort];
	else
		currentSelection = @"";
	
	//And, we return everything
	return [NSString stringWithFormat:@"%d\n%d\n%@\n%.0f", isTabReadOpen ? 1 : 0, isTabDLOpen ? 1 : 0, currentSelection, scrollerPosition];
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
			uint nbChildren = [self getChildrenByInitialisationStage];	//The method could increment initializationStage, aka undefined behavior
			rootItems[index] = [[RakSerieListItem alloc] init : NULL : YES : initializationStage : nbChildren];
			
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
			
			output = content.frame.size.height - ((_nbElemReadDisplayed != 0) * (25 + p) + [rootItems[0] isExpanded] * (_nbElemReadDisplayed * ([outlineView rowHeight] + p)) + (_nbElemDLDisplayed != 0) * (25 + p) + [rootItems[1] isExpanded] * (_nbElemDLDisplayed * ([outlineView rowHeight] + p)) + (25 + p) + p);
			
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
				_mainList = [[RakSerieMainList alloc] init: [self getMainListFrame : [outlineView bounds] : outlineView] : stateMainList[0] : stateMainList[1]];
			
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
			
			if([item isRootItem])
			{
				[(RakText*) rowView setFont:[NSFont fontWithName:[Prefs getFontName:GET_FONT_TITLE] size:13]];
				[(RakText*) rowView setTextColor:[self getFontTopColor]];
			}
			else
			{
				[(RakText*) rowView setFont:[NSFont fontWithName:[Prefs getFontName:GET_FONT_STANDARD] size:13]];
				[(RakText*) rowView setTextColor:[self getFontClickableColor]];
			}
		}
	}
	
	return rowView;
}

- (NSRect) getMainListFrame : (NSRect) frame : (NSOutlineView*) outlineView
{
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

- (void) RakSeriesNeedUpdateContent : (NSNotification *) notification
{
	NSNumber *requestObj = [notification.userInfo objectForKey:@"request"];
	
	if(requestObj == nil)
		return;
	
	int request = [requestObj intValue];
	
	if(request == RELOAD_RECENT || request == RELOAD_BOTH)
		[self reloadContent];
	
	if(request == RELOAD_MAINLIST || request == RELOAD_BOTH)
		[self reloadMainList];
}

#pragma mark - Drag'n Drop

- (uint) getSelfCode
{
	return GUI_THREAD_SERIES;
}

- (MANGAS_DATA) getProjectDataForDrag : (uint) row
{
	if(currentDraggedItem != nil)
	{
		MANGAS_DATA * project = [currentDraggedItem getRawDataChild];
		currentDraggedItem = nil;
		if(project != NULL)
			return *project;
	}
	
	return [super getProjectDataForDrag:row];
}

- (BOOL)outlineView:(NSOutlineView *)outlineView writeItems:(NSArray *)items toPasteboard:(NSPasteboard *)pboard
{
	if([items count] != 1 || [(RakSerieListItem *) [items objectAtIndex:0] isRootItem] || [(RakSerieListItem *) [items objectAtIndex:0] isMainList])
		return NO;
	
	[self registerToPasteboard:pboard];
	return YES;
}

- (NSDragOperation)outlineView:(NSOutlineView *)outlineView validateDrop:(id<NSDraggingInfo>)info proposedItem:(id)item proposedChildIndex:(NSInteger)index
{
	return NSDragOperationNone;	//On ne drop pas dans l'outline view
}

- (void)outlineView:(NSOutlineView *)outlineView draggingSession:(NSDraggingSession *)session willBeginAtPoint:(NSPoint)screenPoint forItems:(NSArray *)draggedItems
{
	if ([draggedItems count] != 1)
		return;
	
	currentDraggedItem = [draggedItems objectAtIndex:0];
	
	[self beginDraggingSession:session willBeginAtPoint:screenPoint forRowIndexes:[NSIndexSet indexSetWithIndex:42] withParent:outlineView];
}

- (BOOL)outlineView:(NSOutlineView *)outlineView acceptDrop:(id<NSDraggingInfo>)info item:(id)item childIndex:(NSInteger)index
{
	return NO;
}

- (NSDragOperation)draggingEntered:(id < NSDraggingInfo >)sender
{
	return NSDragOperationNone;
}

@end

