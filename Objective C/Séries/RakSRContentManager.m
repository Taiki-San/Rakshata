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

@implementation RakSRStupidDataStructure
@end

@implementation RakSRContentManager

- (instancetype) init
{
	self = [super init];
	
	if(self != nil)
	{
		if(![self initData:&project :&cacheList :&filteredToOrdered :&orderedToSorted :&nbElemFull :&nbElemActivated : YES])
			return nil;
		
		//Okay, we have all our data, we can register for updates
		[RakDBUpdate registerForUpdate:self :@selector(DBUpdated:)];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(restrictionsUpdated:) name:NOTIFICATION_SEARCH_UPDATED object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(installedOnlyTriggered:) name:NOTIFICATION_INSTALLED_ONLY_STAB object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(freeOnlyTriggered:) name:NOTIFICATION_FREE_ONLY object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(favsOnlyTriggered:) name:NOTIFICATION_FAVS_ONLY object:nil];
		
		_sharedReference = [NSMutableArray array];
		
		RakSRStupidDataStructure * element;
		for(uint i = 0; i < nbElemActivated; i++)
		{
			element = [[RakSRStupidDataStructure alloc] init];
			element.index = filteredToOrdered[i];
			[_sharedReference addObject:element];
		}
	}
	
	return self;
}

- (void) dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
}

//We craft two lists in order to make diffing feasable in O(n)

//cacheList contain a sorted list of the cacheID in the active projects
//filteredToOrdered tell the index of the item of rank N in case of filters

//cacheList makes it easy to detect insertion and deletion
//orderedToSorted makes it easy to detect moves
//filteredToOrdered serve as a jumptable to deal with filters

- (BOOL) initData : (PROJECT_DATA **) _project : (uint **) _cacheList : (uint **) _filteredToOrdered : (uint **) _orderedToSorted : (uint *) _nbElemFull : (uint *) _nbElemActivated : (BOOL) includeCacheRefresh
{
	if(!includeCacheRefresh && project == NULL)
		includeCacheRefresh = YES;
	
	//We get the full list
	if(includeCacheRefresh)
	{
		*_project = getCopyCache(SORT_NAME, _nbElemFull);
		if(*_project == NULL || *_nbElemFull == 0)
		{
			if(!isDBProjectEmpty())
				return NO;

			*_project = NULL;
			*_cacheList = NULL;
			*_orderedToSorted = NULL;
			*_nbElemFull = *_nbElemActivated = 0;
			return YES;
		}
	}
	else
	{
		*_project = project;
		*_nbElemFull = nbElemFull;
		
		if(nbElemFull == 0)
			return isDBProjectEmpty();
	}
	
	//We get the filtered list
	uint * filtered = getFilteredProject(_nbElemActivated, commitedSearch != nil ? [commitedSearch UTF8String] : NULL, installedOnly, freeOnly, favsOnly);
	if(filtered == NULL && *_nbElemActivated != 0)
	{
		if(includeCacheRefresh)
			freeProjectData(*_project);
		return NO;
	}
	
	BOOL noData = *_nbElemActivated == 0;
	const uint invalidValue = UINT_MAX;
	
	//We alloc memory for everything
	*_cacheList = malloc(*_nbElemFull * sizeof(uint));
	*_orderedToSorted = malloc(*_nbElemFull * sizeof(uint));
	
	if(!noData)
		*_filteredToOrdered = malloc(*_nbElemActivated * sizeof(uint));
	else
		*_filteredToOrdered = NULL;
	
	if(*_cacheList == NULL || *_orderedToSorted == NULL || (!noData && *_filteredToOrdered == NULL))
	{
		if(includeCacheRefresh)
		{
			freeProjectData(*_project);
			*_project = NULL;
		}
		free(filtered);
		free(*_cacheList);			*_cacheList = NULL;
		free(*_filteredToOrdered);	*_filteredToOrdered = NULL;
		free(*_orderedToSorted);	*_orderedToSorted = NULL;
		return NO;
	}
	else
	{
		memset(*_cacheList, (int) UINT_MAX, *_nbElemFull * sizeof(uint));
		memset(*_orderedToSorted, (int) invalidValue, *_nbElemFull * sizeof(uint));
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
		{
			freeProjectData(*_project);
			*_project = NULL;
		}
		free(filtered);
		free(*_cacheList);			*_cacheList = NULL;
		free(*_filteredToOrdered);	*_filteredToOrdered = NULL;
		free(*_orderedToSorted);	*_orderedToSorted = NULL;
		return NO;
	}
	
	//We create a buffer large enough to fit every data at the index of its value
	uint * collector = malloc(++highestValue * sizeof(uint));
	if(collector == NULL)
	{
		if(includeCacheRefresh)
		{
			freeProjectData(*_project);
			*_project = NULL;
		}
		free(filtered);
		free(*_cacheList);			*_cacheList = NULL;
		free(*_filteredToOrdered);	*_filteredToOrdered = NULL;
		free(*_orderedToSorted);	*_orderedToSorted = NULL;
		return NO;
	}
	memset(collector, (int) invalidValue, highestValue * sizeof(uint));
	
	//We store the item position
	for(uint i = 0; i < *_nbElemFull; i++)
		collector[(*_project)[i].cacheDBID] = i;			//collector:			ID						-> rankOfIDInSortedArray
	
	//We compact the large dataset in the reduced final structure
	uint tmpSortedToOrdered[*_nbElemFull];
	memset(tmpSortedToOrdered, 0, sizeof(tmpSortedToOrdered));	//Not really needed but clang analyzer want some clearer initialization
																//I believe so because highestValue > _nbElemFull, and collector contain _nbElemFull valid entry

	for(uint i = 0, pos = 0; i < highestValue; i++)
	{
		if(collector[i] != invalidValue)
		{
			(*_cacheList)[pos] = i;							//cacheList:			rankOfID				-> ID (cacheDBID)
			(*_orderedToSorted)[pos] = collector[i];		//orderedToSorted:		rankOfID				-> rankOfIDInSortedArray
			tmpSortedToOrdered[collector[i]] = pos++;		//tmpSortedToOrdered:	rankOfIDInSortedArray	-> rankOfID
			collector[i] = invalidValue;					//We will recycle the collector later
		}
	}
	
	//_nbElemFull > 0, donc _nbElemActivated == 0 pas un problème pour la première branche
	if(*_nbElemFull == *_nbElemActivated)
	{
		for(uint i = 0; i < *_nbElemActivated; i++)
			(*_filteredToOrdered)[i] = tmpSortedToOrdered[i];
	}
	else if(!noData)
	{
		//filtered contain a list of all the ID we want
		//We need to write the ranks in orderedToSorted that lead to the projects with the ID
		//To do that, we have 3 tools:
		//		- cacheList:			rankOfID				-> ID
		//		- orderedToSorted:		rankOfID				-> rankOfIDInSortedArray
		//		- tmpSortedToOrdered:	rankOfIDInSortedArray	-> rankOfID
		//
		//NB: orderedToSorted[tmpSortedToOrdered[i]] == i
		//
		//	First: we find the various rankOfID of the ID in filtered with cacheList
		//			However, we need to sort them. We use the collector trick to sort in O(n)
		//			They have to be sorted considering the output of orderedToSorted[]
		//
		
		for(uint i = 0, filteredPos = 0; i < *_nbElemFull && filteredPos < *_nbElemActivated; i++)
		{
			if(filtered[filteredPos] == (*_cacheList)[i])
				collector[(*_orderedToSorted)[i]] = filteredPos++;
		}
		
		//	collector:		rankOfIDInSortedArray	->	is selected
		//	Second:	We can then compress collector, convert back the rankOfIDInSortedArray in rankOfID and we are good
		
		for(uint i = 0, filteredPos = 0; i < *_nbElemFull && filteredPos < *_nbElemActivated; i++)
		{
			if(collector[i] != invalidValue)
				(*_filteredToOrdered)[filteredPos++] = tmpSortedToOrdered[i];
		}
	}
	
	free(collector);
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
		return (NSView*) _grid;
	
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
	{
		[_grid setFrame:frame];
		[_grid updateTrackingArea];
	}
	
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
	{
		[_grid resizeAnimation:frame];
		[_grid updateTrackingArea];
	}
	
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

- (void) installedOnlyTriggered : (NSNotification *) notification
{
	if(notification.object != nil && [notification.object isKindOfClass:[NSNumber class]])
	{
		BOOL newInstalledOnly = [notification.object boolValue];
		
		if(newInstalledOnly != installedOnly)
		{
			installedOnly = newInstalledOnly;
			[self updateContext:NO];
		}
	}
}

- (void) freeOnlyTriggered : (NSNotification *) notification
{
	if(notification.object != nil && [notification.object isKindOfClass:[NSNumber class]])
	{
		BOOL newFreeOnly = [notification.object boolValue];
		
		if(newFreeOnly != freeOnly)
		{
			freeOnly = newFreeOnly;
			[self updateContext:NO];
		}
	}
}

- (void) favsOnlyTriggered : (NSNotification *) notification
{
	if(notification.object != nil && [notification.object isKindOfClass:[NSNumber class]])
	{
		BOOL newFavsOnly = [notification.object boolValue];
		
		if(newFavsOnly != favsOnly)
		{
			favsOnly = newFavsOnly;
			[self updateContext:NO];
		}
	}
}

//We won't try to be smartasses, we grab the new context, diff it, apply changes...
//Because of that, it's pretty expensive, both in computation o(~5n) and in memory o(higestValue)
- (void) DBUpdated : (NSNotification*) notification
{
	[self updateContext:YES];
}

- (void) restrictionsUpdated : (NSNotification *) notification
{
	if(notification != nil)
	{
		if(notification.object != nil && [notification.object isKindOfClass:[NSString class]])
			commitedSearch = notification.object;
		
		[self updateContext:NO];
	}
}

- (void) updateContext : (BOOL) includeCacheRefresh
{
	PROJECT_DATA * newProject;
	uint * newCacheList, * newFilteredToOrdered, * newOrderedToSorted, newNbElemFull, newNbElemActivated;
	
	if([self initData:&newProject :&newCacheList :&newFilteredToOrdered :&newOrderedToSorted :&newNbElemFull :&newNbElemActivated : includeCacheRefresh])
	{
		//+ 1 to remove the case where newNbElemActivated == 0
		int64_t removal[nbElemActivated + 1], insertion[newNbElemActivated + 1], nbRemoval = 0, nbInsertion = 0, posOld = 0, posNew = 0;
		BOOL maskValidated[nbElemFull], newMaskValidated[newNbElemFull];
		
		memset(maskValidated, 1, nbElemFull * sizeof(BOOL));
		memset(newMaskValidated, 1, newNbElemFull * sizeof(BOOL));
		
		//First, we detect suppressions/deletions in the global cache list
		while(posOld < nbElemFull && posNew < newNbElemFull)
		{
			//Deletion
			if(cacheList[posOld] < newCacheList[posNew])
			{
				maskValidated[posOld] = NO;	//Invalidate the entry
				removal[nbRemoval++] = orderedToSorted[posOld++];
			}
			
			//Insertion
			else if(cacheList[posOld] > newCacheList[posNew])
			{
				newMaskValidated[posNew] = NO;	//Invalidate the entry
				insertion[nbInsertion++] = ++posNew * -1;
			}
			
			else if(wstrcmp(project[orderedToSorted[posOld]].projectName, newProject[newOrderedToSorted[posNew]].projectName))
			{
				removal[nbRemoval++] = orderedToSorted[posOld];				maskValidated[posOld++] = NO;
				insertion[nbInsertion++] = (posNew + 1) * -1;				newMaskValidated[posNew++] = NO;
			}
			
			else
			{
				posOld++;
				posNew++;
			}
		}
		
		//Complete the list
		for(; posOld < nbElemFull; posOld++)
		{
			if(maskValidated[posOld])
			{
				removal[nbRemoval++] = orderedToSorted[posOld];
				maskValidated[posOld] = NO;
			}
		}
		
		for(; posNew < newNbElemFull; posNew++)
		{
			if(newMaskValidated[posNew])
			{
				insertion[nbInsertion++] = (posNew + 1) * -1;;
				newMaskValidated[posNew] = NO;
			}
		}
		
		//We then track modification in the installed list
		posOld = posNew = 0;
		uint offsetOld = 0, offsetNew = 0;
		while(posOld < nbElemActivated && posNew < newNbElemActivated)
		{
			if(!maskValidated[filteredToOrdered[posOld]])
			{
				offsetOld++;
				posOld++;
			}
			
			else if(!newMaskValidated[newFilteredToOrdered[posNew]])
			{
				offsetNew++;
				posNew++;
			}
			
			//Removal
			else if(orderedToSorted[filteredToOrdered[posOld]] - offsetOld < newOrderedToSorted[newFilteredToOrdered[posNew]] - offsetNew)
				removal[nbRemoval++] = orderedToSorted[filteredToOrdered[posOld++]];
			
			//Insertion
			else if(orderedToSorted[filteredToOrdered[posOld]] - offsetOld > newOrderedToSorted[newFilteredToOrdered[posNew]] - offsetNew)
				insertion[nbInsertion++] = posNew++;
			
			else
			{
				posNew++;
				posOld++;
			}
		}
		
		//Complete the list
		for(; MAX(posOld, nbRemoval) < nbElemActivated; posOld++)
		{
			if(maskValidated[filteredToOrdered[posOld]])
				removal[nbRemoval++] = orderedToSorted[filteredToOrdered[posOld]];
		}
		
		for(; posNew < newNbElemActivated; posNew++)
		{
			if(newMaskValidated[newFilteredToOrdered[posNew]])
				insertion[nbInsertion++] = posNew;
		}
		
#ifdef EXTENSIVE_LOGGING
		printf("Updating RakSRContentManager: %d - %lld + %lld = %d\n", nbElemActivated, nbRemoval, nbInsertion, newNbElemActivated);
#endif
		
		//We replace the old data structure
		PROJECT_DATA * oldProject = project;
		uint * oldCacheList = cacheList, * oldFilteredToOrdered = filteredToOrdered, * oldOrderedToSorted = orderedToSorted;
		
		project = newProject;
		cacheList = newCacheList;
		filteredToOrdered = newFilteredToOrdered;
		orderedToSorted = newOrderedToSorted;
		nbElemActivated = newNbElemActivated;
		nbElemFull = newNbElemFull;
		
		if(includeCacheRefresh)
			freeProjectData(oldProject);
		free(oldCacheList);
		free(oldFilteredToOrdered);
		free(oldOrderedToSorted);
		
		//Apply changes, yay
		NSMutableArray *content = [self mutableArrayValueForKey:@"sharedReference"];
		
		//We have to start from the end of the sorted array so we don't progressively offset our deletion/insertion cursor
		if(nbRemoval)
		{
			qsort(removal, (uint) nbRemoval, sizeof(int64_t), sortNumbers);
			for(uint i = nbRemoval; i != 0; [content removeObjectAtIndex:(uint) removal[--i]]);
		}
		
		if(nbInsertion)
		{
			RakSRStupidDataStructure * element;
			for(uint i = 0, indexInFinal = 0; i < nbInsertion; i++)
			{
				//If the item was inserted by the second part of the routine, insertion[i] contain the index in filteredToOrdered to inject
				//This gives us our injection index
				if(insertion[i] >= 0)
				{
					indexInFinal = insertion[i];
					insertion[i] = filteredToOrdered[indexInFinal];
				}
				//First part of the routine, this is identified by the value being incremented, then multiplied by -1
				//This value is directly filteredToOrdered[i] :(
				//We have to find it to get our insertion point
				else
				{
					insertion[i] *= -1;
					insertion[i]--;
					
					for(indexInFinal = 0; indexInFinal < nbElemActivated && filteredToOrdered[indexInFinal] != (uint) insertion[i]; indexInFinal++);
					if(indexInFinal == nbElemActivated)
						indexInFinal = [content count];
				}
				
				element = [[RakSRStupidDataStructure alloc] init];
				element.index = insertion[i];
				
				[content insertObject:element atIndex:indexInFinal++];
			}
		}
		
		//We fucked up
		if([content count] != nbElemActivated || nbElemActivated > nbElemFull)
		{
#ifdef EXTENSIVE_LOGGING
			NSLog(@"We failed at diffing :( %lu instead of %d", (unsigned long)[content count], nbElemActivated);
#endif
			[content removeAllObjects];
			
			RakSRStupidDataStructure * element;
			for(uint i = 0; i < nbElemActivated; i++)
			{
				element = [[RakSRStupidDataStructure alloc] init];
				element.index = filteredToOrdered[i];
				[content addObject:element];
			}
		}
		
		//We refresh the content frame afterward, as the scrollview tend to bitch a lot :x
		[self setFrame:previousFrame];
	}
}

#pragma mark - Interface

- (uint) nbElement
{
	return nbElemFull;
}

- (uint) nbActivatedElement
{
	return nbElemActivated;
}

- (PROJECT_DATA *) getDataAtIndex : (uint) index
{
	if(index >= nbElemFull)
		return NULL;
	
	return &(project[orderedToSorted[index]]);
}

@end
