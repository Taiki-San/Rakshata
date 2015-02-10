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
 ********************************************************************************************/

@implementation RakSRContentManager

- (instancetype) init
{
	self = [super init];
	
	if(self != nil)
	{
		if(![self initData:&project :&cacheList :&activatedList :&filteredToSorted :&nbElemFull :&nbElemActivated : YES])
			return nil;
		
		//Okay, we have all our data, we can register for updates
		[RakDBUpdate registerForUpdate:self :@selector(DBUpdated:)];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(restrictionsUpdated:) name:NOTIFICATION_SEARCH_UPDATED object:nil];
		
		_sharedReference = [NSMutableArray array];
		
		for(uint i = 0; i < nbElemActivated; i++)
			[_sharedReference addObject:@(filteredToSorted[i])];
	}
	
	return self;
}

- (BOOL) initData : (PROJECT_DATA **) _project : (uint **) _cacheList : (BOOL **) _activatedList : (uint **) _filteredToSorted : (uint *) _nbElemFull : (uint *) _nbElemActivated : (BOOL) includeCacheRefresh
{
	if(!includeCacheRefresh && project == NULL)
		includeCacheRefresh = YES;
	
	//We get the full list
	if(includeCacheRefresh)
	{
		*_project = getCopyCache(SORT_NAME, _nbElemFull);
		if(*_project == NULL)
			return NO;
	}
	else
	{
		*_project = project;
		*_nbElemFull = nbElemFull;
	}
	
	//We get the filtered list
	uint * filtered = getFilteredProject(_nbElemActivated, commitedSearch != nil ? [commitedSearch cStringUsingEncoding:NSUTF8StringEncoding] : NULL);
	if(filtered == NULL)
	{
		if(includeCacheRefresh)
			freeProjectData(*_project);
		return NO;
	}
	
	//We alloc memory for everything
	*_cacheList = malloc(*_nbElemFull * sizeof(uint));
	*_activatedList = calloc(*_nbElemFull, sizeof(BOOL));
	*_filteredToSorted = malloc(*_nbElemActivated * sizeof(uint));
	
	if(*_cacheList == NULL || *_activatedList == NULL || *_filteredToSorted == NULL)
	{
		if(includeCacheRefresh)
			freeProjectData(*_project);
		free(filtered);
		free(*_cacheList);
		free(*_activatedList);
		free(*_filteredToSorted);
		return NO;
	}
	
	//We look for the highest value
	uint highestValue = *_nbElemFull;
	for(uint i = 0; i < *_nbElemFull; i++)
	{
		if((*_project)[i].cacheDBID > highestValue)
			highestValue = (*_project)[i].cacheDBID;
	}
	
	if(highestValue == UINT_MAX)	//We would get an overflow
	{
		if(includeCacheRefresh)
			freeProjectData(*_project);
		free(filtered);
		free(*_cacheList);
		free(*_activatedList);
		free(*_filteredToSorted);
		return NO;
	}
	
	//We create a buffer large enough to fit every data at the index of its value
	const uint invalidValue = UINT_MAX;
	uint * collector = malloc(++highestValue * sizeof(uint));
	if(collector == NULL)
	{
		if(includeCacheRefresh)
			freeProjectData(*_project);
		free(filtered);
		free(*_cacheList);
		free(*_activatedList);
		free(*_filteredToSorted);
		return NO;
	}
	memset(collector, invalidValue, highestValue * sizeof(uint));
	
	//We store the item position
	for(uint i = 0; i < *_nbElemFull; i++)
		collector[(*_project)[i].cacheDBID] = i;
	
	//We compact the large dataset in the reduced final structure
	uint orderedToSorted[*_nbElemFull];
	memset(orderedToSorted, invalidValue, *_nbElemFull * sizeof(uint));
	
	for(uint i = 0, pos = 0; i < highestValue; i++)
	{
		if(collector[i] != invalidValue)
		{
			(*_cacheList)[pos] = i;
			orderedToSorted[pos++] = collector[i];
		}
	}
	
	free(collector);
	
	if(*_nbElemFull == *_nbElemActivated)
	{
		memset(*_activatedList, 1, *_nbElemFull);
		for(uint i = 0; i < *_nbElemActivated; i++)
			(*_filteredToSorted)[i] = i;
	}
	else
	{
		for(uint i = 0, filteredPos = 0; i < *_nbElemFull && filteredPos < *_nbElemActivated; i++)
		{
			if(filtered[filteredPos] == (*_cacheList)[i])
			{
				(*_activatedList)[orderedToSorted[i]] = YES;
				filteredPos++;
			}
		}
		
		for(uint i = 0, filteredPos = 0; i < *_nbElemFull && filteredPos < *_nbElemActivated; i++)
		{
			if((*_activatedList)[i])
				(*_filteredToSorted)[filteredPos++] = i;
		}
	}
	
	free(filtered);
	return YES;
}

- (void) initViews
{
	previousFrame = _controlView.bounds;
	
	_grid = [[RakGridView alloc] initWithFrame : previousFrame : self];
	if(_grid != nil)
	{
		NSView * view = _grid.contentView;
		
		if(_activeView != SR_CELLTYPE_GRID)
		{
			view.alphaValue = 0;
			view.hidden = YES;
		}
		
		[_controlView addSubview : view];
	}
	
	_initialized = YES;
}

#pragma mark - Property

- (NSView *) getActiveView
{
	if(_activeView == SR_CELLTYPE_GRID)
		return (NSView*) [_grid contentView];
	
	else if(_activeView == SR_CELLTYPE_REPO)
		return nil;
	
	else if(_activeView == SR_CELLTYPE_LIST)
		return nil;
	
	return nil;
}

- (void) setFrame : (NSRect) frame
{
	previousFrame = frame;
	
	if(_activeView == SR_CELLTYPE_GRID)
		[_grid setFrame:frame];
	
	else if(_activeView == SR_CELLTYPE_REPO)
	{
		
	}
	
	else if(_activeView == SR_CELLTYPE_LIST)
	{
		
	}
}

- (void) resizeAnimation : (NSRect) frame
{
	previousFrame = frame;
	
	if(_activeView == SR_CELLTYPE_GRID)
		[_grid resizeAnimation:frame];
	
	else if(_activeView == SR_CELLTYPE_REPO)
	{
		
	}
	
	else if(_activeView == SR_CELLTYPE_LIST)
	{
		
	}
}

- (byte) activeView
{
	return _activeView;
}

- (void) setActiveView : (byte) activeView
{
	[self setActiveView:activeView :YES];
}

- (void) setActiveView : (byte) activeView : (BOOL) animated
{
	if(!_initialized || activeView == _activeView)
		return;
	
	if(animated)
		[NSAnimationContext beginGrouping];
	
	//We hide the previous view
	if(_activeView == SR_CELLTYPE_GRID)
	{
		if(animated)
			[_grid contentView].animator.alphaValue = 0;
		else
			[_grid contentView].alphaValue = 0;
	}
	else if(_activeView == SR_CELLTYPE_REPO)
	{}
	else if(_activeView == SR_CELLTYPE_LIST)
	{}
	
	__block byte oldActive = _activeView;
	_activeView = activeView;

	//We reveal the new view
	if(activeView == SR_CELLTYPE_GRID)
	{
		[_grid contentView].hidden = NO;

		if(animated)
			[_grid contentView].animator.alphaValue = 1;
		else
			[_grid contentView].alphaValue = 1;
	}
	else if(activeView == SR_CELLTYPE_REPO)
	{}
	else if(activeView == SR_CELLTYPE_LIST)
	{}
	
	if(animated)
	{
		[[NSAnimationContext currentContext] setCompletionHandler:^{
			if(oldActive == SR_CELLTYPE_GRID)
			{
				if([_grid contentView].alphaValue == 0)
					[_grid contentView].hidden = YES;
			}
			else if(oldActive == SR_CELLTYPE_REPO)
			{}
			else if(oldActive == SR_CELLTYPE_LIST)
			{}

		}];
		
		[NSAnimationContext endGrouping];
	}
}

#pragma mark - Manage update

//We won't try to be smartasses, we grab the new context, diff it, apply changes...
//Because of that, it's pretty expensive, both in computation o(~5n) and in memory o(higestValue)
- (void) DBUpdated : (NSNotification*) notification
{
	[self updateContext:YES];
}

- (void) restrictionsUpdated : (NSNotification *) notification
{
	if(notification != nil && notification.object != nil && [notification.object isKindOfClass:[NSString class]])
	{
		commitedSearch = notification.object;
		[self updateContext:NO];
	}
}

- (void) updateContext : (BOOL) includeCacheRefresh
{
	PROJECT_DATA * newProject;
	uint * newCacheList, * newFilteredToSorted, newNbElemFull, newNbElemActivated;
	BOOL * newActivatedList;
	
	if([self initData:&newProject :&newCacheList :&newActivatedList :&newFilteredToSorted :&newNbElemFull :&newNbElemActivated : includeCacheRefresh])
	{
		uint removal[nbElemActivated], insertion[newNbElemActivated], nbRemoval = 0, nbInsertion = 0, posOld = 0, posNew = 0;
		uint _filteredToSorted[nbElemActivated], _newFilteredToSorted[newNbElemActivated];
		
		memcpy(_filteredToSorted, filteredToSorted, nbElemActivated * sizeof(uint));
		memcpy(_newFilteredToSorted, newFilteredToSorted, newNbElemActivated * sizeof(uint));
		
		//First, we detect suppressions/deletions
		while(posOld < nbElemActivated && posNew < newNbElemActivated)
		{
			if(cacheList[_filteredToSorted[posOld]] < newCacheList[_newFilteredToSorted[posNew]])
			{
				_filteredToSorted[posOld] = UINT_MAX;	//Invalidate the entry
				removal[nbRemoval++] = posOld++;
			}
			else if(cacheList[_filteredToSorted[posOld]] > newCacheList[_newFilteredToSorted[posNew]])
			{
				_newFilteredToSorted[posNew] = UINT_MAX;
				insertion[nbInsertion++] = posNew++;
			}
			else
			{
				posOld++;
				posNew++;
			}
		}

		while(posOld < nbElemActivated)			{	_filteredToSorted[posOld] = UINT_MAX;		removal[nbRemoval++] = posOld++;		}
		while(posNew < newNbElemActivated)		{	_newFilteredToSorted[posNew] = UINT_MAX;	insertion[nbInsertion++] = posNew++;	}
		
		//Then we look for moves
		posOld = posNew = 0;
		while(posOld < nbElemActivated && posNew < newNbElemActivated)
		{
			if(_filteredToSorted[posOld] == UINT_MAX)
				posOld++;

			else if(_newFilteredToSorted[posNew] == UINT_MAX)
				posNew++;
			
			else if(_filteredToSorted[posOld] != newFilteredToSorted[posNew])
			{
				removal[nbRemoval++] = posOld++;
				insertion[nbInsertion++] = posNew++;
			}
			
			else
			{
				posOld++;
				posNew++;
			}
		}
		
		//We replace the old data structure
		PROJECT_DATA * oldProject = project;
		uint * oldCacheList = cacheList, * oldFilteredToSorted = filteredToSorted;
		BOOL * oldActivatedList = activatedList;
		
		project = newProject;
		cacheList = newCacheList;
		filteredToSorted = newFilteredToSorted;
		activatedList = newActivatedList;
		nbElemActivated = newNbElemActivated;
		nbElemFull = newNbElemFull;
		
		if(includeCacheRefresh)
			freeProjectData(oldProject);
		free(oldCacheList);
		free(oldFilteredToSorted);
		free(oldActivatedList);
		
		//Apply changes, yay
		NSMutableArray *content = [self mutableArrayValueForKey:@"sharedReference"];
		
		for(uint i = nbRemoval; i != 0; [content removeObjectAtIndex:removal[--i]]);
		for(uint i = 0; i < nbInsertion; i++)
		{
			[content insertObject:@(filteredToSorted[insertion[i]]) atIndex:insertion[i]];
		}
	}
}

#pragma mark - Interface

- (uint) nbElement
{
	return nbElemFull;
}

- (PROJECT_DATA *) getDataAtIndex : (uint) index
{
	if(index >= nbElemFull)
		return NULL;
		
	return &(project[index]);
}

@end
