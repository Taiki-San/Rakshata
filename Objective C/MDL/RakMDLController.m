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

@implementation RakMDLController

- (id) init : (MDL *) tabMDL : (NSString *) state
{
	self = [super init];
	
	if(self != nil)
	{
		_tabMDL = tabMDL;
		IDToPosition = NULL;
		quit = false;
		
		cache = getCopyCache(RDB_LOADALL | SORT_NAME | RDB_CTXMDL, &sizeCache);

		char * stateChar = NULL;
		if(state != nil && [state isNotEqualTo:STATE_EMPTY])
			stateChar = (char *) [state UTF8String];
		
		if(startMDL(stateChar, cache, &coreWorker, &todoList, &status, &statusCache, &nbElem, &quit, self))
		{
			IDToPosition = malloc(nbElem * sizeof(uint));
			if(IDToPosition != NULL)
			{
				for(discardedCount = 0; discardedCount < nbElem; discardedCount++)
					IDToPosition[discardedCount] = discardedCount;
			}
			else
			{
				[self release];			self = nil;
			}
		}
		else
		{
			[self release];			self = nil;
		}
	}
	
	return self;
}

- (void) needToQuit
{
	quit = true;
	if(isThreadStillRunning(coreWorker))
		MDLQuit();
}

- (NSString *) serializeData
{
	for(int pos = 0; pos < nbElem; pos++)
    {
		if (*status[pos] <= MDL_CODE_DEFAULT)	//Si le moindre élément n'est pas dans la pipeline, on serialise
		{
			char * data = MDLParseFile(*todoList, status, nbElem);
			if(data == NULL)
				return nil;
			NSString * output = [NSString stringWithUTF8String: data];
			free(data);
			return output;
		}
    }
	
	return nil;
}
		   
- (void) dealloc
{
	MDLCleanup(nbElem, status, statusCache, todoList, cache);
	free(IDToPosition);
	[super dealloc];
}

- (uint) getNbElem : (BOOL) considerDiscarded
{
	return considerDiscarded ? discardedCount : nbElem;
}

- (DATA_LOADED **) getData : (uint) row : (BOOL) considerDiscarded
{
	if(row >= (considerDiscarded ? discardedCount : nbElem))
		return NULL;
	
	return &(*todoList)[considerDiscarded ? IDToPosition[row] : row];
}

- (int8_t) statusOfID : (uint) row : (BOOL) considerDiscarded
{
	if(row >= (considerDiscarded ? discardedCount : nbElem) || status == NULL)
		return MDL_CODE_INTERNAL_ERROR;
	
	if(considerDiscarded)
		row = IDToPosition[row];
	
	if(status[row] != NULL)
		return *(status[row]);
	
	return MDL_CODE_INTERNAL_ERROR;
}

- (void) addElement : (MANGAS_DATA) data : (BOOL) isTome : (int) element : (BOOL) partOfBatch
{
	if (element == VALEUR_FIN_STRUCTURE_CHAPITRE)
		return;
	
	uint pos;
	for (pos = 0; pos < sizeCache && cache[pos].cacheDBID != data.cacheDBID ; pos++);
	
	if(pos == sizeCache || cache[pos].cacheDBID != data.cacheDBID)
	{
		//We need to refresh
	}
	
	if(!nbElem || !MDLisThereCollision(data, isTome, element, *todoList, *status, nbElem))
	{
		int newChunkSize;
		DATA_LOADED ** newElement = MDLCreateElement(&cache[pos], isTome, element, &newChunkSize);
		
		if(newElement == NULL || newChunkSize == 0)
		{
			free(newElement);
			return;
		}
		
		uint newSize = nbElem + newChunkSize;
		int8_t **newStatus = realloc(status, newSize * sizeof(int8_t*)), **newStatusCache = realloc(statusCache, newSize * sizeof(int8_t*));
		uint *newIDToPosition = realloc(IDToPosition, (discardedCount + newChunkSize) * sizeof(uint));
		
		//Even if one of them failed, we need to update the pointer of the other
		if(newStatus == NULL || newStatusCache == NULL || newIDToPosition == NULL)
		{
			if(newStatus != NULL)
				status = newStatus;
			
			if(newStatusCache != NULL)
				statusCache = newStatusCache;
			
			if(newIDToPosition != NULL)
				IDToPosition = newIDToPosition;
			
			return;
		}
		
		status = newStatus;
		statusCache = newStatusCache;
		IDToPosition = newIDToPosition;
		
		//Increase the size of the status buffer
		for (uint maxSize = nbElem + newChunkSize; nbElem < maxSize; nbElem++)
		{
			status[nbElem] = malloc(sizeof(int8_t));
			statusCache[nbElem] = malloc(sizeof(int8_t));
			
			if(status[nbElem] == NULL || statusCache[nbElem] == NULL)
			{
				uint i = nbElem - (maxSize - newChunkSize) + 1;
				nbElem = maxSize = newChunkSize;
				while (i > 0)
				{
					free(status[nbElem + --i]);
					free(statusCache[nbElem + i]);
				}
				
				return;
			}
			
			*status[nbElem] = *statusCache[nbElem] = MDL_CODE_DEFAULT;
			IDToPosition[discardedCount] = discardedCount;	discardedCount++;
		}
		
		int curPos = nbElem - 1;
		DATA_LOADED ** newTodoList = realloc(*todoList, nbElem * sizeof(DATA_LOADED *));
		
		if(newTodoList == NULL)
		{
			for (uint limit = nbElem - newChunkSize; limit < nbElem;)
			{
				free(status[--nbElem]);
				free(statusCache[nbElem]);
			}
		}
		
		*todoList = MDLInjectElementIntoMainList(newTodoList, &nbElem, &curPos, newElement, newChunkSize);
	}
	
	if(!partOfBatch && nbElem)
	{
		//Great, the injection is now over... We need to reanimate what needs to be
		if(!isThreadStillRunning(coreWorker))
		{
			startMDL(NULL, cache, &coreWorker, &todoList, &status, &statusCache, &nbElem, &quit, self);
		}
		
		//Worker should be at work, now, let's wake the UI up
		[_tabMDL wakeUp];
	}
}

- (BOOL) checkForCollision : (MANGAS_DATA) data : (BOOL) isTome : (int) element
{
	return nbElem && MDLisThereCollision(data, isTome, element, *todoList, *status, nbElem);
}

- (void) discardElement : (uint) element
{
	discardedCount--;	//If decrement sent in the if, won't work for last element

	if(element <= discardedCount)
	{
		for (uint posDiscarded = element; posDiscarded < discardedCount; posDiscarded++)
		{
			IDToPosition[posDiscarded] = IDToPosition[posDiscarded+1];
		}
	}
}

- (void) setStatusOfID : (uint) row : (BOOL) considerDiscarded : (uint8_t) value
{
	if(row >= (considerDiscarded ? discardedCount : nbElem) || status == NULL)
		return;
	
	if(considerDiscarded)
		row = IDToPosition[row];
	
	if(status[row] != NULL)
		*(status[row]) = value;
}

- (void) refreshCT : (uint) row
{
	DATA_LOADED ** data = [self getData:row :YES];
	
	if(data != NULL && *data != NULL)
	{
		for(NSView* view in _tabMDL.superview.subviews)
		{
			if([view class] == [CTSelec class])
			{
				[(CTSelec *) view refreshCT:YES :(*data)->datas->cacheDBID];
				break;
			}
		}
	}
}

@end
