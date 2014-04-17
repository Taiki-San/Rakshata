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

@implementation RakSerieList

- (id) init : (NSRect) frame
{
	self = [super init];
	
	if(self != nil)
	{
		data = getCopyCache(RDB_CTXSERIES | SORT_NAME | RDB_LOADALL, &amountData);
		_installed = getInstalledFromData(data, amountData);
		
		for(uint i = 0; i < amountData; changeTo(((MANGAS_DATA*)data)[i++].mangaName, '_', ' '));
		
		if(data == NULL || _installed == NULL)
		{
			NSLog(@"Failed at initialize RakSerieList, most probably a memory problem :(");

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
}

#pragma mark - Methods to deal with tableView

- (NSString*) tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
	if(rowIndex < amountData)
	{
		return [NSString stringWithCString:((MANGAS_DATA*) data)[rowIndex].mangaName encoding:NSUTF8StringEncoding];
	}
	else
		return nil;
}

#pragma mark - Get result from NSTableView

- (void)tableViewSelectionDidChange:(NSNotification *)notification;
{
	uint result = [_tableView selectedRow];
	
	if(result < amountData)
	{
		[(RakSerieView*) scrollView.superview gotClickedTransmitData: result];
	}
}

@end
