/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

#define IDENTIFIER_PRICE @"RakCTSelectionListPrice"
#define DEFAULT_MAIN_WIDTH 95
#define DEFAULT_DETAIL_WIDTH 60

@implementation RakCTSelectionList

#pragma mark - Classical initialization

- (instancetype) initWithFrame : (NSRect) frame  isCompact : (BOOL) isCompact projectData : (PROJECT_DATA) project isTome : (BOOL) isTomeRequest selection : (long) elemSelected  scrollerPos : (long) scrollerPosition
{
	self = [self init];
	
	if(self != nil)
	{
		NSInteger row = LIST_INVALID_SELECTION, tmpRow = 0;
		
		_nbElemPerCouple = 1;
		_indexSelectedBeforeUpdate = LIST_INVALID_SELECTION;
		_compactMode = isCompact;
		self.isTome = isTomeRequest;
		chapterPrice = NULL;
		projectData = getEmptyProject();
		_selectionWithoutUI = NO;
		
		//We don't protect chapter/volume list but not really a problem as we'll only use it for drag'n drop
		[self reloadData:project :NO];
		
		//We check we have valid data
		if(elemSelected != LIST_INVALID_SELECTION)
		{
			for(; tmpRow < _nbData && ACCESS_CT(_isTome, _data, _data, tmpRow) < elemSelected; tmpRow++);
			
			if(tmpRow < _nbData && ACCESS_CT(_isTome, _data, _data, tmpRow) == elemSelected)
				row = tmpRow;
		}
		
		[self applyContext : frame : row :scrollerPosition];
		
		if(_tableView != nil && scrollView != nil)
		{
			NSSize size = scrollView.bounds.size;
			size.width /= 2;
			
			_mainColumns = @[[_tableView.tableColumns firstObject]];
			_nbCoupleColumn = 1;
			_numberOfRows = _nbData;
			[self updateMultiColumn: _compactMode : size];
			
			_tableView.wantVerboseClick = YES;
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

- (BOOL) reloadData : (PROJECT_DATA) project : (BOOL) resetScroller
{
	if(!project.isInitialized)
	{
		[self fullAnimatedReload : _nbData :0];
		_data = NULL;
		_nbData = 0;
		return YES;
	}
	
	void * newDataBuf = NULL, *newData, *newPrices = NULL, *installedData = NULL, *oldData = NULL, *oldInstalled = NULL;
	uint allocSize, nbElem, nbInstalledData, nbChapterPrice = 0, *installedJumpTable = NULL, nbOldElem, nbOldInstalled = 0;
	BOOL *installedTable = NULL, sameProject = projectData.cacheDBID == project.cacheDBID, orderProjects = NO, isTome = self.isTome;
	NSInteger element = _tableView != nil ? [self getSelectedElement] : 0;
	
	if(!sameProject)
		orderProjects = compareStrings(projectData.projectName, 0, project.projectName, 0, COMPARE_UTF32) == NSOrderedDescending;
	
	if(isTome)
	{
		allocSize = sizeof(META_TOME);
		
		nbElem = project.nbVolumes;
		newData = project.volumesFull;
		nbInstalledData = project.nbVolumesInstalled;
		installedData = project.volumesInstalled;
		
		if(newData == NULL || nbElem == 0)
			return NO;
	}
	else
	{
		allocSize = sizeof(uint);
		
		nbElem = project.nbChapter;
		newData = project.chaptersFull;
		installedData = project.chaptersInstalled;
		nbInstalledData = project.nbChapterInstalled;
		
		if(newData == NULL || nbElem == 0)
			return NO;
		
		//Price table
		if(project.isPaid && project.chaptersPrix != NULL)
		{
			newPrices = calloc(project.nbChapter, sizeof(uint));
			if(newPrices != NULL)
			{
				nbChapterPrice = project.nbChapter;
				memcpy(newPrices, project.chaptersPrix, nbChapterPrice * sizeof(uint));
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
		memcpy(newDataBuf, newData, nbElem * allocSize);
	
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
	_numberOfRows = _nbData / _nbCoupleColumn;
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
				
				if(isTome)
					freeTomeList(oldDataBak, nbOldElem, true);
				else
					free(oldDataBak);
				
				nbOldElem = nbOldInstalled;

				//New data
				newInstalledData = buildInstalledList(_data, _nbElem, _installedJumpTable, _nbInstalled, isTome);
				nbNewData = _nbInstalled;
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
			[self fullAnimatedReload :nbOldElem :newElem :orderProjects];
		}
		
		//Add the column
		[self updateMultiColumn: self.compactMode : scrollView.bounds.size];
		
		[scrollView updateScrollerState : scrollView.bounds];
		
		if(element != LIST_INVALID_SELECTION)
			[self selectElement : element];
	}
	
	if(isTome)
		freeTomeList(oldData, nbOldElem, true);
	else
		free(oldData);
	free(oldInstalled);
	free(installedTable);
	
	return YES;
}

- (void) flushContext : (BOOL) animated
{
	if(animated)
		[_tableView removeRowsAtIndexes:[NSMutableIndexSet indexSetWithIndexesInRange:NSMakeRange(0, (NSUInteger) _tableView.numberOfRows)] withAnimation:NSTableViewAnimationSlideLeft];
	
	if(self.isTome)
		freeTomeList(_data, _nbElem, true);
	else
	{
		free(_data);
		free(chapterPrice);	chapterPrice = NULL;
	}
	
	_data = NULL;
	_nbElem = _nbInstalled = 0;

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
	return projectData.isInitialized ? (self.isTome ? projectData.nbVolumes == 0 : projectData.nbChapter == 0) : YES;
}

- (uint) nbElem
{
	return _compactMode ? _nbInstalled : _nbElem;
}

#pragma mark - Backup routine

- (uint) getSelectedElement
{
	uint element = [self rowFromCoordinates:selectedRowIndex :selectedColumnIndex];
	
	if(element >= _nbElem)
		return LIST_INVALID_SELECTION;
	
	if(self.isTome)
		return (uint) ((META_TOME *) _data)[element].ID;
	else
		return (uint) ((int *) _data)[element];
}

- (void) jumpScrollerToIndex : (uint) index
{
	uint row = [self coordinateForIndex:index :NULL];
	
	if(_tableView != nil && row != LIST_INVALID_SELECTION)
	{
		uint numberOfVisibleRow = scrollView.bounds.size.height / _tableView.rowHeight;
		if(row > numberOfVisibleRow)
			row = MIN(_numberOfRows - 1, row + numberOfVisibleRow / 2);
		
		[_tableView scrollRowToVisible:row];
	}
}

- (uint) getIndexOfElement : (uint) element
{
	if(_data == NULL || (self.compactMode && _installedJumpTable == NULL))
		return LIST_INVALID_SELECTION;
	
	if(self.isTome)
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
				if(((uint*) _data)[_installedJumpTable[pos]] == element)
					return pos;
			}
		}
		else
		{
			for (uint pos = 0; pos < _nbElem; pos++)
			{
				if(((uint *) _data)[pos] == element)
					return pos;
			}
		}
	}
	
	return LIST_INVALID_SELECTION;
}

#pragma mark - Switch state

- (void) setCompactMode : (BOOL) compactMode
{
	if(compactMode != _compactMode)
	{
		_compactMode = compactMode;
		
		_nbData = [self nbElem];
		_numberOfRows = _nbData / _nbCoupleColumn;
		
		if(selectedRowIndex != LIST_INVALID_SELECTION && _installedJumpTable != NULL)
		{
			if(compactMode)	//We go from full to installed only
			{
				uint pos = 0, element = [self rowFromCoordinates:selectedRowIndex :selectedColumnIndex];
				
				if(element != UINT_MAX)
				{
					for(; pos < _nbInstalled && _installedJumpTable[pos] != element; pos++);
					
					if(pos < _nbInstalled)
						_indexSelectedBeforeUpdate = pos;
					else
						_indexSelectedBeforeUpdate = LIST_INVALID_SELECTION;
				}
				else
					_indexSelectedBeforeUpdate = LIST_INVALID_SELECTION;
				
				selectedColumnIndex = 1;
			}
			else
			{
				if(_installedJumpTable != NULL && selectedRowIndex < _nbInstalled)
				{
					uint index = _installedJumpTable[selectedRowIndex];
					_indexSelectedBeforeUpdate = [self coordinateForIndex:index :&selectedColumnIndex];
				}
				else
					_indexSelectedBeforeUpdate = LIST_INVALID_SELECTION;
			}
		}
		else
			_indexSelectedBeforeUpdate = LIST_INVALID_SELECTION;
		
		//Because of how things have to be handled when er get in vs out, the call order change
		if(compactMode)
		{
			uint nbColumn = _nbCoupleColumn;
			[self updateMultiColumn : compactMode : scrollView.bounds.size];
			[self triggerInstallOnlyAnimate : compactMode numberOfColumns: nbColumn];
			[self needUpdateTableviewHeight];
		}
		else
		{
			[self triggerInstallOnlyAnimate : compactMode numberOfColumns: _nbCoupleColumn];
			[self updateMultiColumn : compactMode : scrollView.bounds.size];
		}
	}
}

#pragma mark - Model manipulation

- (uint) rowFromCoordinates : (uint) row : (uint) column
{
	if(_nbCoupleColumn > 1 && _nbElemPerCouple != 0 && row != LIST_INVALID_SELECTION && column != LIST_INVALID_SELECTION)
	{
		uint modulo = _nbData % _nbCoupleColumn;

		column /= _nbElemPerCouple;

		if(modulo != 0 && column > modulo)
		{
			row += modulo * (_numberOfRows + 1);
			row += (column - modulo) * _numberOfRows;
		}
		else if(modulo != 0)
		{
			//We have several columns with different height, we have to check we're not exceeding one's height
			if(column == modulo && row == _numberOfRows)
				return UINT_MAX;

			row += column * (_numberOfRows + 1);
		}
		else
			row += column * _numberOfRows;
	}
	
	return row;
}

- (uint) coordinateForIndex : (uint) index : (uint *) column
{
	if(_nbCoupleColumn > 1 && index != LIST_INVALID_SELECTION)
	{
		uint modulo = _nbData % _nbCoupleColumn, maxNumberOfRows = _numberOfRows + (modulo != 0), idealColumn = index / maxNumberOfRows;
		
		if(modulo == 0 || idealColumn <= modulo)
		{
			if(column != NULL)
				*column = idealColumn;
			return index % maxNumberOfRows;
		}
		else
		{
			uint curColumn = modulo;
			index -= curColumn * maxNumberOfRows;
			while(index >= maxNumberOfRows)
			{
				curColumn++;
				index -= _numberOfRows;
			}
			
			if(column != NULL)
				*column = curColumn;
			
			return index;
		}
	}
	
	return index;
}

#pragma mark - Methods to deal with tableView

- (void) updateMultiColumn :(NSSize)scrollviewSize
{
	[self updateMultiColumn : self.compactMode : scrollviewSize];
}

- (void) updateMultiColumn : (BOOL) isCompact : (NSSize) scrollviewSize
{
	NSSize initialSize = _tableView.bounds.size;
	
	if(_indexSelectedBeforeUpdate == LIST_INVALID_SELECTION)
		_indexSelectedBeforeUpdate = [self rowFromCoordinates:selectedRowIndex :selectedColumnIndex];

	if(!isCompact || _nbCoupleColumn > 1)
	{
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
			_numberOfRows = _nbData;
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
				_detailWidth = DEFAULT_DETAIL_WIDTH;
				
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
			CGFloat columnWidth = DEFAULT_MAIN_WIDTH, scrollerWidth = 0, maxHeight = [self nbElem] * _tableView.rowHeight;
			uint nbColumn = 1, oldNbColumn = _nbCoupleColumn;
			int newColumns;
			
			//We add space for detail tab
			if(paidContent)
				columnWidth += DEFAULT_DETAIL_WIDTH;
			else
				scrollerWidth = [RakScroller width];
			
			//Define number of columns
#ifdef CT_LIST_PREFER_LESS_COLUMN
			CGFloat minTabHeight = _tableView.numberOfRows * _tableView.rowHeight;
#else
			CGFloat minTabHeight = scrollviewSize.height / 2;
#endif
			
			while(floor(maxHeight / nbColumn) > minTabHeight && columnWidth * nbColumn + scrollerWidth <= scrollviewSize.width)
				nbColumn++;
			
			//If we can't fit in the width we got
			if(nbColumn > 1 && columnWidth * nbColumn + scrollerWidth > scrollviewSize.width)
				nbColumn--;
			
			//Now, apply changes
			_nbCoupleColumn = nbColumn;
			_numberOfRows = _nbData / nbColumn;
			
			newColumns = (((int64_t) nbColumn) - oldNbColumn);
			
			if(newColumns > 0)		//We need to add columns
			{
				NSMutableArray *newMainColumns = [NSMutableArray arrayWithArray:_mainColumns], *newDetailColumns = [NSMutableArray arrayWithArray:_detailColumns];
				NSTableColumn * column;
				
				for(int pos = 0; pos < newColumns; pos++)
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
				[_tableView reloadDataForRowIndexes:[NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, (NSUInteger) [_tableView numberOfRows])] columnIndexes:[NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, (NSUInteger) [_tableView numberOfColumns])]];
				[self updateRowNumber];
			}
			[_tableView endUpdates];
		}
	}
	
	if(!_selectionWithoutUI)
	{
		dispatch_async(dispatch_get_main_queue(), ^{
			[self performSelectorOnMainThread:@selector(postProcessColumnUpdate) withObject:nil waitUntilDone:NO];
		});
	}
	
	initialSize.height = MIN(initialSize.height, [_tableView numberOfRows] * [_tableView rowHeight]);
	
	//Adding or removing columns will impact tableview size
	if(!NSEqualSizes(initialSize, _tableView.bounds.size))
		[_tableView setFrameSize:initialSize];
	
	[self additionalResizing : initialSize : NO];
}

- (void) postProcessColumnUpdate
{
	if(_indexSelectedBeforeUpdate != LIST_INVALID_SELECTION)
	{
		_UIOnlySelection = YES;
		[self selectIndex:_indexSelectedBeforeUpdate];
		_UIOnlySelection = NO;
		[self jumpScrollerToIndex:_indexSelectedBeforeUpdate];
		
		_indexSelectedBeforeUpdate = LIST_INVALID_SELECTION;
	}
}

- (void) updateRowNumber
{
	if(!_rowNumberUpdateQueued)
	{
		_rowNumberUpdateQueued = YES;
		dispatch_async(dispatch_get_main_queue(), ^{
			[self _updateRowNumber];
		});
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
	[self additionalResizing : _tableView.bounds.size : NO];
	[self reloadSize];
	_resizingQueued = NO;
}

- (void) additionalResizing : (NSSize) newSize : (BOOL) animated
{
	CGFloat width = newSize.width / _nbCoupleColumn, detailWidth = self.compactMode ? 0 : _detailWidth, mainWidth = width - detailWidth;
	
	[_mainColumns enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
		((NSTableColumn*) obj).width = mainWidth;
	}];
	
	if(!self.compactMode && _detailColumns != nil) //We update every view size
	{
		[_detailColumns enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
			((NSTableColumn *)obj).width = _detailWidth;
		}];
	}
}

- (RakView*) tableView : (NSTableView *) tableView viewForTableColumn : (NSTableColumn*) tableColumn row : (NSInteger) row
{
	RakText * output = (RakText *) [super tableView:tableView viewForTableColumn:tableColumn row:row];
	
	if(_detailColumns != nil && [_detailColumns containsObject : tableColumn])
	{
		output.alignment = NSTextAlignmentRight;
		
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
		output.alignment = NSTextAlignmentLeft;
	
	return output;
}

#ifdef EXTENSIVE_LOGGING
#define EMPTY_MESSAGE @"Error :("
#else
#define EMPTY_MESSAGE @""
#endif

- (NSString*) tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)rowIndex
{
	NSString * output;
	NSUInteger column = 0;
	BOOL isDetails = NO;
	
	column = [_mainColumns indexOfObject:tableColumn];
	if(column == NSNotFound)
	{
		if(_detailColumns == nil || (column = [_detailColumns indexOfObject:tableColumn]) == NSNotFound)
			return EMPTY_MESSAGE;

		isDetails = YES;
	}
	
	rowIndex = [self rowFromCoordinates : rowIndex : column * _nbElemPerCouple];
	
	if(rowIndex >= _nbData)	//Too much entry?
	{
		if(rowIndex != UINT_MAX && rowIndex / _nbCoupleColumn > _nbData / _nbCoupleColumn)	//Inconsistency
		{
			[self performSelectorOnMainThread:@selector(updateRowNumber) withObject:nil waitUntilDone:NO];
			return EMPTY_MESSAGE;
		}

		//We're just on the last incomplete row
		return @"";
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
		if(element.ID != INVALID_VALUE)
		{
			if(_detailColumns == nil || !isDetails)
				output = getStringForVolumeFull(element);

			else if(_installedTable == NULL || !_installedTable[rowIndex])
				output = getStringForPrice(element.price);

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
			uint ID = ((uint *) _data)[rowIndex];

			if(ID != INVALID_VALUE)
				output = getStringForChapter(ID);
			else
				output = @"Error! Out of bounds D:";
		}
		else if(chapterPrice != NULL && rowIndex < _nbChapterPrice && (_installedTable == NULL || !_installedTable[rowIndex]))
		{
			output = [NSString stringWithFormat:@"%@%c", getStringForPrice(chapterPrice[rowIndex]) , column == _nbCoupleColumn-1 ? '\0' : '	'];
		}
		else
			output = @"";
	}
	
	return output;
}

- (RakColor *) getTextColor
{
	return nil;
}

- (RakColor *) getTextHighlightColor
{
	return nil;
}

- (RakColor*) getTextColor:(uint)column :(uint)row
{
	row = [self rowFromCoordinates : row : column];
	
	if(row < _nbData)
	{
		if(self.compactMode || (_installedTable != NULL && _installedTable[row]))
			return [Prefs getSystemColor : COLOR_CLICKABLE_TEXT];
	}
	
	return [Prefs getSystemColor : COLOR_SURVOL];
}

- (RakColor *) getTextHighlightColor:(uint)column :(uint)row
{
	return [Prefs getSystemColor : COLOR_ACTIVE];
}

#pragma mark - Smart reloading

- (SR_DATA *) getSmartReloadData : (void*) data : (BOOL) isTome : (uint) nbElem : (BOOL *) installed
{
	if(data == NULL || !nbElem)
		return NULL;
	
	SR_DATA * output = calloc(nbElem, sizeof(SR_DATA));
	
	if(output != NULL)
	{
		for(uint i = 0; i < nbElem; i++)
		{
			output[i].data = (uint) (isTome ? ((META_TOME *) data)[i].ID : ((uint *) data)[i]);
			output[i].installed = installed == NULL ? NO : installed[i];
		}
	}
	
	return output;
}

- (void) triggerInstallOnlyAnimate : (BOOL) enter numberOfColumns : (uint) nbColumns
{
	BOOL foundOneIn = NO, foundOneOut = NO;
	uint size = MIN(_nbElem, [_tableView numberOfRows]);
	NSMutableIndexSet * indexIn = [NSMutableIndexSet new], * indexOut = [NSMutableIndexSet new];
	
	if(_installedTable != NULL)
	{
		if(enter)
		{
			for(uint i = 0, rank = 0; i < _nbElem; i++)
			{
				if(_installedTable[i])
				{
					//If the element was on a column > 1, when have to add it to the main column as it's going to be the only one remaining
					if(nbColumns > 1 && i >= size)
					{
						[indexIn addIndex:rank];
						foundOneIn = YES;
					}
					
					rank++;
				}
				else if(i < size)	//Always true when a single columns, overwise skip the columns afterward
				{
					[indexOut addIndex : i];
					foundOneOut = YES;
				}
			}
		}
		else
		{
			for(uint i = 0; i < _nbElem; i++)
			{
				if(!_installedTable[i])
				{
					[indexIn addIndex:i];
					foundOneIn = YES;
				}
			}
		}
	}
	else if(enter)
		[self fullAnimatedReload:_nbElem :0];
	else
		[self fullAnimatedReload:_nbInstalled :_nbElem];
	
	if(foundOneIn || foundOneOut)
	{
		[NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
			
			[context setDuration:CT_TRANSITION_ANIMATION];
			
			if(foundOneOut)
				[_tableView removeRowsAtIndexes:indexOut withAnimation:NSTableViewAnimationSlideLeft];
			if(foundOneIn)
				[_tableView insertRowsAtIndexes:indexIn withAnimation:NSTableViewAnimationSlideLeft];
			
		} completionHandler:^{
			
			NSSize tableSize = _tableView.bounds.size;
			tableSize.height = MIN(tableSize.height, [_tableView numberOfRows] * [_tableView rowHeight]);
			
			//Adding or removing columns will impact tableview size
			if(!NSEqualSizes(tableSize, _tableView.bounds.size))
				[_tableView setFrameSize:tableSize];

			[scrollView updateScrollerState:scrollView.bounds];
		}];
	}
}

#pragma mark - Get result from NSTableView

- (BOOL) tableView : (RakTableView *) tableView shouldSelectRow:(NSInteger)rowIndex
{
	NSInteger index = [self rowFromCoordinates : rowIndex : tableView.preCommitedLastClickedColumn];
	
	//If not installed, we don't want to reflect the UI
	//We have one bypass for init, we don't block signal when in compactMode, then sanity checks and check if installed
	if(!_UIOnlySelection && !self.compactMode && index >= 0 && index < _nbElem && _installedTable != NULL && !_installedTable[index])
	{
		[self processElement:[self rowFromCoordinates : rowIndex : tableView.preCommitedLastClickedColumn]];
		return NO;
	}
	else
	{
		self._selectionChangeComeFromClic = YES;
		return [super tableView:tableView shouldSelectRow:rowIndex];
	}
}

- (void) tableViewSelectionDidChange : (NSNotification *) notification;
{
	if(!self._selectionChangeComeFromClic)
		return;
	else
		self._selectionChangeComeFromClic = NO;
	
	[self processElement:[self rowFromCoordinates : selectedRowIndex : selectedColumnIndex]];
}

- (void) processElement : (uint) index
{
	if(index != LIST_INVALID_SELECTION && index < [self nbElem])
	{
		BOOL installed = self.compactMode || (_installedTable != NULL && _installedTable[index]);
		
		[[NSNotificationCenter defaultCenter] postNotificationName: CT_CLIC_NOTIFICATION object:nil userInfo: @{@"index": @(index), @"isTome" : @(self.isTome), @"isInstalled" : @(installed)}];
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
	uint column = _tableView.preCommitedLastClickedColumn;
	
	if(column != UINT_MAX && column % _nbElemPerCouple)
		column -= column % _nbElemPerCouple;
	
	return [self tableView:_tableView objectValueForTableColumn:[_tableView.tableColumns objectAtIndex:column] row:row];
}

- (void) fillDragItemWithData:(RakDragItem *)item :(uint)row
{
	uint selection;
	row = [self rowFromCoordinates : row : _tableView.preCommitedLastClickedColumn];
	
	if(self.isTome)
	{
		selection = (((META_TOME *) _data)[row]).ID;
		item.price = (((META_TOME *) _data)[row]).price;
	}
	else
	{
		selection = ((uint *) _data)[row];
		
		if(chapterPrice != NULL && row < _nbChapterPrice)
			item.price = chapterPrice[row];
	}
	
	[item setDataProject:getCopyOfProjectData(projectData) fullProject:NO isTome:self.isTome element:selection];
}

- (BOOL) shouldPromiseFile : (RakDragItem *) item
{
	return !item.canDL;
}

- (void) additionalDrawing : (RakDragView *) _draggedView : (uint) row
{
	row = [self rowFromCoordinates : row : _tableView.preCommitedLastClickedColumn];
	
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
			priceView.textColor = [Prefs getSystemColor:COLOR_CLICKABLE_TEXT];
			priceView.stringValue = getStringForPrice(price);
			[priceView sizeToFit];
			
			[_draggedView addPrice:priceView];
		}
	}
}

- (NSRect) updateFrameBeforeDrag : (NSRect) earlyFrame
{
	//FIXME: Update earlyFrame with a more precise metric
	uint column = _tableView.preCommitedLastClickedColumn / _nbElemPerCouple;
	
	if(column != 0 && _nbCoupleColumn != 0)
		earlyFrame.origin.x += column * (_tableView.bounds.size.width / _nbCoupleColumn);
	
	return earlyFrame;
}

@end
