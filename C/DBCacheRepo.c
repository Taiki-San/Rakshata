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

void updateRepoCache(REPO_DATA ** repoData, uint newAmountOfRepo)
{
	uint lengthRepoCopy = lengthRepo;
	
	REPO_DATA ** newReceiver;
	
	if(newAmountOfRepo == -1 || newAmountOfRepo == lengthRepoCopy)
	{
		newReceiver = repoList;
	}
	else	//Resize repoList
	{
		newReceiver = calloc(lengthRepoCopy + 1, sizeof(REPO_DATA*));	//calloc important, otherwise, we have to set last entries to NULL
		if(newReceiver == NULL)
			return;
		
		memcpy(newReceiver, repoList, lengthRepoCopy);
		lengthRepoCopy = newAmountOfRepo;
	}
	
	for(int pos = 0; pos < lengthRepoCopy; pos++)
	{
		if(newReceiver[pos] != NULL && repoData[pos] != NULL)
		{
			memcpy(newReceiver[pos], repoData[pos], sizeof(REPO_DATA));
			free(repoData[pos]);
			repoData[pos] = NULL;
		}
		else if(repoData[pos] != NULL)
		{
			newReceiver[pos] = repoData[pos];
		}
	}
	
	getRidOfDuplicateInRepo(repoData, lengthRepoCopy);
	if(repoList != newReceiver)
	{
		void * buf = repoList;
		repoList = newReceiver;
		free(buf);
		lengthRepo = lengthRepoCopy;
	}
}

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

//Be carefull, you can't add repo using this method, only existing repo will be updated with new root data
void updateRootRepoCache(ROOT_REPO_DATA ** repoData, const uint newAmountOfRepo)
{
	uint lengthRepoCopy = lengthRootRepo;
	
	ROOT_REPO_DATA ** newReceiver;
	
	if(newAmountOfRepo != -1)	//Resize teamList
	{
		newReceiver = calloc(lengthRepoCopy + newAmountOfRepo + 1, sizeof(ROOT_REPO_DATA*));	//calloc important, otherwise, we have to set last entries to NULL
		if(newReceiver == NULL)
			return;
		
		memcpy(newReceiver, rootRepoList, lengthRepoCopy);
		
		for(uint count = 0; count < newAmountOfRepo; count++, lengthRepoCopy++)
		{
			if(newReceiver[lengthRepoCopy] != NULL && repoData[lengthRepoCopy] != NULL)
			{
				memcpy(newReceiver[lengthRepoCopy], repoData[lengthRepoCopy], sizeof(ROOT_REPO_DATA));
				free(repoData[lengthRepoCopy]);
			}
			else if(repoData[lengthRepoCopy] != NULL)
			{
				newReceiver[lengthRepoCopy] = repoData[lengthRepoCopy];
			}
			else
				lengthRepoCopy--;
		}
	}
	else
		newReceiver = rootRepoList;
	
	getRideOfDuplicateInRootRepo(newReceiver, lengthRepoCopy);
	if(rootRepoList != newReceiver)
	{
		void * buf = rootRepoList;
		rootRepoList = newReceiver;
		free(buf);
		lengthRootRepo = lengthRepoCopy;
	}
	
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
		
		//Find our specefic repo
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

void removeNonInstalledSubRepo(REPO_DATA ** _subRepo, uint * nbSubRepo, bool haveExtra)
{
	if(_subRepo == NULL || *_subRepo == NULL || nbSubRepo == NULL || *nbSubRepo == 0)
		return;
	
	REPO_DATA * subRepo = *_subRepo;
	uint parentID, validatedCount = 0;
	bool validated[*nbSubRepo];
	
	memset(validated, 0, sizeof(validated));
	
	if(haveExtra)
		parentID = ((REPO_DATA_EXTRA *) subRepo)[0].data->parentRepoID;
	else
		parentID = subRepo[0].parentRepoID;
	
	for(uint pos = 0; pos < lengthRepo; pos++)
	{
		if(repoList[pos] != NULL && repoList[pos]->parentRepoID == parentID)
		{
			for(uint posSub = 0, currentID; posSub < *nbSubRepo; posSub++)
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
	
	if(validatedCount != *nbSubRepo)
	{
		if(validatedCount == 0)
			*_subRepo = NULL;
		else
		{
			REPO_DATA * newSubrepo = calloc(validatedCount, sizeof(REPO_DATA));
			
			for(uint pos = 0, posValidated = 0; pos < *nbSubRepo; pos++)
			{
				if(validated[pos])
					newSubrepo[posValidated++] = subRepo[pos];
			}
			
			*_subRepo = newSubrepo;
			free(subRepo);
		}
		
		*nbSubRepo = validatedCount;
	}
}

void getRideOfDuplicateInRootRepo(ROOT_REPO_DATA ** data, uint nombreRepo)
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
			
			if(data[posBase]->repoID == data[posToCompareWith]->repoID)
			{
				free(data[posToCompareWith]);
				data[posToCompareWith] = NULL;
			}
		}
	}
}

bool isAppropriateNumberOfRepo(uint requestedNumber)
{
	return requestedNumber == lengthRepo;
}
