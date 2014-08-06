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

- (id) init : (NSRect) frame : (NSInteger) selectedDBID : (NSInteger) scrollPosition
{
	self = [super init];
	
	if(self != nil)
	{
		_jumpToInstalled = NULL;
		
		data = getCopyCache(RDB_CTXSERIES | SORT_NAME | RDB_LOADALL, &amountData);
		_installed = getInstalledFromData(data, amountData);
		[self updateJumpTable];
		
		for(uint i = 0, positionInInstalled = 0; i < amountData; i++)
		{
			if(selectedDBID != -1 && selectedIndex == -1 && _installed[i])
			{
				if (((PROJECT_DATA*)data)[i].cacheDBID == selectedDBID)
					selectedIndex = positionInInstalled;
				else
					positionInInstalled++;
			}
		}
		
		if(data == NULL || _installed == NULL)
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
	return data != NULL && _installed != NULL;
}

- (BOOL) installOnlyMode
{
	return _jumpToInstalled != NULL;
}

- (void) setInstallOnly : (BOOL) installedOnly
{
	if((installedOnly && _jumpToInstalled != NULL) || (!installedOnly && _jumpToInstalled == NULL))
		return;
	
	if(installedOnly)
	{
		[self updateJumpTable];
	}
	else
	{
		void * tmp = _jumpToInstalled;

		_jumpToInstalled = NULL;
		_nbElemInstalled = 0;
		
		free(tmp);
	}
}

- (NSInteger) selectedRow
{
	return selectedIndex;
}

#pragma mark - Data manipulation

- (void) reloadData
{
	BOOL newData = NO;
	
	NSInteger element = [self getSelectedElement];
	
	for(uint pos = 0; pos < amountData; pos++)
	{
		if(updateIfRequired(&((PROJECT_DATA*) data)[pos], RDB_CTXSERIES))
		{
			_installed[pos] = isProjectInstalledInCache(((PROJECT_DATA*)data)[pos].cacheDBID);
			newData = YES;
		}
	}
	
	if(newData)
	{
		if(_jumpToInstalled != NULL)
			[self updateJumpTable];

		[_tableView reloadData];
		
		if(element != -1)
			[self selectRow:[self getIndexOfElement:element]];
	}
}

- (void) updateJumpTable
{
	if(amountData == 0)
		return;
	
	if(_jumpToInstalled != NULL)
		free(_jumpToInstalled);
	
	_jumpToInstalled = NULL;
	
	uint rawJumpTable[amountData], positionRawTable, positionInstalled;
	
	for (positionInstalled = positionRawTable = 0; positionInstalled < amountData; positionInstalled++)
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
		if (_jumpToInstalled != NULL && index < _nbElemInstalled)
			index = _jumpToInstalled[index];
		
		output = getCopyOfProjectData(((PROJECT_DATA*) data)[index]);
	}
	else
		memset(&output, 0, sizeof(output));
	
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
	if(data == NULL)
		return 0;
	else if(_jumpToInstalled == NULL)
		return amountData;
	else
		return _nbElemInstalled;
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
		return nil;
}

- (void) tableView:(NSTableView *)tableView didAddRowView:(NSTableRowView *)rowView forRow:(NSInteger)row
{
	RakText * element = [tableView viewAtColumn:0 row:row makeIfNecessary:NO];
	
	if (row == selectedIndex)
	{
		[element setTextColor:highlight];
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
	return GUI_THREAD_SERIES;
}

- (PROJECT_DATA) getProjectDataForDrag : (uint) row
{
	return [self getElementAtIndex:row];
}

- (void) fillDragItemWithData:(RakDragItem *)item :(uint)row
{
	PROJECT_DATA project = getCopyOfProjectData([self getElementAtIndex:row]);
	
	BOOL isTome = [item defineIsTomePriority:&project alreadyRefreshed:NO];
	
	getUpdatedCTList(&project, true);
	getUpdatedCTList(&project, false);
	
	[item setDataProject:project isTome:isTome element:VALEUR_FIN_STRUCT];
}

@end
