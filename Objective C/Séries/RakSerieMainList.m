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

- (id) init : (NSRect) frame : (NSInteger) selectedDBID : (NSInteger) scrollPosition : (BOOL) installOnly
{
	self = [super init];
	
	if(self != nil)
	{
		_jumpToInstalled = NULL;
		
		[RakDBUpdate registerForUpdate:self :@selector(DBUpdated:)];
		data = getCopyCache(SORT_NAME | RDB_LOADALL, &_nbElemFull);
		_installed = getInstalledFromData(data, _nbElemFull);
		
		if(installOnly)
		{
			[self updateJumpTable];
			amountData = _nbElemInstalled;
		}
		else
			amountData = _nbElemFull;
		
		if(selectedDBID != -1)
		{
			for(uint i = 0, positionInInstalled = 0; i < amountData; i++)
			{
				if(!installOnly || _installed[i])
				{
					if(((PROJECT_DATA*)data)[i].cacheDBID == selectedDBID)
					{
						selectedIndex = positionInInstalled;
						break;
					}
					
					positionInInstalled++;
				}
			}
		}
		
		if(data == NULL || (self.installOnlyMode && _installed == NULL))
		{
			NSLog(@"Failed at initialize RakSerieMainList, most probably a memory problem :(");

			freeProjectData(data); //Seul _cache peut ne pas être null dans cette branche
		}
		
		[self applyContext:frame : selectedIndex : scrollPosition];
	}
	return self;
}

- (bool) didInitWentWell
{
	return data != NULL && (!self.installOnlyMode || _installed != NULL);
}

- (BOOL) installOnlyMode
{
	return _jumpToInstalled != NULL;
}

- (void) setInstallOnly : (BOOL) installedOnly
{
	if((installedOnly && _jumpToInstalled != NULL) || (!installedOnly && _jumpToInstalled == NULL))
		return;
	
	NSMutableIndexSet * index = [[NSMutableIndexSet alloc] init];
	
	if(installedOnly)
	{
		[self updateJumpTable];
		amountData = _nbElemInstalled;
	}
	else
	{
		void * tmp = _jumpToInstalled;

		_jumpToInstalled = NULL;
		_nbElemInstalled = 0;
		
		free(tmp);
		amountData = _nbElemFull;
	}

	//Gather rows that will have to be removed/inserted
	for(uint pos = 0; pos < _nbElemFull; pos++)
	{
		if(!_installed[pos])
			[index addIndex:pos];
	}
	
	if(installedOnly)
		[_tableView removeRowsAtIndexes:index withAnimation:NSTableViewAnimationSlideLeft];
	else
		[_tableView insertRowsAtIndexes:index withAnimation:NSTableViewAnimationSlideLeft];
	
	[scrollView updateScrollerState : scrollView.bounds];
}

- (NSInteger) selectedRow
{
	return selectedIndex;
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
			if(((PROJECT_DATA*)data)[pos].cacheDBID == updatedID)
			{
				PROJECT_DATA newElem = getElementByID(updatedID), *current = &((PROJECT_DATA*)data)[pos];
				
				if(!newElem.isInitialized)
					return;
				
				releaseCTData(*current);
				*current = newElem;
				
				BOOL newIsInstalled = isProjectInstalledInCache(updatedID);
				if(_installed[pos] == newIsInstalled)
					break;
				
				_installed[pos] = newIsInstalled;
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
		uint nbElem;
		PROJECT_DATA * projects = getCopyCache(SORT_NAME | RDB_LOADALL, &nbElem);
		bool * newInstalled;
		if(projects == NULL)
			return;
		
		newInstalled = getInstalledFromData(projects, nbElem);
		if(newInstalled == NULL)
		{
			freeProjectData(projects);
			return;
		}
		
		freeProjectData(data);
		data = projects;
		free(_installed);
		_installed = newInstalled;
		
		_nbElemFull = nbElem;
		
		NSInteger element = [self selectedRow];
		if(self.installOnlyMode)
			[self updateJumpTable];
		
#warning "what about an efficient system?"
		[_tableView reloadData];
		
		if(element != -1)
			[self selectRow:element];
	}
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
	
	if (positionRawTable == 0)	//nothing installed
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
	PROJECT_DATA output;
	
	if(index >= 0 && index < amountData)
	{
		if(self.installOnlyMode && index < _nbElemInstalled)
			index = _jumpToInstalled[index];
		
		output = getCopyOfProjectData(((PROJECT_DATA*) data)[index]);
	}
	else
		output.isInitialized = false;
	
	return output;
}

- (NSInteger) getSelectedElement
{
	if(_tableView == nil)
		return -1;
	
	if(selectedIndex == -1)
		return -1;
	
	PROJECT_DATA project = [self getElementAtIndex:selectedIndex];

	if(project.team == NULL)
		return -1;
	
	return project.cacheDBID;
}

- (NSInteger) getIndexOfElement : (NSInteger) element
{
	if(_jumpToInstalled == NULL)
		return -1;
	
	for (uint pos = 0; pos < _nbElemInstalled; pos++)
	{
		if(((PROJECT_DATA*) data)[_jumpToInstalled[pos]].cacheDBID == element)
			return pos;
	}
	
	return -1;
}

#pragma mark - Methods to deal with tableView

- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView
{
	return data == NULL ? 0 : amountData;
}

- (NSString*) tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
	if(rowIndex < amountData)
	{
		if (_jumpToInstalled != NULL && rowIndex < _nbElemInstalled)
			rowIndex = _jumpToInstalled[rowIndex];
		
		return [[NSString alloc] initWithData:[NSData dataWithBytes:((PROJECT_DATA*) data)[rowIndex].projectName length:sizeof(((PROJECT_DATA*) data)[rowIndex].projectName)] encoding:NSUTF32LittleEndianStringEncoding];
	}
	else
		return @"Error D:";
}

- (void) tableView:(NSTableView *)tableView didAddRowView:(NSTableRowView *)rowView forRow:(NSInteger)row
{
	RakText * element = [tableView viewAtColumn:0 row:row makeIfNecessary:NO];
	
	if (row == selectedIndex)
	{
		[element setTextColor : highlight != nil ? highlight : [self getTextHighlightColor:0 :row]];
		[element setDrawsBackground:YES];
		return;
	}
}

#pragma mark - Get result from NSTableView

- (void)tableViewSelectionDidChange:(NSNotification *)notification;
{
	PROJECT_DATA dataToSend = [self getElementAtIndex : selectedIndex];
	
	if(dataToSend.team != NULL)
		[RakTabView broadcastUpdateContext: scrollView : dataToSend : NO : VALEUR_FIN_STRUCT];
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
	PROJECT_DATA project = getCopyOfProjectData([self getElementAtIndex:row]);
	
	BOOL isTome = [[item class] defineIsTomePriority:&project alreadyRefreshed:NO];
	
	getUpdatedCTList(&project, true);
	getUpdatedCTList(&project, false);
	
	[item setDataProject:project isTome:isTome element:VALEUR_FIN_STRUCT];
}

@end
