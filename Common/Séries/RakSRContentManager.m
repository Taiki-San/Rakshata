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
 **			file, You can obtain one at https://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
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
		//Okay, we have all our data, we can register for updates
		[RakDBUpdate registerForUpdate:self :@selector(DBUpdated:)];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(restrictionsUpdated:) name:NOTIFICATION_SEARCH_UPDATED object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(installedOnlyTriggered:) name:NOTIFICATION_INSTALLED_ONLY_STAB object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(freeOnlyTriggered:) name:NOTIFICATION_FREE_ONLY object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(favsOnlyTriggered:) name:NOTIFICATION_FAVS_ONLY object:nil];

		if(![self initData:&project :&cacheList :&filteredToOrdered :&orderedToSorted :&nbElemFull :&nbElemActivated : YES])
			return nil;
		
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
	
	*_cacheList = NULL;
	*_orderedToSorted = NULL;
	*_filteredToOrdered = NULL;
	*_nbElemActivated = 0;
	
	//We get the full list
	if(includeCacheRefresh)
	{
		*_project = getCopyCache(SORT_NAME, _nbElemFull);
		if(*_project == NULL || *_nbElemFull == 0)
		{
			return isDBProjectEmpty();
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
		//
		//		- filteredToOrdered:	rankInList ->	rankOfID
		
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

#if !TARGET_OS_IPHONE
- (void) initViews
{
	previousFrame = _controlView.bounds;
	
	_grid = [[RakGridView alloc] initWithFrame : previousFrame : self];
	if(_grid != nil)
	{
		RakView * view = _grid.contentView;
		
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

- (RakView *) getActiveView
{
	if(_activeView == SR_CELLTYPE_GRID)
		return (RakView*) _grid;
	
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
			[_grid contentView]. alphaAnimated = 0;
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
			[_grid contentView]. alphaAnimated = 1;
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

#endif

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
		int64_t insertion[newNbElemActivated + 1], nbRemoval = 0, nbInsertion = 0, posOld = 0, posNew = 0;
		BOOL maskValidated[nbElemFull], newMaskValidated[newNbElemFull];
		
		memset(maskValidated, 1, nbElemFull * sizeof(BOOL));
		memset(newMaskValidated, 1, newNbElemFull * sizeof(BOOL));
		
		//First, we detect suppressions/deletions in the global cache list
		//
		//	cacheList contain a sorted list of the cacheID of the project in the list
		//				by looking for variations. CachedID only grows, so if there is a missing entry, something was deleted
		//				if there is extra entries at the end, there was insertion
		//
		//	NB: We don't write the same values in removal and insertion for good reasons, see the deletion/insertion routine to see why
		//
		
		while(posOld < nbElemFull && posNew < newNbElemFull)
		{
			//Deletion
			if(cacheList[posOld] < newCacheList[posNew])
			{
				maskValidated[orderedToSorted[posOld++]] = NO;	//Invalidate the entry
				nbRemoval++;
			}
			
			//Insertion, I'm not even sure this option can be hit
			else if(cacheList[posOld] > newCacheList[posNew])
			{
				newMaskValidated[posNew] = NO;				//Invalidate the entry
				insertion[nbInsertion++] = ++posNew * -1;	//We mark that this is not an index of newFilteredToOrdered, so the diff routine will have to find it itself
			}
			
			//If the name are different, we mark to update the tile (removing + inserting it back)
			else if(wstrcmp(project[orderedToSorted[posOld]].projectName, newProject[newOrderedToSorted[posNew]].projectName))
			{
				maskValidated[orderedToSorted[posOld++]] = NO;				nbRemoval++;
				newMaskValidated[posNew++] = NO;							insertion[nbInsertion++] = (posNew + 1) * -1;
			}
			
			else
			{
				posOld++;
				posNew++;
			}
		}
		
		//Complete the list with all the item we had to catch, basically if extra items at the end were deleted
		for(; posOld < nbElemFull; posOld++)
		{
			maskValidated[orderedToSorted[posOld]] = NO;
			nbRemoval++;
		}
		
		//We goes through newFilteredToOrdered (rankInList -> rankOfID) and check whenever we find a cacheDBID higher than the last one we matched
		//This is a bit more expensive (as we go through values already looked up) but enable us not to force the insertion routine to look for the index in filteredToOrdered itself
		if(posNew < newNbElemFull)
		{
			uint lastMatchedID = posNew == 0 ? 0 : newCacheList[posNew - 1];
			for(uint i = 0; i < newNbElemActivated; ++i)
			{
				if(newCacheList[newFilteredToOrdered[i]] > lastMatchedID)
				{
					insertion[nbInsertion++] = i;
					newMaskValidated[newFilteredToOrdered[i]] = NO;
				}
			}
		}
		
		//We then track modification in the installed list by going through filteredToOrdered and newFilteredToOrdered
		posOld = posNew = 0;
		uint offsetOld = 0, offsetNew = 0;
		while(posOld < nbElemActivated && posNew < newNbElemActivated)
		{
			//Check if the item was not already deleted
			if(!maskValidated[orderedToSorted[filteredToOrdered[posOld]]])
			{
				offsetOld++;
				posOld++;
			}
			
			//Check if the item was not already inserted
			else if(!newMaskValidated[newFilteredToOrdered[posNew]])
			{
				offsetNew++;
				posNew++;
			}
			
			//If the index of the posOld nth entry in the sorted list in below the one of posNew,
			//		this mean the entry pointed by posOld is skipped in the new filteredToOrdered.
			//If the opposite is true, then some items that were not in the old list are in the new list
			//
			//We must keep in mind the items that were already scrapped by the first routine will
			//		introduce an offset that we must cancel (the positions in the main list will
			//		be different because some items don't exist anymore in the new list).
			//
			//No offset must be introduced in this part because the main list isn't missing entries, they are just skipped by filteredToOrdered
			
			//Removal
			else if(orderedToSorted[filteredToOrdered[posOld]] - offsetOld < newOrderedToSorted[newFilteredToOrdered[posNew]] - offsetNew)
			{
				maskValidated[orderedToSorted[filteredToOrdered[posOld++]]] = NO;
				nbRemoval++;
			}
			
			//Insertion
			else if(orderedToSorted[filteredToOrdered[posOld]] - offsetOld > newOrderedToSorted[newFilteredToOrdered[posNew]] - offsetNew)
				insertion[nbInsertion++] = posNew++;
			
			//We just go to the next element
			else
			{
				posNew++;
				posOld++;
			}
		}
		
		//Complete the list: all items not deleted/inserted at this point are either new, or removed
		for(posOld = MAX(posOld, nbRemoval); posOld < nbElemActivated; posOld++)
		{
			if(maskValidated[orderedToSorted[filteredToOrdered[posOld]]])
			{
				maskValidated[orderedToSorted[filteredToOrdered[posOld]]] = NO;
				nbRemoval++;
			}
		}
		
		for(; posNew < newNbElemActivated; posNew++)
		{
			if(newMaskValidated[newFilteredToOrdered[posNew]])
				insertion[nbInsertion++] = posNew;
		}
		
#ifdef DEV_VERSION
		if(nbRemoval || nbInsertion)
			NSLog(@"Updating RakSRContentManager: %d - %lld + %lld = %d\n", nbElemActivated, nbRemoval, nbInsertion, newNbElemActivated);
#endif
		
		//We replace the old data structure
		PROJECT_DATA * oldProject = project;
		uint * oldCacheList = cacheList, * oldFilteredToOrdered = filteredToOrdered, * oldOrderedToSorted = orderedToSorted;
		uint oldElemActivated = nbElemActivated;
		
		project = newProject;
		cacheList = newCacheList;
		filteredToOrdered = newFilteredToOrdered;
		orderedToSorted = newOrderedToSorted;
		nbElemActivated = newNbElemActivated;
		nbElemFull = newNbElemFull;
		
		//Apply changes, yay
		NSMutableArray *content = [self mutableArrayValueForKey:@"sharedReference"];
		
		if(nbRemoval)
		{
			//We look for indexes marked as to delete, and nuke them :]
			for(uint i = oldElemActivated; i-- > 0;)
			{
				if(!maskValidated[oldOrderedToSorted[oldFilteredToOrdered[i]]])
					[content removeObjectAtIndex:i];
			}
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
		
		//We release the memory
		if(includeCacheRefresh)
			freeProjectData(oldProject);
		free(oldFilteredToOrdered);
		free(oldOrderedToSorted);
		free(oldCacheList);
		
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
		
#if !TARGET_OS_IPHONE
		//We refresh the content frame afterward, as the scrollview tend to bitch a lot :x
		[self setFrame:previousFrame];
#endif
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

- (PROJECT_DATA *) getDirectDataAtIndex : (uint) index
{
	if(index >= nbElemFull)
		return NULL;
	
	return &(project[index]);
}

- (PROJECT_DATA *) getDataAtIndex : (uint) index
{
	if(index >= nbElemFull)
		return NULL;
	
	return &(project[orderedToSorted[index]]);
}

@end
