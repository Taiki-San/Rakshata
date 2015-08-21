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

@implementation RakSerieMainList

- (instancetype) init : (NSRect) frame : (NSInteger) selectedDBID : (NSInteger) scrollPosition : (BOOL) installOnly
{
	self = [super init];
	
	if(self != nil)
	{
		_jumpToInstalled = NULL;
		
		[RakDBUpdate registerForUpdate:self :@selector(DBUpdated:)];
		_data = getCopyCache(SORT_NAME | RDB_LOADALL, &_nbElemFull);
		_installed = getInstalledFromData(_data, _nbElemFull);
		installedRequested = installOnly;
		
		if(installOnly)
		{
			[self updateJumpTable];
			
			if(_nbElemInstalled != 0)
				_nbData = _nbElemInstalled;
			else
				installOnly = NO;
		}
		
		if(!installOnly)
			_nbData = _nbElemFull;
		
		if(selectedDBID != LIST_INVALID_SELECTION)
		{
			for(uint i = 0, positionInInstalled = 0; i < _nbData; i++)
			{
				if(!installOnly || _installed[i])
				{
					if(((PROJECT_DATA*)_data)[i].cacheDBID == selectedDBID)
					{
						selectedRowIndex = positionInInstalled;
						break;
					}
					
					positionInInstalled++;
				}
			}
		}
		
		if(_data == NULL || (self.installOnlyMode && _installed == NULL))
		{
			NSLog(@"Failed at initialize RakSerieMainList, most probably a memory problem :(");
			
			freeProjectData(_data); //Seul _cache peut ne pas être null dans cette branche
		}
		
		[self applyContext:frame : selectedRowIndex : scrollPosition];
	}
	return self;
}

- (void) dealloc
{
	[RakDBUpdate unRegister : self];
}

- (BOOL) installOnlyMode
{
	return _jumpToInstalled != NULL;
}

- (void) setInstallOnly : (BOOL) installedOnly
{
	if((installedOnly && _jumpToInstalled != NULL) || (!installedOnly && _jumpToInstalled == NULL))
		return;
	else
		installedRequested = installedOnly;
	
	NSMutableIndexSet * index = [[NSMutableIndexSet alloc] init];
	
	if(installedOnly)
		[self updateJumpTable];
	
	if(installedOnly && _nbElemInstalled)
	{
		_nbData = _nbElemInstalled;
	}
	else
	{
		installedOnly = NO;
		
		void * tmp = _jumpToInstalled;
		
		_jumpToInstalled = NULL;
		_nbElemInstalled = 0;
		
		free(tmp);
		_nbData = _nbElemFull;
	}
	
	//Gather rows that will have to be removed/inserted
	uint newSelectedIndex = LIST_INVALID_SELECTION;
	
	for(uint pos = 0, counter = 0; pos < _nbElemFull; pos++)
	{
		if(!_installed[pos])				//Not installed
		{
			[index addIndex:pos];
		}
		else if(installedOnly)				//Installed, and we look to our rank in installed
		{
			if(pos == selectedRowIndex)
				newSelectedIndex = counter;
			else
				counter++;
		}
		else								//Installed, and we're looking at our position in the main list
		{
			if(counter++ == selectedRowIndex)
				newSelectedIndex = pos;
		}
	}
	
	if(installedOnly)
		[_tableView removeRowsAtIndexes:index withAnimation:NSTableViewAnimationSlideLeft];
	else
		[_tableView insertRowsAtIndexes:index withAnimation:NSTableViewAnimationSlideLeft];
	
	[self needUpdateTableviewHeight];
	[scrollView updateScrollerState : scrollView.bounds];
	
	if(newSelectedIndex != LIST_INVALID_SELECTION)	//Previous selection is in the new list
		[self selectIndex:newSelectedIndex];
}

#pragma mark - Data manipulation

- (void) DBUpdated : (NSNotification *) notification
{
	if(![NSThread isMainThread])
	{
		[self performSelectorOnMainThread:@selector(DBUpdated:) withObject:notification waitUntilDone:NO];
		return;
	}
	
	uint updatedID;
	if(![RakDBUpdate isPluralUpdate:notification.userInfo] && [RakDBUpdate getIDUpdated:notification.userInfo :&updatedID])			//Single item updated
	{
		//The only data that could change if only a single project changed is the installation state
		for(uint pos = 0; pos < _nbElemFull; pos++)
		{
			if(((PROJECT_DATA*)_data)[pos].cacheDBID == updatedID)
			{
				PROJECT_DATA newElem = getProjectByID(updatedID), *current = &((PROJECT_DATA*)_data)[pos];
				
				if(!newElem.isInitialized)
					return;
				
				releaseCTData(*current);
				*current = newElem;
				
				BOOL newIsInstalled = isProjectInstalledInCache(updatedID);
				if(_installed[pos] == newIsInstalled)
					break;
				
				_installed[pos] = newIsInstalled;

				uint mainThread;
				[Prefs getPref:PREFS_GET_MAIN_THREAD :&mainThread];
				if(self.installOnlyMode || mainThread == TAB_READER)
					[self updateJumpTable];
				
				if(!self.installOnlyMode)
					break;
				
				//We get our position in the tableview to update
				uint posOfElemInInstalled;
				for(posOfElemInInstalled = 0; posOfElemInInstalled < _nbElemInstalled && _jumpToInstalled[posOfElemInInstalled] != pos; posOfElemInInstalled++);
				
				//We remove then our row to animate the update
				if(!_installed[pos])
					[_tableView removeRowsAtIndexes:[NSIndexSet indexSetWithIndex:posOfElemInInstalled] withAnimation:NSTableViewAnimationSlideLeft];
				else
					[_tableView insertRowsAtIndexes:[NSIndexSet indexSetWithIndex:posOfElemInInstalled] withAnimation:NSTableViewAnimationSlideLeft];
				
				break;
			}
		}
	}
	else
	{
		uint nbElem, oldNbElem = _nbElemFull;
		PROJECT_DATA * projects = getCopyCache(SORT_NAME | RDB_LOADALL, &nbElem), *oldData = _data;
		bool * newInstalled, *oldInstalled = _installed, needSwapInstalled = NO, newState;
		
		if(projects == NULL)
			return;
		
		newInstalled = getInstalledFromData(projects, nbElem);
		if(newInstalled == NULL)
		{
			freeProjectData(projects);
			return;
		}
		
		//We need to inverse variables, to let the table view access the new data
		
		_data = projects;
		_installed = newInstalled;
		_nbElemFull = nbElem;
		
		uint element = [self getSelectedElement];
		if(installedRequested)
		{
			BOOL wasInstalled = self.installOnlyMode;
			
			[self updateJumpTable];
			if((!wasInstalled && _nbElemInstalled) || (wasInstalled && _nbElemInstalled == 0))
			{
				needSwapInstalled = YES;
				newState = !wasInstalled;
			}
		}
		
		[self smartReload : [self getSmartReloadData : oldData : oldNbElem : oldInstalled] : oldNbElem : [self getSmartReloadData : _data : _nbElemFull : _installed]: _nbElemFull];
		
		freeProjectData(oldData);
		free(oldInstalled);
		
		if(element != LIST_INVALID_SELECTION)
			[self selectElement : element];
		
		if(needSwapInstalled)
			self.installOnlyMode = newState;
	}
}

- (SR_DATA *) getSmartReloadData : (PROJECT_DATA*) data : (uint) nbElem : (bool *) installed
{
	if(!nbElem)
		return NULL;
	
	SR_DATA * output = calloc(nbElem, sizeof(SR_DATA));
	
	if(output != NULL)
	{
		for(uint i = 0; i < nbElem; i++)
		{
			output[i].data = data[i].cacheDBID;
			output[i].installed = installed == NULL ? NO : installed[i];
		}
	}
	
	return output;
}

- (void) updateJumpTable
{
	if(_nbElemFull == 0)
		return;
	
	if(_jumpToInstalled != NULL)
		free(_jumpToInstalled);
	
	_jumpToInstalled = NULL;
	
	uint rawJumpTable[_nbElemFull], positionRawTable, positionInstalled;
	
	for (positionInstalled = positionRawTable = 0; positionInstalled < _nbElemFull; positionInstalled++)
	{
		if(_installed[positionInstalled])
			rawJumpTable[positionRawTable++] = positionInstalled;
	}
	
	if(positionRawTable == 0)	//nothing installed
	{
		_jumpToInstalled = NULL;
		_nbElemInstalled = 0;
		return;
	}
	
	uint * newJumpTable = malloc(positionRawTable * sizeof(uint));
	
	if(newJumpTable != NULL)
	{
		memcpy(newJumpTable, &rawJumpTable, positionRawTable * sizeof(uint));
		_jumpToInstalled = newJumpTable;
		_nbElemInstalled = positionRawTable;
	}
}

- (PROJECT_DATA) getElementAtIndex : (NSInteger) index
{
	if(index >= 0 && index < _nbData)
	{
		if(self.installOnlyMode && index < _nbElemInstalled)
			index = _jumpToInstalled[index];
		
		return getCopyOfProjectData(((PROJECT_DATA*) _data)[index]);
	}
	
	return getEmptyProject();
}

- (uint) getSelectedElement
{
	if(_tableView == nil)
		return LIST_INVALID_SELECTION;
	
	if(selectedRowIndex == LIST_INVALID_SELECTION)
		return LIST_INVALID_SELECTION;
	
	PROJECT_DATA project = [self getElementAtIndex:selectedRowIndex];
	
	if(!project.isInitialized)
		return LIST_INVALID_SELECTION;
	
	return project.cacheDBID;
}

- (uint) getIndexOfElement : (uint) element
{
	if(_jumpToInstalled == NULL)
		return LIST_INVALID_SELECTION;
	
	for (uint pos = 0; pos < _nbElemInstalled; pos++)
	{
		if(((PROJECT_DATA*) _data)[_jumpToInstalled[pos]].cacheDBID == element)
			return pos;
	}
	
	return LIST_INVALID_SELECTION;
}

#pragma mark - Methods to deal with tableView

- (void) resetHeight
{
	NSRange range = NSMakeRange(0, (NSUInteger) [_tableView numberOfRows]);
	
	[_tableView noteHeightOfRowsWithIndexesChanged:[NSIndexSet indexSetWithIndexesInRange:range]];
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView
{
	return _data == NULL ? 0 : _nbData;
}

- (CGFloat)tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row
{
	CGFloat width = _tableView.bounds.size.width;
	
	if(row >= _nbData)
		return 17;
	
	else if(preloadedRow == nil)
		preloadedRow = (RakText *) [self tableView:tableView viewForTableColumn:[[tableView tableColumns] firstObject] row:row];
	
	else if(preloadedRow.fixedWidth != width && width > 0)
		preloadedRow.fixedWidth = width;
	
	preloadedRow.stringValue = [self tableView:tableView objectValueForTableColumn:nil row:row];
	
	return preloadedRow.bounds.size.height;
}

- (NSView *) tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
	if(row >= _nbData)
		return nil;
	
	NSView * output = [super tableView:tableView viewForTableColumn:tableColumn row:row];
	
	if(output != nil && [output isKindOfClass:[RakText class]])
	{
		[((RakText *) output).cell setWraps:YES];
		((RakText *) output).discardHeight = YES;
	}
	
	return output;
}

- (NSString*) tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
	if(rowIndex < _nbData)
	{
		if(_jumpToInstalled != NULL && rowIndex < _nbElemInstalled)
			rowIndex = _jumpToInstalled[rowIndex];
		
		return getStringForWchar(((PROJECT_DATA*) _data)[rowIndex].projectName);
	}

	return @"Error D:";
}

- (void) tableView:(NSTableView *)tableView didAddRowView:(NSTableRowView *)rowView forRow:(NSInteger)row
{
	RakText * element = [tableView viewAtColumn:0 row:row makeIfNecessary:NO];
	
	if(row == selectedRowIndex)
	{
		[element setTextColor : highlight != nil ? highlight : [self getTextHighlightColor:0 :row]];
		[element setDrawsBackground:YES];
		return;
	}
}

#pragma mark - Get result from NSTableView

- (BOOL) tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)row
{
	return self._selectionChangeComeFromClic = [super tableView:tableView shouldSelectRow:row];
}

- (void)tableViewSelectionDidChange:(NSNotification *)notification;
{
	if(!self._selectionChangeComeFromClic)
		return;
	else
		self._selectionChangeComeFromClic = NO;
	
	PROJECT_DATA dataToSend = [self getElementAtIndex : selectedRowIndex];
	
	if(dataToSend.isInitialized)
		[RakTabView broadcastUpdateContext: scrollView : dataToSend : NO : INVALID_VALUE];
}

#pragma mark - Drag and drop support

- (uint) getSelfCode
{
	return TAB_SERIES;
}

- (PROJECT_DATA) getProjectDataForDrag : (uint) row
{
	return [self getElementAtIndex:row];
}

- (NSString *) contentNameForDrag : (uint) row
{
	PROJECT_DATA project = [self getElementAtIndex:row];
	
	if(project.isInitialized)
		return [RakSerieList contentNameForDrag : project];
	else
		return [super contentNameForDrag:row];
}

- (void) fillDragItemWithData:(RakDragItem *)item :(uint)row
{
	PROJECT_DATA project = [self getElementAtIndex:	row / _nbCoupleColumn + _tableView.preCommitedLastClickedColumn / _nbElemPerCouple];
	
	if(!project.isInitialized)
		return;
	
	project = getCopyOfProjectData(project);
	
	BOOL isTome = [[item class] defineIsTomePriority:&project alreadyRefreshed:NO];
	
	getUpdatedCTList(&project, true);
	getUpdatedCTList(&project, NO);
	
	[item setDataProject:project fullProject:YES isTome:isTome element:INVALID_VALUE];
}

- (BOOL) shouldPromiseFile : (RakDragItem *) item
{
	return isInstalled(item.project, NULL);
}

@end
