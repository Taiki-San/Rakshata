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
#define DEFAULT_MAIN_WIDTH 110

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
		chapterPrice = NULL;
		projectData.cacheDBID = UINT_MAX;	//Prevent incorrect beliefs we are updating a project
		
		//We don't protect chapter/volume list but not really a problem as we'll only use it for drag'n drop
		[self reloadData:project :NO];
		
		if(elemSelected != -1)
		{
			if(self.isTome)
			{
				for(; tmpRow < _nbData && ((META_TOME*)_data)[tmpRow].ID < elemSelected; tmpRow++);
				
				if(tmpRow < _nbData && ((META_TOME*)_data)[tmpRow].ID == elemSelected)
					row = tmpRow;
			}
			else if(!self.isTome)
			{
				for(; tmpRow < _nbData && ((int*)_data)[tmpRow] < elemSelected; tmpRow++);
				
				if(tmpRow < _nbData && ((int*)_data)[tmpRow] == elemSelected)
					row = tmpRow;
			}
		}
		
		[self applyContext : frame : row :scrollerPosition];
		
		if(_tableView != nil && scrollView != nil)
		{
			_mainColumns = @[[_tableView.tableColumns firstObject]];
			_nbCoupleColumn = 1;
			[self updateMultiColumn: _compactMode : scrollView.bounds.size];
			
			scrollView.wantsLayer = YES;
			scrollView.layer.backgroundColor = [NSColor whiteColor].CGColor;
			scrollView.layer.cornerRadius = 4;
		}
		else
		{
			free(_data);
			free(chapterPrice);
			self = nil;
		}
	}
	
	return self;
}

- (bool) didInitWentWell
{
	return YES;
}

- (BOOL) reloadData : (PROJECT_DATA) project : (BOOL) resetScroller
{
	void * newDataBuf = NULL, *newData, *newPrices = NULL, *installedData = NULL, *oldData = NULL, *oldInstalled = NULL;
	uint allocSize, nbElem, nbInstalledData, nbChapterPrice = 0, *installedJumpTable = NULL, nbOldElem, nbOldInstalled = 0;
	BOOL *installedTable = NULL, sameProject = projectData.cacheDBID == project.cacheDBID, isTome = self.isTome;
	
	NSInteger element = _tableView != nil ? [self getSelectedElement] : 0;
	
	if(isTome)
	{
		allocSize = sizeof(META_TOME);
		
		nbElem = project.nombreTomes;
		newData = project.tomesFull;
		nbInstalledData = project.nombreTomesInstalled;
		installedData = project.tomesInstalled;
		
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
		if(project.isPaid && project.chapitresPrix != NULL)
		{
			newPrices = calloc(project.nombreChapitre, sizeof(uint));
			if(newPrices != NULL)
			{
				nbChapterPrice = project.nombreChapitre;
				memcpy(newPrices, project.chapitresPrix, nbChapterPrice * sizeof(uint));
			}
		}
	}
	
	//Post-processing
	//Create newDataBuf
	newDataBuf = malloc((nbElem + 1) * allocSize);
	if(newDataBuf == NULL)
	{
		free(newPrices);
		return NO;
	}
	if(isTome)
		copyTomeList(newData, nbElem, newDataBuf);
	else
		memcpy(newDataBuf, newData, (nbElem + 1) * allocSize);
	
	//Set up table of installed
	if(newData != NULL && installedData != NULL)
	{
		installedTable = calloc(nbElem, sizeof(BOOL));
		installedJumpTable = malloc(nbInstalledData * sizeof(uint));
		
		if(installedTable != NULL && installedJumpTable != NULL)
		{
			uint posInst = 0;
			
			for(uint posFull = 0; posFull < nbElem && posInst < nbInstalledData; posFull++)
			{
				if((isTome && ((META_TOME*)newDataBuf)[posFull].ID == ((META_TOME*)installedData)[posInst].ID) || (!isTome && ((int*)newDataBuf)[posFull] == ((int*)installedData)[posInst]))
				{
					installedTable[posFull] = YES;
					installedJumpTable[posInst++] = posFull;
				}
			}
			
			if(!posInst)	//No data
			{
				free(installedTable);		installedTable = NULL;
				free(installedJumpTable);	installedJumpTable = NULL;
			}
			else if(posInst < nbInstalledData)
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
	
	//We copy the old data structure
	oldData = _data;
	nbOldElem = _nbElem;
	if(self.compactMode)
	{
		oldInstalled = _installedJumpTable;
		_installedJumpTable = NULL;
		nbOldInstalled = _nbInstalled;
	}
	
	//Update the main data list
	_data = newDataBuf;
	_nbElem = nbElem;
	_nbData = self.compactMode ? nbInstalledData : nbElem;
	projectData = project;
	
	//Update installed list

	free(_installedJumpTable);
	_installedJumpTable = (void*) _installedTable;
	_installedTable = installedTable;
	installedTable = (void*) _installedJumpTable;

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
		
		//Add the column
		[self updateMultiColumn: self.compactMode : scrollView.bounds.size];
		
		//We get a usable data structure is required
		if(sameProject)
		{
			void * newInstalledData = _data;
			uint nbNewData = _nbElem;
			if(self.compactMode)
			{
				//Old data
				void * oldDataBak = oldData;
				oldData = buildInstalledList(oldData, nbOldElem, oldInstalled, nbOldInstalled, isTome);
				nbOldElem = nbOldInstalled;
				
				if(isTome)
					freeTomeList(oldDataBak, true);
				else
					free(oldDataBak);
				
				//New data
				newInstalledData = buildInstalledList(_data, _nbElem, _installedJumpTable, _nbInstalled, isTome);
			}
			
			[self smartReload : [self getSmartReloadData:oldData :isTome :nbOldElem :installedTable] :nbOldElem
							  : [self getSmartReloadData:newInstalledData :isTome :nbNewData :_installedTable] :nbNewData];
			
			if(self.compactMode)
				free(newInstalledData);
		}
		else
		{
			uint newElem = _nbElem;
			
			if(self.compactMode)
			{
				nbOldElem = nbOldInstalled;
				newElem = _nbInstalled;
			}
			[self fullAnimatedReload : nbOldElem :newElem];
		}

		[scrollView updateScrollerState : scrollView.bounds];
		
		if(element != -1)
		{
			_UIOnlySelection = YES;
			[self selectRow:[self getIndexOfElement:element]];
			_UIOnlySelection = NO;
		}
		
	}


	if(isTome)
		freeTomeList(oldData, true);
	else
		free(oldData);
	free(oldInstalled);
	free(installedTable);
	
	return YES;
}

- (void) flushContext : (BOOL) animated
{
	if(animated)
		[_tableView removeRowsAtIndexes:[NSMutableIndexSet indexSetWithIndexesInRange:NSMakeRange(0, _tableView.numberOfRows)] withAnimation:NSTableViewAnimationSlideLeft];
	
	_nbElem = _nbInstalled = 0;
	
	if(self.isTome)
		freeTomeList(_data, true);
	else
	{
		free(_data);
		free(chapterPrice);	chapterPrice = NULL;
	}
	
	_data = NULL;
	free(_installedJumpTable);	_installedJumpTable = NULL;
	free(_installedTable);		_installedTable = NULL;
	
	if(!animated)
		[_tableView noteNumberOfRowsChanged];
}

#pragma mark - Properties

- (uint) cacheID
{
	return projectData.isInitialized ? projectData.cacheDBID : UINT_MAX;
}

- (BOOL) isEmpty
{
	return projectData.isInitialized ? (self.isTome ? projectData.nombreTomes == 0 : projectData.nombreChapitre == 0) : YES;
}

- (uint) nbElem
{
	return self.compactMode ? _nbInstalled : _nbElem;
}

#pragma mark - Backup routine

- (NSInteger) getSelectedElement
{
	NSInteger row = selectedRowIndex;
	
	if(row < 0 || row > _nbData)
		return -1;
	
	if(self.isTome)
		return ((META_TOME *) _data)[row].ID;
	else
		return ((int *) _data)[row];
}

- (void) jumpScrollerToRow : (int) row
{
	if(_tableView != nil && row != -1 && row < _nbData)
		[_tableView scrollRowToVisible:row];
}
 
- (NSInteger) getIndexOfElement : (NSInteger) element
{
	if (_data == NULL || (self.compactMode && _installedJumpTable == NULL))
		return -1;
	
	if (self.isTome)
	{
		if(self.compactMode)
		{
			for (uint pos = 0; pos < _nbInstalled; pos++)
			{
				if(((META_TOME *) _data)[_installedJumpTable[pos]].ID == element)
					return pos;
			}
		}
		else
		{
			for (uint pos = 0; pos < _nbElem; pos++)
			{
				if(((META_TOME *) _data)[pos].ID == element)
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
				if(((int*) _data)[_installedJumpTable[pos]] == element)
					return pos;
			}
		}
		else
		{
			for (uint pos = 0; pos < _nbElem; pos++)
			{
				if(((int *) _data)[pos] == element)
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
		
		_nbData = [self nbElem];
		
		if(selectedRowIndex != -1 && _installedJumpTable != NULL)
		{
			if(compactMode)	//We go from full to installed only
			{
				uint pos = 0;
				for(; pos < _nbInstalled && _installedJumpTable[pos] != selectedRowIndex; pos++);
				
				if(pos < _nbInstalled)
					selectedRowIndex = pos;
				else
					selectedRowIndex = -1;
			}
			else
			{
				if(_installedJumpTable != NULL && selectedRowIndex < _nbInstalled)
					selectedRowIndex = _installedJumpTable[selectedRowIndex];
			}
		}
		else
			selectedRowIndex = -1;
		
		//Because of how things have to be handled when er get in vs out, the call order change
		if(compactMode)
		{
			uint nbColumn = _nbCoupleColumn;
			[self updateMultiColumn : compactMode : scrollView.bounds.size];
			[self triggerInstallOnlyAnimate : compactMode numberOfColumns: nbColumn];
		}
		else
		{
			[self triggerInstallOnlyAnimate : compactMode numberOfColumns: _nbCoupleColumn];
			[self updateMultiColumn : compactMode : scrollView.bounds.size];
		}
	}
}

#pragma mark - Methods to deal with tableView

- (void) updateMultiColumn :(NSSize)scrollviewSize
{
	[self updateMultiColumn : self.compactMode : scrollviewSize];
}

- (void) updateMultiColumn : (BOOL) isCompact : (NSSize) scrollviewSize
{
	NSSize initialSize = _tableView.bounds.size;

	if(isCompact)	//We clean everything up if required
	{
		if(_detailColumns != nil)
		{
			[_detailColumns enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {	[_tableView removeTableColumn:obj];	}];
			_detailColumns = nil;
		}
		
		if(_nbCoupleColumn > 1 && _mainColumns != nil)
		{
			[_mainColumns enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
				if(idx)
					[_tableView removeTableColumn:obj];
			}];
			
			_mainColumns = @[_mainColumns.firstObject];
		}
		_nbCoupleColumn = 1;
		_nbElemPerCouple = 1;
		_detailWidth = 0;
	}
	else
	{
		[_tableView beginUpdates];
		
		//First, we define if we need a detail column, and update the context if required
		BOOL paidContent = projectData.isPaid && (self.isTome || chapterPrice != NULL);
		if(!paidContent && _detailColumns != nil)
		{
			[_detailColumns enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {	[_tableView removeTableColumn:obj];	}];
			_detailColumns = nil;
			_nbElemPerCouple = 1;
			_detailWidth = 0;
		}
		else if(paidContent && _detailColumns == nil)
		{
			NSMutableArray * newDetails = [NSMutableArray new];
			NSTableColumn * column;
			BOOL needToInjectMainColumns = _nbCoupleColumn > 1;

			_nbElemPerCouple = 2;
			_detailWidth = 60;
			
			if(needToInjectMainColumns)
			{
				[_mainColumns enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
					if(idx)
						[_tableView removeTableColumn:obj];
				}];
			}
			
			for(uint pos = 0; pos < _nbCoupleColumn; pos++)
			{
				column = [[NSTableColumn alloc] initWithIdentifier:IDENTIFIER_PRICE];
				column.width = _detailWidth;
				[_tableView addTableColumn:column];
				
				if(needToInjectMainColumns && pos + 1 < _nbCoupleColumn)
					[_tableView addTableColumn:[_mainColumns objectAtIndex:pos + 1]];
				
				[newDetails addObject:column];
			}
			
			if([newDetails firstObject] != nil)	//not empty
				_detailColumns = [NSArray arrayWithArray:newDetails];
			else
				_detailColumns = nil;
		}
		
		if(_mainColumns == nil)
			_mainColumns = @[[_tableView.tableColumns firstObject]];
		
		//Great, now that we are up to date, we're going to check if we need to add new columns
		NSSize singleColumn = NSMakeSize(DEFAULT_MAIN_WIDTH + (_detailWidth == 0 ? 15 : _detailWidth), scrollviewSize.height / _nbCoupleColumn);
		uint nbColumn = 1, oldNbColumn = _nbCoupleColumn;
		int newColumns;
		
		//Define number of columns
		while(![scrollView willContentFitInHeight:singleColumn.height * nbColumn] && singleColumn.width * nbColumn <= scrollviewSize.width)
			nbColumn++;
		
		if(nbColumn > 1 && singleColumn.width * nbColumn <= scrollviewSize.width)	//We can't get under a single column
			nbColumn--;
		
		_nbCoupleColumn = nbColumn;
		newColumns = nbColumn - oldNbColumn;
		
		if(newColumns > 0)		//We need to add columns
		{
			NSMutableArray *newMainColumns = [NSMutableArray arrayWithArray:_mainColumns], *newDetailColumns = [NSMutableArray arrayWithArray:_detailColumns];
			NSTableColumn * column;
			
			for(uint pos = 0; pos < newColumns; pos++)
			{
				column = [[NSTableColumn alloc] initWithIdentifier:RAKLIST_MAIN_COLUMN_ID];
				[_tableView addTableColumn:column];
				[newMainColumns addObject:column];
				
				if(paidContent)
				{
					column = [[NSTableColumn alloc] initWithIdentifier:IDENTIFIER_PRICE];
					[_tableView addTableColumn:column];
					[newDetailColumns addObject:column];
				}
			}
			
			_mainColumns = newMainColumns;

			if([newDetailColumns firstObject] != nil)	//not empty
				_detailColumns = [NSArray arrayWithArray:newDetailColumns];
			else
				_detailColumns = nil;
		}
		else if(newColumns < 0)	//We have to remove some
		{
			NSMutableArray * newMainColumns = [NSMutableArray new];
			
			[_mainColumns enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
				if(idx < nbColumn)
					[newMainColumns addObject:obj];
				else
					[_tableView removeTableColumn:obj];
			}];
			_mainColumns = [NSArray arrayWithArray:newMainColumns];
			
			[newMainColumns removeAllObjects];
			
			[_detailColumns enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
				if(idx < nbColumn)
					[newMainColumns addObject:obj];
				else
					[_tableView removeTableColumn:obj];
			}];
			
			if([newMainColumns firstObject] != nil)	//not empty
				_detailColumns = [NSArray arrayWithArray:newMainColumns];
			else
				_detailColumns = nil;
		}
		
		if(newColumns != 0)
		{
			[_tableView reloadDataForRowIndexes:[NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, [_tableView numberOfRows])] columnIndexes:[NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, [_tableView numberOfColumns])]];
			[self updateRowNumber];
		}
		[_tableView endUpdates];
	}
	
	//Adding or removing columns will impact tableview size
	if(!NSEqualSizes(initialSize, _tableView.bounds.size))
		[_tableView setFrameSize:initialSize];

	[self additionalResizing : initialSize];
}

- (void) updateRowNumber
{
	if(!_rowNumberUpdateQueued)
	{
		_rowNumberUpdateQueued = YES;
		[self performSelectorOnMainThread:@selector(_updateRowNumber) withObject:nil waitUntilDone:NO];
	}
}

- (void) _updateRowNumber
{
	if(_rowNumberUpdateQueued)
	{
		[_tableView noteNumberOfRowsChanged];
		_rowNumberUpdateQueued = NO;
	}
}

- (void) additionalResizingProxy
{
	[self additionalResizing : _tableView.bounds.size];
	[self reloadSize];
	_resizingQueued = NO;
}

- (void) additionalResizing : (NSSize) newSize
{
	RakText * element;
	uint width = newSize.width / _nbCoupleColumn, detailWidth = self.compactMode ? 0 : _detailWidth, mainWidth = width - detailWidth;
	
	[_mainColumns enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
		((NSTableColumn*) obj).width = mainWidth;
	}];
	
	if(self.compactMode)
	{
		for(uint column = 0; column < _nbCoupleColumn; column++)
		{
			for(uint i = 0, rows = [_tableView numberOfRows]; i < rows; i++)
			{
				element = [_tableView viewAtColumn:2 * column row:i makeIfNecessary:NO];
				if(element != nil)
				{
					if(element.frame.origin.x < 0)
						[element setFrameOrigin:NSMakePoint(0, element.frame.origin.y)];
				}
			}
		}
	}
	else if(_detailColumns != nil) //We update every view size
	{
		[_detailColumns enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
			((NSTableColumn *)obj).width = _detailWidth;
		}];
		
		for(uint column = 0; column < _nbCoupleColumn; column++)
		{
			for(uint i = 0, rows = [_tableView numberOfRows]; i < rows; i++)
			{
				element = [_tableView viewAtColumn : 2 * column + 1 row:i makeIfNecessary:NO];
				if(element != nil && element.bounds.size.width != _detailWidth)
					[element setFrameSize:NSMakeSize(_detailWidth, element.bounds.size.height)];
			}
		}
	}
}

- (NSView*) tableView : (NSTableView *) tableView viewForTableColumn : (NSTableColumn*) tableColumn row : (NSInteger) row
{
	RakText * output = (RakText *) [super tableView:tableView viewForTableColumn:tableColumn row:row];
	
	if(_detailColumns != nil && [_detailColumns containsObject : tableColumn])
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

- (NSString*) tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)rowIndex
{
	NSString * output;
	NSUInteger column = 0;
	BOOL isDetails = NO;
	
	column = [_mainColumns indexOfObject:tableColumn];
	if(column == NSNotFound)
	{
		if(_detailColumns == nil || (column = [_detailColumns indexOfObject:tableColumn]) == NSNotFound)
			return @"Error :(";
		else
			isDetails = YES;
	}
	
	rowIndex = rowIndex * _nbCoupleColumn + column;
	
	if(rowIndex >= _nbData)	//Too much entry?
	{
		if(rowIndex / _nbCoupleColumn > _nbData / _nbCoupleColumn)	//Inconsistency
		{
			[self performSelectorOnMainThread:@selector(updateRowNumber) withObject:nil waitUntilDone:NO];
			return @"Error :(";
		}
		else	//We're just on the last incomplete row
		{
			return @"";
		}
	}
	
	if(self.compactMode)
	{
		if(_installedJumpTable != NULL && rowIndex < _nbInstalled)
			rowIndex = _installedJumpTable[rowIndex];
		else
			rowIndex = _nbData;	//Will trigger an error
	}
	
	if(self.isTome)
	{
		META_TOME element = ((META_TOME *) _data)[rowIndex];
		if(element.ID != VALEUR_FIN_STRUCT)
		{
			if(_detailColumns == nil || !isDetails)
			{
				if(element.readingName[0])
					output = getStringForWchar(element.readingName);
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
		if(_detailColumns == nil || !isDetails)
		{
			int ID = ((int *) _data)[rowIndex];
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
		{
			output = [NSString stringWithFormat:@"%@%c", priceString(chapterPrice[rowIndex]) , column == _nbCoupleColumn-1 ? '\0' : '	'];
		}
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
	row = row * _nbCoupleColumn + column / _nbElemPerCouple;
	if(row >= _nbData)
		return nil;
	
	if(self.compactMode || (_installedTable != NULL && _installedTable[row]))
		return [Prefs getSystemColor : GET_COLOR_CLICKABLE_TEXT : nil];

	return [Prefs getSystemColor : GET_COLOR_SURVOL : nil];
}

- (NSColor *) getTextHighlightColor:(uint)column :(uint)row
{
	return [Prefs getSystemColor : GET_COLOR_ACTIVE : nil];
}

#pragma mark - Smart reloading

- (SR_DATA *) getSmartReloadData : (void*) data : (BOOL) isTome : (uint) nbElem : (BOOL *) installed
{
	if(!nbElem)
		return NULL;
	
	SR_DATA * output = calloc(nbElem, sizeof(SR_DATA));
	
	if(output != NULL)
	{
		for(uint i = 0; i < nbElem; i++)
		{
			output[i].data = isTome ? ((META_TOME*)data)[i].ID : ((int*)data)[i];
			output[i].installed = installed == NULL ? NO : installed[i];
		}
	}
	
	return output;
}

- (void) triggerInstallOnlyAnimate : (BOOL) enter numberOfColumns : (uint) nbColumns
{
	BOOL foundOneIn = NO, foundOneOut = NO;
	uint nbRows = [_tableView numberOfRows];
	NSMutableIndexSet * indexIn = [NSMutableIndexSet new], * indexOut = [NSMutableIndexSet new];

	if(_installedTable != NULL)
	{
		if(enter)
		{
			for(uint i = 0, rank = 0; i < _nbElem; i++)
			{
				if(_installedTable[i])
				{
					if(i % nbColumns)
					{
						[indexIn addIndex:rank];
						foundOneIn = YES;
					}
					
					rank++;
				}
				else if(i % nbColumns == 0 && i / nbColumns < nbRows)
				{
					[indexOut addIndex : i / nbColumns];
					foundOneOut = YES;
				}
			}
		}
		else
		{
			for(uint i = 0, size = MIN(_nbElem, [_tableView numberOfRows]); i < size; i++)
			{
				if(!_installedTable[i])
				{
					[indexIn addIndex:i];
					foundOneOut = YES;
				}
			}
		}
	}
	
	if(foundOneIn || foundOneOut)
	{
		[NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
			
			[context setDuration:CT_TRANSITION_ANIMATION];
			
			if(foundOneOut)
				[_tableView removeRowsAtIndexes:indexOut withAnimation:NSTableViewAnimationSlideLeft];
			if(foundOneIn)
				[_tableView insertRowsAtIndexes:indexIn withAnimation:NSTableViewAnimationSlideLeft];
			
		} completionHandler:^{}];
	}
}

#pragma mark - Get result from NSTableView

- (BOOL) tableView : (RakTableView *) tableView shouldSelectRow:(NSInteger)rowIndex
{
	NSInteger index = rowIndex * _nbCoupleColumn + tableView.preCommitedLastClickedColumn;
	
	if(!_UIOnlySelection && !self.compactMode && index >= 0 && index < _nbElem && _installedTable != NULL && !_installedTable[index])
	{
		CGFloat oldselectedRowIndex = selectedRowIndex;
		selectedRowIndex = rowIndex;
		[self tableViewSelectionDidChange:nil];
		selectedRowIndex = oldselectedRowIndex;
		
		return NO;
	}
	else
		return [super tableView:tableView shouldSelectRow:rowIndex];
}

- (void)tableViewSelectionDidChange:(NSNotification *)notification;
{
	NSInteger index = selectedRowIndex *_nbCoupleColumn + selectedColumnIndex;
	
	if(selectedRowIndex != -1 && index < [self nbElem])
	{
		BOOL installed = self.compactMode || (_installedTable != NULL && _installedTable[index]);
		
		[[NSNotificationCenter defaultCenter] postNotificationName: @"RakCTSelectedManually" object:nil userInfo: @{@"index": @(index), @"isTome" : @(self.isTome), @"isInstalled" : @(installed)}];
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
	return [self tableView:_tableView objectValueForTableColumn:[_tableView.tableColumns objectAtIndex:_tableView.preCommitedLastClickedColumn] row:row];
}

- (void) fillDragItemWithData:(RakDragItem *)item :(uint)row
{
	int selection;
	row = row / _nbCoupleColumn + _tableView.preCommitedLastClickedColumn / _nbElemPerCouple;
	
	if(self.isTome)
	{
		selection = (((META_TOME *) _data)[row]).ID;
		item.price = (((META_TOME *) _data)[row]).price;
	}
	else
	{
		selection = ((int *) _data)[row];
		
		if(chapterPrice != NULL && row < _nbChapterPrice)
			item.price = chapterPrice[row];
	}
	
	[item setDataProject:getCopyOfProjectData(projectData) isTome:self.isTome element:selection];
}

- (void) additionalDrawing : (RakDragView *) _draggedView : (uint) row
{
	row = row / _nbCoupleColumn + _tableView.preCommitedLastClickedColumn / _nbElemPerCouple;

	if(!self.compactMode && _installedTable != NULL && row < _nbElem && !_installedTable[row])
	{
		if(_data == NULL)
			return;
		
		//We may have to add the price
		uint price = 0;
		if(self.isTome)
			price = ((META_TOME*)_data)[row].price;
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

- (NSRect) updateFrameBeforeDrag : (NSRect) earlyFrame
{
	//FIXME: Update earlyFrame with a more precise metric
	uint column = _tableView.preCommitedLastClickedColumn;
	
	if(column != 0 && _nbElemPerCouple && _nbCoupleColumn)
		earlyFrame.origin.x += (column / _nbElemPerCouple) * (_tableView.bounds.size.width / _nbCoupleColumn);
	
	return earlyFrame;
}

@end
