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
		selectedIndex = -1;
		_jumpToInstalled = NULL;
		
		data = getCopyCache(RDB_CTXSERIES | SORT_NAME | RDB_LOADALL, &amountData);
		_installed = getInstalledFromData(data, amountData);
		[self updateJumpTable];
		
		for(uint i = 0, positionInInstalled = 0; i < amountData; i++)
		{
			if(selectedDBID != -1 && selectedIndex == -1 && _installed[i])
			{
				if (((MANGAS_DATA*)data)[i].cacheDBID == selectedDBID)
					selectedIndex = positionInInstalled;
				else
					positionInInstalled++;
			}
			
			changeTo(((MANGAS_DATA*)data)[i].mangaName, '_', ' ');
		}
		
		if(data == NULL || _installed == NULL)
		{
			NSLog(@"Failed at initialize RakSerieMainList, most probably a memory problem :(");

			freeMangaData(data); //Seul _cache peut ne pas être null dans cette branche
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
	
	for(uint pos = 0; pos < amountData; pos++)
	{
		if(updateIfRequired(&((MANGAS_DATA*) data)[pos], RDB_CTXSERIES))
		{
			_installed[pos] = isProjectInstalledInCache(((MANGAS_DATA*)data)[pos].cacheDBID);
			changeTo(((MANGAS_DATA*)data)[pos].mangaName, '_', ' ');
			newData = YES;
		}
	}
	
	if(newData)
	{
		if(_jumpToInstalled != NULL)
			[self updateJumpTable];

		[_tableView reloadData];
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

- (MANGAS_DATA) getElementAtIndex : (NSInteger) index
{
	MANGAS_DATA output;
	
	if(index >= 0 && index < amountData)
	{
		if (_jumpToInstalled != NULL && index < _nbElemInstalled)
			index = _jumpToInstalled[index];
		
		memcpy(&output, &((MANGAS_DATA*) data)[index], sizeof(output));
	}
	else
		memset(&output, 0, sizeof(output));
	
	return output;
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
		
		return [NSString stringWithCString:((MANGAS_DATA*) data)[rowIndex].mangaName encoding:NSUTF8StringEncoding];
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
	else
	{
		[element setTextColor:normal];
		[element setDrawsBackground:NO];
	}
}

- (void) tableView:(NSTableView *)tableView didRemoveRowView:(NSTableRowView *)rowView forRow:(NSInteger)row
{
	if (row == -1)
		return;
	
	RakText * element = [tableView viewAtColumn:0 row:selectedIndex makeIfNecessary:NO];
	
	if(element != nil && element.drawsBackground)
	{
		[element setTextColor:normal];
		[element setDrawsBackground:NO];
	}
}

#pragma mark - Get result from NSTableView

//Have to subclass because main trick doesn't work there D:
- (BOOL)tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)rowIndex
{
	RakText* element;
	
	if(selectedIndex != -1 && selectedIndex != rowIndex)
	{
		element = [tableView viewAtColumn:0 row:selectedIndex makeIfNecessary:NO];
		if (element != nil)
		{
			[element setTextColor:normal];
			[element setDrawsBackground:NO];
		}
	}
	
	element = [tableView viewAtColumn:0 row:rowIndex makeIfNecessary:YES];
    if (element != nil)
    {
		[element setTextColor: highlight];
		[element setDrawsBackground:YES];
    }
	
	selectedIndex = rowIndex;
	
	return YES;
}

- (void)tableViewSelectionDidChange:(NSNotification *)notification;
{
	MANGAS_DATA dataToSend = [self getElementAtIndex : selectedIndex];
	
	if(dataToSend.team != NULL)
	{
		changeTo(dataToSend.mangaName, ' ', '_');
		
		[RakTabView broadcastUpdateContext: scrollView : dataToSend : NO : VALEUR_FIN_STRUCTURE_CHAPITRE];
	}
}

#pragma mark - Drag and drop support

- (uint) getSelfCode
{
	return GUI_THREAD_SERIES;
}

- (MANGAS_DATA) getProjectDataForDrag : (uint) row
{
	return [self getElementAtIndex:row];
}

- (void) fillDragItemWithData:(RakDragItem *)item :(uint)row
{
	MANGAS_DATA project = [self getElementAtIndex:row];
	BOOL isTome = [item defineIsTomePriority:&project alreadyRefreshed:NO];

	[item setDataProject:getCopyOfProjectData(project) isTome:isTome element:VALEUR_FIN_STRUCTURE_CHAPITRE];
}

@end
