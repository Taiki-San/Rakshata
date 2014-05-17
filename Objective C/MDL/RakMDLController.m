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

@implementation RakMDLController

- (id) init
{
	self = [super init];
	
	if(self != nil)
	{
		quit = false;
		
		cache = getCopyCache(RDB_LOADALL | SORT_NAME | RDB_CTXMDL, NULL);

		if(startMDL(cache, &coreWorker, &todoList, &status, &statusCache, &nbElem, &quit, self))
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
	{
		MDLQuit();
	}
}
		   
- (void) dealloc
{
	MDLCleanup(nbElem, status, statusCache, todoList, cache);
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

- (void) discardElement : (uint) element
{
	if(element < discardedCount)
	{
		uint posDiscarded = IDToPosition[element];
		
		if(posDiscarded < discardedCount - 1)
			memcpy(&IDToPosition[posDiscarded], &IDToPosition[posDiscarded+1], nbElem - posDiscarded);
		discardedCount--;
		
	}
}

- (int8_t) statusOfID : (uint) row
{
	if(row < nbElem && status != NULL && status[row] != NULL)
		return *(status[row]);
	
	return MDL_CODE_INTERNAL_ERROR;
}

- (void) setStatusOfID : (uint) row : (uint8_t) value
{
	if(row < nbElem && status != NULL && status[row] != NULL)
		*(status[row]) = value;
}

@end
