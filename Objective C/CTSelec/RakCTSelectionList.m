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

#define IDENTIFIER_PRICE @"RakCTSelectionListPrice"

@implementation RakCTSelectionList

#pragma mark - Classical initialization

- (instancetype) initWithFrame : (NSRect) frame  isCompact : (BOOL) isCompact projectData : (PROJECT_DATA) project isTome : (bool) isTomeRequest selection : (long) elemSelected  scrollerPos : (long) scrollerPosition
{
	self = [self init];

	if(self != nil)
	{
		NSInteger row = -1, tmpRow = 0;

		//We check we have valid data
		_compactMode = isCompact;
		self.isTome = isTomeRequest;
		projectData = project;	//We don't protect chapter/volumen list but not really a problem as we'll only use it for drag'n drop
		chapterPrice = NULL;
		
		if(![self reloadData:projectData :NO])
		{
			self = nil;
			return nil;
		}
		
		if(elemSelected != -1)
		{
			if(self.isTome)
			{
				for(; tmpRow < amountData && ((META_TOME*)data)[tmpRow].ID < elemSelected; tmpRow++);
				
				if(tmpRow < amountData && ((META_TOME*)data)[tmpRow].ID == elemSelected)
					row = tmpRow;
			}
			else if(!self.isTome)
			{
				for(; tmpRow < amountData && ((int*)data)[tmpRow] < elemSelected; tmpRow++);
				
				if(tmpRow < amountData && ((int*)data)[tmpRow] == elemSelected)
					row = tmpRow;
			}
		}
		
		[self applyContext : frame : row :scrollerPosition];
		
		if(_tableView != nil && scrollView != nil)
		{
			_mainColumn = _tableView.tableColumns[0];
			[self updateColumnPrice : _compactMode];
			
			scrollView.wantsLayer = YES;
			scrollView.layer.backgroundColor = [NSColor whiteColor].CGColor;
			scrollView.layer.cornerRadius = 4;
		}
		else
		{
			free(data);
			free(chapterPrice);
			self = nil;
		}
	}
	
	return self;
}

- (bool) didInitWentWell
{
	return data != NULL;
}

- (BOOL) reloadData : (PROJECT_DATA) project : (BOOL) resetScroller
{
	void * newDataBuf = NULL, *newData, *newPrices = NULL, *installedData = NULL;
	uint allocSize, nbElem, nbInstalledData, nbChapterPrice = 0, *installedJumpTable = NULL;
	BOOL *installedTable = NULL;
	
	NSInteger element = _tableView != nil ? [self getSelectedElement] : 0;
	
	if(self.isTome)
	{
		allocSize = sizeof(META_TOME);
		
		nbElem = project.nombreTomes;
		newData = project.tomesFull;
		installedData = project.tomesInstalled;
		nbInstalledData = project.nombreTomesInstalled;
		
		if(newData == NULL)
			return NO;
	}
	else
	{
		allocSize = sizeof(int);
		
		nbElem = project.nombreChapitre;
		newData = project.chapitresFull;
		installedData = project.chapitresInstalled;
		nbInstalledData = project.nombreChapitreInstalled;
		
		if(newData == NULL)
			return NO;
		
		//Price table
		if(projectData.isPaid && projectData.chapitresPrix != NULL)
		{
			newPrices = calloc(projectData.nombreChapitre, sizeof(uint));
			if(newPrices != NULL)
			{
				nbChapterPrice = projectData.nombreChapitre;
				memcpy(newPrices, projectData.chapitresPrix, nbChapterPrice * sizeof(uint));
				
			}
		}
	}
	
	//Post-processing
	//Create newDataBuf
	newDataBuf = malloc(nbElem * allocSize);
	if(newDataBuf == NULL)
	{
		free(newPrices);
		return NO;
	}
	memcpy(newDataBuf, newData, nbElem * allocSize);
	
	//Set up table of installed
	if(newData != NULL && installedData != NULL)
	{
		installedTable = calloc(nbElem, sizeof(BOOL));
		installedJumpTable = malloc(nbInstalledData * sizeof(uint));
		
		if(installedTable != NULL && installedJumpTable != NULL)
		{
			uint posInst = 0;
			BOOL isTome = self.isTome;
			
			for(uint posFull = 0; posFull < nbElem && posInst < nbInstalledData; posFull++)
			{
				if((isTome && ((META_TOME*)newDataBuf)[posFull].ID == ((META_TOME*)installedData)[posInst].ID) || (!isTome && ((int*)newDataBuf)[posFull] == ((int*)installedData)[posInst]))
				{
					installedTable[posFull] = YES;
					installedJumpTable[posInst++] = posFull;
				}
			}
			
			if(posInst < nbInstalledData)
			{
				nbInstalledData = posInst;
				void * tmp = realloc(installedJumpTable, posInst * sizeof(uint));
				if(tmp != NULL)
					installedJumpTable = tmp;
			}
		}
		else
		{
			free(installedTable);		installedTable = NULL;
			free(installedJumpTable);	installedJumpTable = NULL;
		}
	}
	
	//Update the main data list
	free(data);
	data = newDataBuf;
	_nbElem = nbElem;
	amountData = self.compactMode ? _nbInstalled : nbElem;
	projectData = project;
	
	//Update installed list
	free(_installedTable);
	_installedTable = installedTable;
	free(_installedJumpTable);
	_installedJumpTable = installedJumpTable;
	_nbInstalled = nbInstalledData;
	
	//Update chapter price
	free(chapterPrice);
	chapterPrice = newPrices;
	_nbChapterPrice = nbChapterPrice;
	
	//Tell the view to update
	if(_tableView != nil)
	{
		if(resetScroller)
			[_tableView scrollRowToVisible:0];
		
		[_tableView reloadData];
		[scrollView updateScrollerState : scrollView.bounds];
		
		if(element != -1)
			[self selectRow:[self getIndexOfElement:element]];
	}
	
	return YES;
}

#pragma mark - Properties

- (uint) nbElem
{
	return self.compactMode ? _nbInstalled : _nbElem;
}

#pragma mark - Backup routine

- (NSInteger) getSelectedElement
{
	NSInteger row = selectedIndex;
	
	if(row < 0 || row > amountData)
		return -1;
	
	if(self.isTome)
		return ((META_TOME *) data)[row].ID;
	else
		return ((int *) data)[row];
}

- (void) jumpScrollerToRow : (int) row
{
	if(_tableView != nil && row != -1 && row < amountData)
		[_tableView scrollRowToVisible:row];
}
 
- (NSInteger) getIndexOfElement : (NSInteger) element
{
	if (data == NULL || (self.compactMode && _installedJumpTable == NULL))
		return -1;
	
	if (self.isTome)
	{
		if(self.compactMode)
		{
			for (uint pos = 0; pos < _nbInstalled; pos++)
			{
				if(((META_TOME *) data)[_installedJumpTable[pos]].ID == element)
					return pos;
			}
		}
		else
		{
			for (uint pos = 0; pos < _nbElem; pos++)
			{
				if(((META_TOME *) data)[pos].ID == element)
					return pos;
			}
		}
	}
	else
	{
		if(self.compactMode)
		{
			for (uint pos = 0; pos < _nbInstalled; pos++)
			{
				if(((int*) data)[_installedJumpTable[pos]] == element)
					return pos;
			}
		}
		else
		{
			for (uint pos = 0; pos < _nbElem; pos++)
			{
				if(((int *) data)[pos] == element)
					return pos;
			}
		}
	}
	
	return -1;
}

#pragma mark - Switch state

- (BOOL) compactMode
{
	return _compactMode;
}

- (void) setCompactMode : (BOOL) compactMode
{
	if(compactMode != _compactMode)
	{
		_compactMode = compactMode;
		
		amountData = [self nbElem];
		
		if(selectedIndex != -1 && _installedJumpTable != NULL)
		{
			if(compactMode)	//We go from full to installed only
			{
				uint pos = 0;
				for(; pos < _nbInstalled && _installedJumpTable[pos] != selectedIndex; pos++);
				
				if(pos < _nbInstalled)
					selectedIndex = pos;
				else
					selectedIndex = -1;
			}
			else
			{
				if(_installedJumpTable != NULL && selectedIndex < _nbInstalled)
					selectedIndex = _installedJumpTable[selectedIndex];
			}
		}
		else
			selectedIndex = -1;
		
		[self triggerInstallOnlyAnimate : compactMode];
		[self updateColumnPrice:compactMode];
	}
}

- (void) updateColumnPrice : (BOOL) isCompact
{
	if(isCompact)
	{
		[_tableView removeTableColumn:_detailColumn];
		_detailColumn = nil;
	}
	else if(projectData.isPaid && (self.isTome || chapterPrice != NULL))
	{
		_detailColumn = [[NSTableColumn alloc] initWithIdentifier:IDENTIFIER_PRICE];
		[_tableView addTableColumn:_detailColumn];
	}
	
	[self additionalResizing : _tableView.bounds.size];
}

#pragma mark - Methods to deal with tableView

- (void) additionalResizingProxy
{
	[self additionalResizing : _tableView.bounds.size];
	_resizingQueued = NO;
}

- (void) additionalResizing : (NSSize) newSize
{
	RakText * element;
	
	if(self.compactMode)
	{
		_mainColumn.width = newSize.width;
		for(uint i = 0; i < amountData; i++)
		{
			element = [_tableView viewAtColumn:0 row:i makeIfNecessary:NO];
			if(element != nil)
			{
				if(element.frame.origin.x < 0)
					[element setFrameOrigin:NSMakePoint(0, element.frame.origin.y)];
			}
		}
	}
	else
	{
		_detailColumn.width = _detailWidth;
		_mainColumn.width = newSize.width - _detailWidth;
		
		//We update every view size
		for(uint i = 0; i < amountData; i++)
		{
			element = [_tableView viewAtColumn:1 row:i makeIfNecessary:NO];
			if(element != nil && element.bounds.size.width != _detailWidth)
				[element setFrameSize:NSMakeSize(_detailWidth, element.bounds.size.height)];
		}
	}
}

- (NSView*) tableView : (NSTableView *) tableView viewForTableColumn : (NSTableColumn*) tableColumn row : (NSInteger) row
{
	RakText * output = (RakText *) [super tableView:tableView viewForTableColumn:tableColumn row:row];
	
	if(tableColumn == _detailColumn)
	{
		output.alignment = NSRightTextAlignment;

		output.stringValue = [self tableView:tableView objectValueForTableColumn:tableColumn row:row];
		[output sizeToFit];
		
		if(output.bounds.size.width > _detailWidth)
		{
			_detailWidth = output.bounds.size.width;
			
			if(!_resizingQueued)
			{
				_resizingQueued = YES;
				[self performSelectorOnMainThread:@selector(additionalResizingProxy) withObject:nil waitUntilDone:NO];
			}
		}
		else
		{
			[output setFrameSize:NSMakeSize(_detailWidth, output.bounds.size.height)];
		}
	}
	else
		output.alignment = NSLeftTextAlignment;

	return output;
}

- (NSString*) tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
	NSString * output;
	
	if(rowIndex >= amountData)
		return @"Error :(";
	
	if(self.compactMode)
	{
		if(_installedJumpTable != NULL && rowIndex < _nbInstalled)
			rowIndex = _installedJumpTable[rowIndex];
		else
			rowIndex = amountData;	//Will trigger an error
	}
	
	if(self.isTome)
	{
		META_TOME element = ((META_TOME *) data)[rowIndex];
		if(element.ID != VALEUR_FIN_STRUCT)
		{
			if(aTableColumn != _detailColumn)
			{
				if(element.readingName[0])
					output = [[NSString alloc] initWithBytes:element.readingName length:sizeof(element.readingName) encoding:NSUTF32LittleEndianStringEncoding];
				else
					output = [NSString stringWithFormat:@"Tome %d", element.readingID];
			}
			else if(_installedTable == NULL || !_installedTable[rowIndex])
				output = priceString(element.price);
			else
				output = @"";
		}
		else
			output = @"Error! Out of bounds D:";
		
	}
	else
	{
		if(aTableColumn != _detailColumn)
		{
			int ID = ((int *) data)[rowIndex];
			if(ID != VALEUR_FIN_STRUCT)
			{
				if(ID % 10)
					output = [NSString stringWithFormat:@"Chapitre %d.%d", ID / 10, ID % 10];
				else
					output = [NSString stringWithFormat:@"Chapitre %d", ID / 10];
			}
			else
				output = @"Error! Out of bounds D:";
		}
		else if(chapterPrice != NULL && rowIndex < _nbChapterPrice && (_installedTable == NULL || !_installedTable[rowIndex]))
			output = priceString(chapterPrice[rowIndex]);
		else
			output = @"";
	}
	
	return output;
}

- (NSColor *) getTextColor
{
	return nil;
}

- (NSColor *) getTextHighlightColor
{
	return nil;
}

- (NSColor*) getTextColor:(uint)column :(uint)row
{
	if(row >= amountData)
		return nil;
	
	if(self.compactMode || _installedTable[row])
		return [Prefs getSystemColor : GET_COLOR_CLICKABLE_TEXT : nil];

	return [Prefs getSystemColor : GET_COLOR_SURVOL : nil];
}

- (NSColor *) getTextHighlightColor:(uint)column :(uint)row
{
	return [Prefs getSystemColor : GET_COLOR_ACTIVE : nil];
}

#pragma mark - Smart reloading

//Ceci est l'algorithme naif en O(n^2)
//Il est viable sur < 1000 données, mais pourrait poser des problèmes à l'avenir
//Un algo alternatif, en O(n*log(n)) serait de faire des copies de _oldData et de _newData
//Les trier (avec un introsort (cf implé de g++), qsort est en n^2 dans notre cas générique)
//Retirer les doublons (vérifier que les positions collent, un déplacement doit être detecté)
//Regarder les positions de ce qu'il reste et voilà
- (void) smartReload : (void*) oldData : (uint) nbElemOld : (void*) newData : (uint) nbElemNew
{
	if(_tableView != nil)
		return;
	
	NSMutableIndexSet * new = [NSMutableIndexSet new], * old = [NSMutableIndexSet new];
	uint newElem = 0, oldElem = 0;

	if(oldData == NULL)
	{
		while(newElem < nbElemNew)
			[new addIndex : newElem++];
	}
	else if(newData == NULL)
	{
		while(oldElem < nbElemOld)
			[old addIndex : oldElem++];
	}
	else
	{
		BOOL isTome = self.isTome;
		int current;
		
		for(uint posNew = 0, posOld = 0, i; posNew < nbElemNew; posNew++)
		{
			i = posOld;
			
			if(isTome)
				for(current = ((META_TOME*)newData)[posNew].ID; i < nbElemOld && ((META_TOME*)oldData)[i].ID != current; i++);
			else
				for(current = ((int*)newData)[posNew]; i < nbElemOld && ((int*)oldData)[i] != current; i++);
			
			if(i < nbElemOld)
			{
				for(; posOld < i; oldElem++)
					[old addIndex : posOld++];
			}
			else
			{
				[new addIndex:posNew];
				newElem++;
			}
		}
	}
	
	if(oldElem)
		[_tableView removeRowsAtIndexes:old withAnimation:NSTableViewAnimationSlideLeft];
	if(newElem)
		[_tableView insertRowsAtIndexes:new withAnimation:NSTableViewAnimationSlideLeft];
}

- (void) triggerInstallOnlyAnimate : (BOOL) enter
{
	BOOL foundOne = NO;
	NSMutableIndexSet * index = [NSMutableIndexSet new];

	for(uint i = 0; i < _nbElem; i++)
	{
		if(!_installedTable[i])
		{
			[index addIndex:i];
			foundOne = YES;
		}
	}
	
	if(foundOne)
	{
		if(enter)
			[_tableView removeRowsAtIndexes:index withAnimation:NSTableViewAnimationSlideLeft];
		else
			[_tableView insertRowsAtIndexes:index withAnimation:NSTableViewAnimationSlideLeft];
	}
}

#pragma mark - Get result from NSTableView

- (void)tableViewSelectionDidChange:(NSNotification *)notification;
{
	if(selectedIndex != -1 && selectedIndex < amountData)
	{
		BOOL installed = self.compactMode || (_installedTable != NULL && _installedTable[selectedIndex]);
		
		[[NSNotificationCenter defaultCenter] postNotificationName: @"RakCTSelectedManually" object:nil userInfo: @{@"index": @(selectedIndex), @"isTome" : @(self.isTome), @"isInstalled" : @(installed)}];
	}
}

#pragma mark - Drag and drop support

- (uint) getSelfCode
{
	return TAB_CT;
}

- (PROJECT_DATA) getProjectDataForDrag : (uint) row
{
	return projectData;
}

- (NSString *) contentNameForDrag : (uint) row
{
	return [self tableView:nil objectValueForTableColumn:nil row:row];
}

- (void) fillDragItemWithData:(RakDragItem *)item :(uint)row
{
	int selection;
	
	if(self.isTome)
	{
		selection = (((META_TOME *) data)[row]).ID;
		item.price = (((META_TOME *) data)[row]).price;
	}
	else
	{
		selection = ((int *) data)[row];
		
		if(chapterPrice != NULL && row < _nbChapterPrice)
			item.price = chapterPrice[row];
	}
	
	[item setDataProject:getCopyOfProjectData(projectData) isTome:self.isTome element:selection];
}

- (void) additionalDrawing : (RakDragView *) _draggedView : (uint) row
{
	if(!self.compactMode && _installedTable != NULL && row < _nbElem && !_installedTable[row])
	{
		if(data == NULL)
			return;
		
		//We may have to add the price
		uint price = 0;
		if(self.isTome)
			price = ((META_TOME*)data)[row].price;
		else if(chapterPrice != NULL)
			price = chapterPrice[row];
		
		if(price != 0)
		{
			RakText * priceView = [[RakText alloc] init];
			priceView.textColor = [Prefs getSystemColor:GET_COLOR_CLICKABLE_TEXT :nil];
			priceView.stringValue = priceString(price);
			[priceView sizeToFit];
			
			[_draggedView addPrice:priceView];
		}
	}
}

@end
