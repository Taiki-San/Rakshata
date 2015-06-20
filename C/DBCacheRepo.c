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

#include "dbCache.h"

void getRidOfDuplicateInRepo(REPO_DATA ** data, uint nombreRepo)
{
	//On va chercher des collisions
	for(uint posBase = 0; posBase < nombreRepo; posBase++)	//On test avec jusqu'à nombreRepo - 1 mais la boucle interne s'occupera de nous faire dégager donc pas la peine d'aouter ce calcul à cette condition
	{
		if(data[posBase] == NULL)	//On peut avoir des trous au milieu de la chaîne
			continue;
		
		for(uint posToCompareWith = posBase + 1; posToCompareWith < nombreRepo; posToCompareWith++)
		{
			if(data[posToCompareWith] == NULL)
				continue;
			
			if(data[posBase]->parentRepoID == data[posToCompareWith]->parentRepoID && data[posBase]->repoID == data[posToCompareWith]->repoID)
			{
				free(data[posToCompareWith]);
				data[posToCompareWith] = NULL;
			}
		}
	}
}

void insertRootRepoCache(ROOT_REPO_DATA ** newRoot, uint newRootEntries)
{
	if(newRoot == NULL || newRootEntries == 0)
		return;
	
	uint lengthRepoCopy = lengthRootRepo, newLengthRepo = lengthRepoCopy, baseNewRoot = lengthRepoCopy;
	
	//We cautiously insert the new entries in the root store
	//calloc important, otherwise, we have to set last entries to NULL
	ROOT_REPO_DATA ** newReceiver = calloc(lengthRepoCopy + newRootEntries + 1, sizeof(ROOT_REPO_DATA*));
	if(newReceiver == NULL)
		return;
	
	memcpy(newReceiver, rootRepoList, lengthRepoCopy * sizeof(ROOT_REPO_DATA *));
	
	for(uint count = 0; count < newRootEntries; count++, newLengthRepo++)
	{
		if(newRoot[count] != NULL)
			newReceiver[newLengthRepo] = newRoot[count];
		else
			newLengthRepo--;
	}
	
	//Remove collisions in the case there might be
	//Also update the state of existing repo to reflect the one submitted there
	REPO_DATA ** updatedRepo = NULL;
	uint lengthUpdated = 0;
	
	getRideOfDuplicateInRootRepo(newReceiver, newLengthRepo, &updatedRepo, &lengthUpdated);
	
	//We compact the list
	uint base = 0;
	for(uint carry = 1; base < newLengthRepo; base++)
	{
		if(newReceiver[base] == NULL)
		{
			if(carry <= base)
				carry = base + 1;
			
			while(carry < newLengthRepo && newReceiver[carry] == NULL)
				carry++;
			
			if(carry < newLengthRepo)
			{
				//If we move the base of the new elements
				if(carry == baseNewRoot)
					baseNewRoot = base;
					
				newReceiver[base] = newReceiver[carry];
				newReceiver[carry] = NULL;
			}
			else
				break;
		}
	}

	//If we didn't use some entries, we release them
	if(base < lengthRepoCopy + newRootEntries)
	{
		newLengthRepo = base;
		void * tmp = realloc(newReceiver, (newLengthRepo + 1) * sizeof(ROOT_REPO_DATA *));
		if(tmp != NULL)
			newReceiver = tmp;
	}
	
	//We set the repoID of our new roots
	if(baseNewRoot > 0 && baseNewRoot != newLengthRepo)
	{
		//We get our used ID
		uint usedID[baseNewRoot];
		for(uint i = 0; i < baseNewRoot; i++)
		{
			if(newReceiver[i] != NULL)
				usedID[i] = newReceiver[i]->repoID;
			else
				usedID[i] = UINT_MAX;
		}
		
		//We sort them
		qsort(usedID, baseNewRoot, sizeof(uint), sortNumbers);
		
		//We now picks our IDs
		for(uint pos = baseNewRoot, currentID = 1, currentPosUsed = 0; pos < newLengthRepo;)
		{
			if(currentPosUsed < baseNewRoot)
			{
				if(currentID == usedID[currentPosUsed])
				{
					currentID++;
					currentPosUsed++;
					continue;
				}
				else if(currentID > usedID[currentPosUsed])
				{
					currentPosUsed++;
					continue;
				}
			}
			
			//We insert the repoID in our children
			if(newReceiver[pos]->nombreSubrepo > 0 && newReceiver[pos]->subRepo != NULL)
			{
				for (uint i = 0; i < newReceiver[pos]->nombreSubrepo; i++)
					newReceiver[pos]->subRepo[i].parentRepoID = currentID;
			}
			
			//We set the current root repo
			newReceiver[pos++]->repoID = currentID++;
		}
	}
	else	//There was no root earlier, let's insert ours from the begining
	{
		for(uint i = baseNewRoot; i < newLengthRepo; i++)
			newReceiver[i]->repoID = i + 1;
	}
	
	//Actual update
	MUTEX_LOCK(cacheMutex);
	
	void * buf = rootRepoList;
	rootRepoList = newReceiver;
	free(buf);
	lengthRootRepo = newLengthRepo;
	
	MUTEX_UNLOCK(cacheMutex);
	
	if(newLengthRepo <= baseNewRoot)
		return;

	newRootEntries = newLengthRepo - baseNewRoot;
	
	//We now update the standard repo store
	ROOT_REPO_DATA * element;
	REPO_DATA ** tmpRepo[newRootEntries];
	uint repoSize[newRootEntries], cumulativeSize = 0;
	
	//We extract all the active repo
	for(uint posRoot = baseNewRoot, posInOut = 0; posRoot < newLengthRepo; posRoot++, posInOut++)
	{
		element = newReceiver[posRoot];
		tmpRepo[posInOut] = NULL;
		repoSize[posInOut] = 0;
		
		if(element == NULL)
			continue;
		
		//We get the size of the chunk to insert
		for(uint posSub = 0; posSub < element->nombreSubrepo; posSub++)
		{
			if(element->subRepo[posSub].active)
				repoSize[posInOut]++;
		}

		//Empty chunk
		if(repoSize[posInOut] == 0)
			continue;
		
		//We allocate the array to receive the new REPO_DATA
		tmpRepo[posInOut] = calloc(repoSize[posInOut], sizeof(REPO_DATA *));
		if(tmpRepo[posInOut] == NULL)
		{
			repoSize[posInOut] = 0;
			continue;
		}
		
		for(uint posSub = 0; posSub < repoSize[posInOut]; posSub++)
		{
			tmpRepo[posInOut][posSub] = malloc(sizeof(REPO_DATA));
			if(tmpRepo[posInOut] == NULL)
			{
				while(posSub-- > 0)
					free(tmpRepo[posInOut][posSub]);
				
				free(tmpRepo[posInOut]);
				tmpRepo[posInOut] = NULL;
				repoSize[posInOut] = 0;
				
				continue;
			}
		}
		
		//the REPO_DATA structure is thankfully static, so copying it is trivial
		for(uint posSub = 0, posOut = 0; posSub < element->nombreSubrepo && posOut < repoSize[posInOut]; posSub++)
		{
			if(element->subRepo[posSub].active)
			{
				*tmpRepo[posInOut][posOut++] = element->subRepo[posSub];
				cumulativeSize++;	//We may theorically risk an overflow here, but come on, 4B repo? It'd take ages to refresh and it'll break elsewhere earlier
			}
		}
	}
	
	//We will finally start messing with the shared structure
	MUTEX_LOCK(cacheMutex);
	
	cumulativeSize += lengthRepo;

	REPO_DATA ** mainList = realloc(repoList, (cumulativeSize + lengthUpdated + 1) * sizeof(REPO_DATA *));
	if(mainList == NULL)
	{
		for(uint posRoot = 0; posRoot < newRootEntries; posRoot++)
		{
			for(uint posSub = 0; posSub < repoSize[posRoot]; free(tmpRepo[posRoot][posSub++]));
			free(tmpRepo[posRoot]);
		}
		
		free(updatedRepo);
		
		MUTEX_UNLOCK(cacheMutex);
		return;
	}
	
	uint currentRoot = 0, currentSub = 0;
	while(lengthRepo < cumulativeSize && currentRoot < newRootEntries)
	{
		if(currentSub < repoSize[currentRoot])
			mainList[lengthRepo++] = tmpRepo[currentRoot][currentSub++];
		else
		{
			free(tmpRepo[currentRoot++]);
			currentSub = 0;
		}
	}
	
	//We now reflect the changes we were notified by getRideOfDuplicateInRootRepo (lengthUpdated - 0 if none)
	//It can only mean activated repo
	for(uint i = 0; i < lengthUpdated; i++)
		mainList[lengthRepo++] = updatedRepo[i];
	
	free(updatedRepo);
	for(; currentRoot < newRootEntries; free(tmpRepo[currentRoot++]));
	
	mainList[lengthRepo] = NULL;
	repoList = mainList;
	MUTEX_UNLOCK(cacheMutex);
}

void updateRootRepoCache(ROOT_REPO_DATA ** repoData)
{
	if(repoData == NULL)
		return;
	
	MUTEX_LOCK(cacheMutex);

	//We update the root store
	for(uint i = 0, posInMain = 0; i < lengthRootRepo; i++)
	{
		//We find the previous entry
		if(rootRepoList[posInMain]->repoID != repoData[i]->repoID)
		{
			for(; posInMain < lengthRootRepo && rootRepoList[posInMain]->repoID != repoData[i]->repoID; posInMain++);
			if(posInMain >= lengthRootRepo)
			{
				for(posInMain = 0; posInMain < lengthRootRepo && rootRepoList[posInMain]->repoID != repoData[i]->repoID; posInMain++);
				if(posInMain >= lengthRootRepo)
				{
					posInMain = 0;
					continue;
				}
			}
		}
		
		_freeSingleRootRepo(rootRepoList[posInMain], false);
		*(rootRepoList[posInMain]) = *(repoData[i]);
	}
	
	MUTEX_UNLOCK(cacheMutex);
	
	//We updated the root store, we now have to update the repo store
	//From then on, we can fail without significant issues, as this list will get rebuild at next launch
	
	for(uint64_t pos = 0, prevParent = 0, prevChild = 0, subChildCount; pos < lengthRepo; pos++)
	{
		//Look for the begining of the root sequence
		if(prevParent >= lengthRootRepo || repoList[pos]->parentRepoID != rootRepoList[prevParent]->repoID)
		{
			for(; prevParent < lengthRootRepo && rootRepoList[prevParent]->repoID != repoList[pos]->parentRepoID; prevParent++);
			if(prevParent == lengthRootRepo)
			{
				for (prevParent = 0; prevParent < lengthRootRepo && rootRepoList[prevParent]->repoID != repoList[pos]->parentRepoID; prevParent++);
				if(prevParent == lengthRootRepo)
					continue;
			}
		}
		
		subChildCount = rootRepoList[prevParent]->nombreSubrepo;
		
		//Find our specific repo
		for(prevChild++; prevChild < subChildCount && rootRepoList[prevParent]->subRepo[prevChild].repoID != repoList[pos]->repoID; prevChild++);
		if(prevChild == subChildCount)
		{
			for(prevChild = 0; prevChild < subChildCount && rootRepoList[prevParent]->subRepo[prevChild].repoID != repoList[pos]->repoID; prevChild++);
			if(prevChild == subChildCount)
				continue;
		}
		
		memcpy(repoList[pos], &(rootRepoList[prevParent]->subRepo[prevChild]), sizeof(REPO_DATA));
	}
}

void removeNonInstalledSubRepo(REPO_DATA ** _subRepo, uint nbSubRepo, bool haveExtra)
{
	if(_subRepo == NULL || *_subRepo == NULL || nbSubRepo == 0)
		return;
	
	REPO_DATA * subRepo = *_subRepo;
	uint parentID, validatedCount = 0;
	bool validated[nbSubRepo];
	
	memset(validated, 0, sizeof(validated));
	
	if(haveExtra)
		parentID = ((REPO_DATA_EXTRA *) subRepo)[0].data->parentRepoID;
	else
		parentID = subRepo[0].parentRepoID;
	
	for(uint pos = 0; pos < lengthRepo; pos++)
	{
		if(repoList[pos] != NULL && repoList[pos]->parentRepoID == parentID)
		{
			for(uint posSub = 0, currentID; posSub < nbSubRepo; posSub++)
			{
				currentID = haveExtra ? ((REPO_DATA_EXTRA *) subRepo)[0].data->repoID : subRepo[posSub].repoID;
				if(currentID == repoList[pos]->repoID && !validated[posSub])
				{
					validated[posSub] = true;
					validatedCount++;
					break;
				}
			}
		}
	}
	
	if(validatedCount != nbSubRepo)
	{
		for(uint pos = 0; pos < nbSubRepo; pos++)
		{
			if(!validated[pos] && repoList[pos] != NULL)
				repoList[pos]->active = false;
		}
	}
}

void getRideOfDuplicateInRootRepo(ROOT_REPO_DATA ** data, uint nombreRepo, REPO_DATA *** wantUpdatedRepo, uint *lengthUpdated)
{
	bool wantUpdated = (wantUpdatedRepo != NULL && lengthUpdated != NULL);
	
	if(wantUpdated)
		*lengthUpdated = 0;
	
	//On va chercher des collisions
	for(uint posBase = 0; posBase < nombreRepo; posBase++)	//On test avec jusqu'à nombreRepo - 1 mais la boucle interne s'occupera de nous faire dégager donc pas la peine d'aouter ce calcul à cette condition
	{
		if(data[posBase] == NULL)	//On peut avoir des trous au milieu de la chaîne
			continue;
		
		for(uint posToCompareWith = posBase + 1; posToCompareWith < nombreRepo; posToCompareWith++)
		{
			if(data[posToCompareWith] == NULL)
				continue;
			
			//If the item already have an ID
			if(data[posToCompareWith]->repoID != 0 && data[posBase]->repoID != data[posToCompareWith]->repoID)
				continue;
			//If the mean to access the repo is different
			else if(data[posBase]->type != data[posToCompareWith]->type || strcmp(data[posBase]->URL, data[posToCompareWith]->URL))
				continue;
			
			//Merge the active state if needed
			if(wantUpdated)
			{
				if(data[posBase]->nombreSubrepo == data[posToCompareWith]->nombreSubrepo)
				{
					for(uint i = 0, nbSub = data[posBase]->nombreSubrepo; i < nbSub; i++)
					{
						//State update
						if(data[posBase]->subRepo[i].active == false && data[posToCompareWith]->subRepo[i].active)
						{
							data[posBase]->subRepo[i].active = data[posToCompareWith]->subRepo[i].active;
							
							//We check we don't already have inserted it
							uint posUpdated = 0;
							
							for(; posUpdated < *lengthUpdated; posUpdated++)
							{
								if(getRepoID((*wantUpdatedRepo)[posUpdated]) == getRepoID(&(data[posBase]->subRepo[i])))
									break;
							}
							
							//Nop, first time seeing you there
							if(posUpdated == *lengthUpdated)
							{
								REPO_DATA * newSlot = malloc(sizeof(REPO_DATA));
								
								if(newSlot != NULL)
								{
									void * tmp = realloc(*wantUpdatedRepo, (*lengthUpdated + 1) * sizeof(REPO_DATA *));
									if(tmp != NULL)
									{
										*lengthUpdated += 1;
										
										*newSlot = data[posBase]->subRepo[i];
										*wantUpdatedRepo = tmp;
										(*wantUpdatedRepo)[posUpdated] = newSlot;
									}
									else
										free(newSlot);
								}
							}
						}
					}
				}
			}

			freeSingleRootRepo(data[posToCompareWith]);
			data[posToCompareWith] = NULL;
		}
	}
}

bool isAppropriateNumberOfRepo(uint requestedNumber)
{
	return requestedNumber == lengthRepo;
}

uint getNumberInstalledProjectForRepo(bool isRoot, void * repo)
{
	uint output = 0;
	sqlite3_stmt * request = NULL;
	
	MUTEX_LOCK(cacheMutex);
	
	if(createRequest(cache, "SELECT COUNT() FROM "MAIN_CACHE" WHERE "DBNAMETOID(RDB_repo)" = ?1 AND "DBNAMETOID(RDB_isInstalled)" = 1", &request) == SQLITE_OK)
	{
		if(isRoot)
		{
			ROOT_REPO_DATA * root = repo;
			
			if(root->subRepo != NULL)
			{
				for(uint i = 0; i < root->nombreSubrepo; i++)
				{
					if(!root->subRepo[i].active)
						continue;
					
					sqlite3_bind_int64(request, 1, (int64_t) getRepoID(&(root->subRepo[i])));
					if(sqlite3_step(request) == SQLITE_ROW)
					{
						uint newValue = (uint32_t) sqlite3_column_int(request, 0);
						if(newValue + output < newValue)
						{
							output = UINT_MAX;
							break;
						}
						else
							output += newValue;
					}
					
					sqlite3_reset(request);
				}
			}
		}
		else
		{
			sqlite3_bind_int64(request, 1, (int64_t) getRepoID(repo));
			if(sqlite3_step(request) == SQLITE_ROW)
			{
				output = (uint32_t) sqlite3_column_int(request, 0);
			}
		}
		
		destroyRequest(request);
	}
	
	MUTEX_UNLOCK(cacheMutex);
	
	return output;
}