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

@implementation RakPrefsFavoriteList

- (instancetype) initWithFrame : (NSRect) frame
{
	self = [self init];
	
	if(self != nil)
	{
		lastTransmittedSelectedRowIndex = LIST_INVALID_SELECTION;
		projectDB = getCopyCache(RDB_LOAD_FAVORITE | SORT_NAME, &_nbData);
		IDList = getFavoritesID(&lengthList);
		
		if(lengthList != _nbData || IDList == NULL)
		{
			lengthList = _nbData;
			free(IDList);	IDList = NULL;
		}
		
		[RakDBUpdate registerForUpdate:self :@selector(DBUpdated:)];
		
		repoRefresh = [NSMutableArray array];
		
		[self applyContext:frame : selectedRowIndex : -1];

		scrollView.wantsLayer = YES;
		scrollView.layer.cornerRadius = 3;
		scrollView.drawsBackground = YES;
		scrollView.backgroundColor = [Prefs getSystemColor:COLOR_BACKGROUND_REPO_LIST];
		
		[self enableDrop];
	}
	
	return self;
}

- (void) dealloc
{
	[RakDBUpdate unRegister:self];
	free(projectDB);
	free(IDList);
}

- (void) DBUpdated : (NSNotification*) notification
{
	//Full reload if we lack too much data
	if(_nbData == 0 || IDList == NULL)
	{
		PROJECT_DATA * old = projectDB;
		
		projectDB = getCopyCache(RDB_LOAD_FAVORITE | SORT_NAME, &_nbData);
		[_tableView reloadData];
		
		free(old);
		return;
	}
	
	//We check if stuffs were added/removed
	uint newNbFavorites, * newIDList = getFavoritesID(&newNbFavorites), checkNbFavorites;
	PROJECT_DATA * newFavorites = getCopyCache(RDB_LOAD_FAVORITE | SORT_NAME, &checkNbFavorites);
	
	//No more favorites :O
	if(newNbFavorites == 0 || newIDList == NULL || newFavorites == NULL)
	{
		if(lengthList != 0)
			[_tableView removeRowsAtIndexes:[NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, lengthList)] withAnimation:NSTableViewAnimationSlideLeft];
		
		free(IDList);		IDList = NULL;
		free(projectDB);	projectDB = NULL;
		free(newIDList);
		free(newFavorites);

		_nbData = 0;
		return;
	}
	
	//Let's diff that
	uint oldNbData = _nbData, posOld = 0, posNew = 0, nbInsert = 0, insertions[oldNbData], nbRemoval = 0, removal[newNbFavorites];

	while(posOld < lengthList && posNew < newNbFavorites)
	{
		if(IDList[posOld] > newIDList[posNew])
			insertions[nbInsert++] = newIDList[posNew++];
		
		else if(IDList[posOld] < newIDList[posNew])
			removal[nbRemoval++] = IDList[posOld++];
		
		else
		{
			posNew++;
			posOld++;
		}
	}
	
	//Now, apply the changes, first, deletion
	NSMutableIndexSet * indexSet = [NSMutableIndexSet indexSet];

	if(nbRemoval)
	{
		for(posOld = 0; posOld < oldNbData && [indexSet count] < nbRemoval; posOld++)
		{
			for(uint posRemoval = 0; posRemoval < nbRemoval; posRemoval++)
			{
				if(projectDB[posOld].cacheDBID == removal[posRemoval])
				{
					[indexSet addIndex:posOld];
					break;
				}
			}
		}
		
		if([indexSet count])
		{
			[_tableView removeRowsAtIndexes:indexSet withAnimation:NSTableViewAnimationSlideLeft];

			if(nbInsert)
				[indexSet removeAllIndexes];
		}
	}
	
	//Now, insertion
	free(projectDB);	free(IDList);
	projectDB = newFavorites;	IDList = newIDList;
	_nbData = newNbFavorites;

	if(nbInsert)
	{
		for(posNew = 0; posNew < _nbData && [indexSet count] < nbInsert; posNew++)
		{
			for(uint posInsert = 0; posInsert < nbInsert; posInsert++)
			{
				if(projectDB[posNew].cacheDBID == insertions[posInsert])
				{
					[indexSet addIndex:posNew];
					break;
				}
			}
		}
		
		if([indexSet count])
			[_tableView insertRowsAtIndexes:indexSet withAnimation:NSTableViewAnimationSlideLeft];
	}
}

- (Class) contentClass
{
	return [RakPrefsRepoListItemView class];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	
	scrollView.backgroundColor = [Prefs getSystemColor:COLOR_BACKGROUND_REPO_LIST];
	[scrollView setNeedsDisplay:YES];
}

#pragma mark - Repo update management

- (BOOL) isRepoRefreshing : (uint64_t) repoID
{
	return [repoRefresh containsObject: @(repoID)];
}

- (BOOL) tryInsertRepo : (uint64_t) repoID
{
	if([self isRepoRefreshing:repoID])
		return NO;
	
	[repoRefresh addObject:@(repoID)];
	[[NSNotificationCenter defaultCenter] postNotificationName:NOTIFICATION_FAVS_REPO_UPDATE object:nil];
	return YES;
}

- (void) removeRepo : (uint64_t) repoID
{
	NSUInteger index = [repoRefresh indexOfObject:@(repoID)];
	
	if(index == NSNotFound)
		return;
	
	[repoRefresh removeObject:@(repoID)];
	[[NSNotificationCenter defaultCenter] postNotificationName:NOTIFICATION_FAVS_REPO_UPDATE object:nil];
}

#pragma mark - Tableview code

- (void) tableViewSelectionDidChange:(NSNotification *)notification
{
	if(selectedRowIndex < _nbData)
		[RakPrefsFavoriteListView readProject:projectDB[selectedRowIndex]];
}

- (NSInteger) numberOfRowsInTableView : (RakTableView *) tableView
{
	return _nbData;
}

- (CGFloat) tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row
{
	return LIST_WIDE_LINE_HEIGHT;
}

- (NSView*) tableView : (RakTableView *) tableView viewForTableColumn : (NSTableColumn*) tableColumn row : (NSInteger) row
{
	if(row >= _nbData)
		return nil;

	PROJECT_DATA project = projectDB[row];
	if(!project.isInitialized)
		return nil;
		
	RakPrefsFavoriteListView * result = [tableView makeViewWithIdentifier : _identifier owner:self];
	if(result == nil)
		result = [[RakPrefsFavoriteListView alloc] initWithProject:project andList:self];
	else
		[result updateContent : project];
	
	return result;
}

#pragma mark - Drop support

- (BOOL) grantDropAuthorization : (RakDragItem *) item
{
	return ![item isFavorite];
}

- (NSDragOperation) operationForContext : (id < NSDraggingInfo >) item : (uint) sourceTab : (NSInteger) suggestedRow : (NSTableViewDropOperation) operation
{
	return [RakDragItem isFavorite:[item draggingPasteboard]] ? NSDragOperationNone : NSDragOperationCopy;
}

- (uint) getSelfCode
{
	return TAB_UNKNOWN;
}

- (BOOL) receiveDrop : (PROJECT_DATA) project : (BOOL) isTome : (uint) element : (uint) sender : (NSInteger)row : (NSTableViewDropOperation)operation
{
	setFavorite(&project);
	return YES;
}

@end
