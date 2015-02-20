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

- (id) init : (NSRect) frame : (BOOL) _readerMode : (NSString*) state
{
	self = [super init];
	
	if(self != nil)
	{
		initializationStage = INIT_FIRST_STAGE;
		[Prefs getCurrentTheme:self];		//register
		[RakDBUpdate registerForUpdate:self :@selector(DBUpdated:)];
		
		rootItems[0] = rootItems[1] = rootItems[2] = nil;
		readerMode = _readerMode;
		
		[self loadContent];
		[self restoreState:state];
		
		if(_data != nil)
		{
			content = [[RakTreeView alloc] initWithFrame:frame];
			[content setFrame:frame];
			column = [[NSTableColumn alloc] initWithIdentifier:@"The Solar Empire shall fall!"];
			column.width = column.maxWidth = column.minWidth = content.frame.size.width;	//Lock the width
			
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
			
			[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(needUpdateRecent:) name:@"RakSeriesNeedUpdateRecent" object:nil];
		}
		else
			self = nil;
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
				uint indexTeam = getRepoIndexFromURL((char*)[[dataState objectAtIndex:2] cStringUsingEncoding:NSASCIIStringEncoding]);
				if(indexTeam == UINT_MAX)
				{
					NSLog(@"Couldn't find the repo to restore, abort :/");
					break;
				}
				
				const uint projectID = [[dataState objectAtIndex:3] longLongValue];
				
				PROJECT_DATA * project = getDataFromSearch (indexTeam, projectID, false);
				
				if(project == NULL || project->repo == NULL)
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

- (void) dealloc
{
	[content removeFromSuperview];
	[RakDBUpdate unRegister : self];
	[[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void) setFrame: (NSRect) frame
{
	column.width = column.maxWidth = column.minWidth = frame.size.width;

	[content setFrame:frame];
	
	[_mainList setFrame:[self getMainListFrame : [content bounds] : content]];
	
	for(byte i = 0; i < 3 && rootItems[i] != nil; i++)
	{
		if([rootItems[i] isMainList])
		{
			[rootItems[i] resetMainListHeight];
			break;
		}
	}
	
	[content reloadData];
}

- (void) resizeAnimation : (NSRect) frame
{
	column.width = column.maxWidth = column.minWidth = frame.size.width;
	
	[content.animator setFrame:frame];
	[content setDefaultFrame:frame];
	
	frame.origin = NSZeroPoint;
	
	[_mainList resizeAnimation:[self getMainListFrame : frame : content]];

	for(byte i = 0; i < 3 && rootItems[i] != nil; i++)
	{
		if([rootItems[i] isMainList])
		{
			[rootItems[i] resetMainListHeight];
			break;
		}
	}
	
	[content reloadData];
}

- (void) setFrameOrigin : (NSPoint) newOrigin
{
	[content setFrameOrigin:newOrigin];
}

#pragma mark - Color

- (NSColor *) getFontTopColor
{
	return [Prefs getSystemColor:GET_COLOR_INACTIVE : nil];
}

- (NSColor *) getFontClickableColor
{
	return [Prefs getSystemColor:GET_COLOR_CLICKABLE_TEXT : nil];
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return;

	[content reloadData];
}

#pragma mark - Loading routines

- (void) loadContent
{
	_data = [[NSPointerArray alloc] initWithOptions:NSPointerFunctionsOpaqueMemory];
	[self loadRecentFromDB];
}

- (void) loadRecentFromDB
{
	if(_data == nil)
		return;
	
	else if([_data count])	//Empty _data if required
	{
		[_data compact];
		
		uint8_t nbElem = [_data count];
		
		if(nbElem)
		{
			while (nbElem-- > 0)
			{
				free([_data pointerAtIndex:nbElem]);
				[_data removePointerAtIndex:nbElem];
			}
		}
	}
	
	//Recent read
	uint8_t i = 0;
	PROJECT_DATA ** recent = getRecentEntries(false, &_nbElemReadDisplayed);
	
	if(recent != NULL)
	{
		for (; i < _nbElemReadDisplayed; i++)
			[_data insertPointer:recent[i] atIndex:i];
		
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
			[_data insertPointer:recent[i] atIndex:i + 3];
		
		free(recent);
	}
	
	for (; i < 3; i++)
		[_data insertPointer:NULL atIndex: i + 3];
}

- (void) DBUpdated : (NSNotification *) notification
{
	if(![NSThread isMainThread])
	{
		[self performSelectorOnMainThread:@selector(reloadContent) withObject:nil waitUntilDone:NO];
		return;
	}
	else
		[self reloadContent];
}

- (void) reloadContent
{
	uint8_t posMainList = 0;
	for(; posMainList < 3 && ![rootItems[posMainList] isMainList]; posMainList++);
	
	[self loadRecentFromDB];
	
	//We remove/add sections, to reflect what we just loaded
	uint8_t data[2] = {_nbElemReadDisplayed, _nbElemDLDisplayed};
	
	for(uint8_t currentPos = 0, i = 0; i < 2; i++)
	{
		BOOL isExpected;
		
		if(i == 0)
			isExpected = [rootItems[currentPos] isRecentList];
		else
			isExpected = [rootItems[currentPos] isDLList];
		
		//Test moves
		if(data[i] != 0 && !isExpected)
		{
			//We move the data (backward to prevent replicating the first element
			for(uint8_t pos = ++posMainList; pos > currentPos; rootItems[pos] = rootItems[pos - 1], pos--);
			
			//We create the new item
			rootItems[currentPos] = [[RakSerieListItem alloc] init : NULL : YES : i == 0 ? INIT_FIRST_STAGE : INIT_SECOND_STAGE : i == 0 ? _nbElemReadDisplayed : _nbElemDLDisplayed];
			
			//We animate the insertion
			[content insertItemsAtIndexes:[NSIndexSet indexSetWithIndex:currentPos++] inParent:nil withAnimation:NSTableViewAnimationSlideLeft];
		}
		else if(data[i] == 0 && isExpected)
		{
			for(uint8_t pos = currentPos; pos < posMainList; rootItems[pos] = rootItems[pos + 1], pos++);
			rootItems[posMainList--] = nil;
			
			[content removeItemsAtIndexes:[NSIndexSet indexSetWithIndex:currentPos] inParent:nil withAnimation:NSTableViewAnimationSlideLeft];
		}
		else if(data[i] != 0)
		{
			if([rootItems[currentPos] getNbChildren] != data[i])
			{
				[rootItems[currentPos] setNbChildren:data[i] : YES];
				[content reloadItem : rootItems[currentPos] reloadChildren:YES];
			}
			
			currentPos++;
		}
	}
	
	//We expand items wanted so
	for(uint8_t i = 0; i < posMainList; i++)
	{
		if(rootItems[i] != nil && rootItems[i].expanded)
			[content expandItem:rootItems[i]];
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
		[self goToNextInitStage];
	}
	
	if(initializationStage == INIT_SECOND_STAGE)
	{
		if(_nbElemDLDisplayed)
			return _nbElemDLDisplayed;
		[self goToNextInitStage];
	}
	
	if(initializationStage == INIT_THIRD_STAGE)
	{
		return 1;
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
		RakSerieListItem* item = [content itemAtRow:row];
		
		if(item == nil)
			continue;
		
		else if([item isRootItem] && ![item isMainList])
		{
			if(tabExported == 0)
				isTabReadOpen = item.expanded;
			else
				isTabDLOpen = item.expanded;
			
			tabExported++;
		}
	}
	//We don't really care if we can't find both tabs, variables were defined with the default value
	
	//Great, now, export the state of the main list
	NSString *currentSelection;
	PROJECT_DATA project = [_mainList getElementAtIndex:[_mainList selectedRow]];
	
	if(project.repo != NULL)
		currentSelection = [NSString stringWithFormat:@"%s\n%d", project.repo->URL, project.projectID];
	else
		currentSelection = @"";
	
	//And, we return everything
	return [NSString stringWithFormat:@"%d\n%d\n%@\n%.0f", isTabReadOpen ? 1 : 0, isTabDLOpen ? 1 : 0, currentSelection, scrollerPosition];
}

#pragma mark - Context change

- (BOOL) isHidden
{
	return content.isHidden;
}

- (void) setHidden:(BOOL)hidden
{
	content.hidden = hidden;
}

- (BOOL) installOnly
{
	return _mainList != NULL ? _mainList.installOnlyMode : NO;
}

- (void) setInstallOnly : (BOOL) installOnly
{
	readerMode = installOnly;
	
	if(_mainList != nil)
		_mainList.installOnlyMode = installOnly;
}

#pragma mark - Data source to the view

- (NSInteger)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item
{
    if(item == nil)
	{
		return (_nbElemReadDisplayed != 0) + (_nbElemDLDisplayed != 0) + 1;
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
				[self goToNextInitStage];
		}
		
		output = rootItems[index];
	}
	else if(![item isMainList])
	{
		output = [item getChildAtIndex:index];
		if(output == nil)
		{
			uint8_t recentIndex =  (index + ([item isDLList] ? 3 : 0));
			
			output = [[RakSerieListItem alloc] init : [_data pointerAtIndex:recentIndex] : NO : initializationStage : 0];
			[item setChild:output atIndex:index];
			
			[_data removePointerAtIndex:recentIndex];
			[_data insertPointer:NULL atIndex:recentIndex];
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
	if(item == nil || [item isMainList])
		return NO;
	else if([item isRootItem])
	{
		if(((RakSerieListItem*)item).expanded)
			[outlineView collapseItem:item];
		else
			[outlineView expandItem:item];
		
		return NO;
	}
	
	PROJECT_DATA tmp = [item getRawDataChild];
	
	if(!tmp.isInitialized)
		return NO;

	[RakTabView broadcastUpdateContext: content : tmp : NO : VALEUR_FIN_STRUCT];
	
	return YES;
}

- (BOOL)outlineView:(NSOutlineView *)outlineView shouldExpandItem:(id)item
{
	((RakSerieListItem*) item).expanded = YES;
	return YES;
}

- (BOOL) outlineView:(NSOutlineView *)outlineView shouldCollapseItem:(id)item
{
	((RakSerieListItem*) item).expanded = NO;
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
	
	else if((output = ((RakSerieListItem*)item).height) == 0)
	{
		if([item isMainList])
		{
			CGFloat p = [outlineView intercellSpacing].height;	//Padding
			
			output = content.frame.size.height - ((_nbElemReadDisplayed != 0) * (21 + p) + rootItems[0].expanded * (_nbElemReadDisplayed * ([outlineView rowHeight] + p)) + (_nbElemDLDisplayed != 0) * (21 + p) + rootItems[1].expanded * (_nbElemDLDisplayed * ([outlineView rowHeight] + p)) + (21 + p) + p);
			
			if(output < 21)
				output = 21;
			
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
	if (![self outlineView:outlineView isGroupItem:item] && ([item class] != [RakSerieListItem class] || ![(RakSerieListItem*) item isRootItem]))
		return nil;
	
	NSTableRowView *rowView = [outlineView makeViewWithIdentifier:@"HeaderRowView" owner:self];
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
			rowView = [outlineView makeViewWithIdentifier:@"RootLineMainList" owner:self];
			if(rowView == nil)
			{
				rowView = [[RakSRSubMenu alloc] initWithText:outlineView.bounds :@"RootLineMainList"];
				((RakSRSubMenu*)rowView).barWidth = 1;
				rowView.identifier = @"RootLineMainList";
			}
		}
		else
		{
			if(_mainList == nil)
				_mainList = [[RakSerieMainList alloc] init: [self getMainListFrame : [outlineView bounds] : outlineView] : stateMainList[0] : stateMainList[1] : readerMode];
			
			rowView = [_mainList getContent];
		}
	}
	else
	{
		NSString * identifier = [item isRootItem] ? @"RootLine" : @"StandardLine";
		rowView = [outlineView makeViewWithIdentifier:identifier owner:self];
		if (rowView == nil)
		{
			rowView = [[RakText alloc] init];
			rowView.identifier = identifier;
			
			if([item isRootItem])
			{
				[(RakText*) rowView setFont:[NSFont fontWithName:[Prefs getFontName:GET_FONT_TITLE] size:14]];
				[(RakText*) rowView setTextColor:[self getFontTopColor]];
				((RakText*) rowView).forcedOffsetY = 7;
			}
			else
			{
				[(RakText*) rowView setFont:[NSFont fontWithName:[Prefs getFontName:GET_FONT_STANDARD] size:13]];
				[(RakText*) rowView setTextColor:[self getFontClickableColor]];
			}
		}
		else
		{
			if([item isRootItem])
				[(RakText*) rowView setTextColor:[self getFontTopColor]];
			else
				[(RakText*) rowView setTextColor:[self getFontClickableColor]];
		}
	}
	
	return rowView;
}

- (NSRect) getMainListFrame : (NSRect) frame : (NSOutlineView*) outlineView
{
	frame.size.width -= 2 * outlineView.indentationPerLevel + 5;
	
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
			height += 21 + padding;
			currentPortionExpanded = item.expanded;
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

- (void) needUpdateRecent : (NSNotification *) notification
{
	if([NSThread isMainThread])
		[self reloadContent];
	else
		[self performSelectorOnMainThread:@selector(needUpdateRecent:) withObject:notification waitUntilDone:YES];
}

#pragma mark - Drag'n Drop

- (uint) getSelfCode
{
	return TAB_SERIES;
}

- (PROJECT_DATA) getProjectDataForDrag : (uint) row
{
	if(currentDraggedItem != nil)
	{
		PROJECT_DATA project = [currentDraggedItem getRawDataChild];
		if(project.isInitialized)
			return project;
	}
	
	return [super getProjectDataForDrag:row];
}

- (NSString *) contentNameForDrag : (uint) row
{
	if(currentDraggedItem != nil)
	{
		PROJECT_DATA project = [currentDraggedItem getRawDataChild];
		if(project.isInitialized)
			return [[self class] contentNameForDrag:project];
	}
	
	return [super contentNameForDrag:row];
}

+ (NSString *) contentNameForDrag : (PROJECT_DATA) project
{
	BOOL shouldUseVols = [RakDragItem defineIsTomePriority:&project alreadyRefreshed:YES];
	uint delta;
	NSString * output;
	
	if(shouldUseVols)
	{
		delta = project.nombreTomes - project.nombreTomesInstalled;
		output = [NSString stringWithFormat:NSLocalizedString(@"%u-VOLUME%c", nil), delta, delta > 1 ? 's' : '\0'];
	}
	else
	{
		delta = project.nombreChapitre - project.nombreChapitreInstalled;
		output = [NSString stringWithFormat:NSLocalizedString(@"%u-CHAPTER%c", nil), delta, delta > 1 ? 's' : '\0'];
	}
	
	if(delta != 0)
		return output;
	
	return nil;
}

//Ran first, so afterward, we can assume project was refreshed
- (BOOL)outlineView:(NSOutlineView *)outlineView writeItems:(NSArray *)items toPasteboard:(NSPasteboard *)pboard
{
	if([items count] != 1)
		return NO;
	
	RakSerieListItem * item = [items objectAtIndex:0];
							  
	if(item == nil || [item isRootItem] || [item isMainList])
		return NO;
	
	[RakDragResponder registerToPasteboard:pboard];
	
	RakDragItem * pbData = [[RakDragItem alloc] init];
	
	if(pbData == nil)
		return NO;
	
	PROJECT_DATA project = [item getRawDataChild];
	
	getUpdatedChapterList(&project, true);
	getUpdatedTomeList(&project, true);
	
	[pbData setDataProject : project isTome: [[pbData class] defineIsTomePriority:&project alreadyRefreshed:YES]  element: VALEUR_FIN_STRUCT];
	
	return [pboard setData:[pbData getData] forType:PROJECT_PASTEBOARD_TYPE];
}

- (void)outlineView:(NSOutlineView *)outlineView draggingSession:(NSDraggingSession *)session willBeginAtPoint:(NSPoint)screenPoint forItems:(NSArray *)draggedItems
{
	if ([draggedItems count] != 1)
		return;
	
	currentDraggedItem = [draggedItems objectAtIndex:0];
	
	[self beginDraggingSession:session willBeginAtPoint:screenPoint forRowIndexes:[NSIndexSet indexSetWithIndex:42] withParent:outlineView];
	[RakList propagateDragAndDropChangeState :YES : [RakDragItem canDL:[session draggingPasteboard]]];
	
	currentDraggedItem = nil;
}

- (void) outlineView:(NSOutlineView *)outlineView draggingSession:(NSDraggingSession *)session endedAtPoint:(NSPoint)screenPoint operation:(NSDragOperation)operation
{
	[RakList propagateDragAndDropChangeState :NO : [RakDragItem canDL:[session draggingPasteboard]]];
}

- (BOOL)outlineView:(NSOutlineView *)outlineView acceptDrop:(id<NSDraggingInfo>)info item:(id)item childIndex:(NSInteger)index
{
	return NO;
}

@end

