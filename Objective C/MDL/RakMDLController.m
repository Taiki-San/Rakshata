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

		if(startMDL(cache, &coreWorker, &todoList, &status, &statusCache, &nbElem, &quit, self) == false)
		{
			[self release];			self = nil;
		}
	}
	
	return self;
}

- (void) needToQuit
{
	quit = true;
	MDLQuit();
	sleep(2);
}

- (uint) getNbElem
{
	return nbElem;
}

- (DATA_LOADED **) getData : (uint) row
{
	if(row >= nbElem)
		return NULL;
	
	return &(*todoList)[row];
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
