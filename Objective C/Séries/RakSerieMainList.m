/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite  **
 **                                                                                         **
 *********************************************************************************************/

@implementation RakSerieMainList

- (id) init : (NSRect) frame
{
	self = [super init];
	
	if(self != nil)
	{
		_jumpToInstalled = NULL;
		
		data = getCopyCache(RDB_CTXSERIES | SORT_NAME | RDB_LOADALL, &amountData);
		_installed = getInstalledFromData(data, amountData);
		[self updateJumpTable];
		
		for(uint i = 0; i < amountData; changeTo(((MANGAS_DATA*)data)[i++].mangaName, '_', ' '));
		
		if(data == NULL || _installed == NULL)
		{
			NSLog(@"Failed at initialize RakSerieMainList, most probably a memory problem :(");

			freeMangaData(data); //Seul _cache peut ne pas être null dans cette branche
		}
		[self applyContext:frame :-1 :-1];
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

- (BOOL) fontBold
{
	return NO;
}

#pragma mark - Data manipulation

- (void) reloadData
{
	for(uint pos = 0; pos < amountData; pos++)
	{
		if(updateIfRequired(&((MANGAS_DATA*) data)[pos], RDB_CTXSERIES))
		{
			_installed[pos] = isProjectInstalledInCache(((MANGAS_DATA*)data)[pos].cacheDBID);
			changeTo(((MANGAS_DATA*)data)[pos].mangaName, '_', ' ');
		}
	}
	
	if(_jumpToInstalled != NULL)
		[self updateJumpTable];
}

- (void) updateJumpTable
{
	if(_jumpToInstalled != NULL)
		free(_jumpToInstalled);
	
	_jumpToInstalled = NULL;
	
	uint rawJumpTable[amountData], positionRawTable, positionInstalled;
	
	for (positionInstalled = positionRawTable = 0; positionInstalled < amountData; positionInstalled++)
	{
		if(_installed[positionInstalled])
			rawJumpTable[positionRawTable++] = positionInstalled;
	}
	
	uint * newJumpTable = malloc(positionRawTable * sizeof(uint));
	
	if(newJumpTable != NULL)
	{
		memcpy(newJumpTable, &rawJumpTable, positionRawTable * sizeof(uint));
		_jumpToInstalled = newJumpTable;
		_nbElemInstalled = positionRawTable;
	}
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

#pragma mark - Get result from NSTableView

//Have to subclass because main trick doesn't work there D:
- (BOOL)tableView:(NSTableView *)aTableView shouldSelectRow:(NSInteger)rowIndex
{
	RakText* element;
	uint nbRow = [aTableView numberOfRows];
	for(uint row = 0; row < nbRow; row++)
	{
		element = [aTableView viewAtColumn:0 row:row makeIfNecessary:NO];
		if (element != nil && [element drawsBackground])
		{
			[element setTextColor:normal];
			[element setDrawsBackground:NO];
		}
	}
	
	element = [aTableView viewAtColumn:0 row:rowIndex makeIfNecessary:YES];
    if (element != nil)
    {
		[element setTextColor: highlight];
		[element setDrawsBackground:YES];
    }
	
	return YES;
}

- (void)tableViewSelectionDidChange:(NSNotification *)notification;
{
	uint result = [_tableView selectedRow];
	
	if(result < amountData)
	{
		if (_jumpToInstalled != NULL && result < _nbElemInstalled)
			result = _jumpToInstalled[result];
		
		MANGAS_DATA dataToSend = ((MANGAS_DATA*) data)[result];
		changeTo(dataToSend.mangaName, ' ', '_');
		
		[RakTabView broadcastUpdateContext: scrollView : dataToSend : NO : VALEUR_FIN_STRUCTURE_CHAPITRE];
	}
}

@end
