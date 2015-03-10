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

uint maxRootID = 0;

//La sélection des repo MaJ se fera avant l'appel à cette fonction
ROOT_REPO_DATA ** loadRootRepo(char * repoDB, uint *nbRepo)
{
	size_t length = strlen(repoDB);
	
	if(length > 1 && repoDB[length - 1] == '\n')	length--;
	
	char * decoded = (char*) base64_decode(repoDB, length, &length);
	
	ROOT_REPO_DATA ** output = parseLocalRepo(decoded, nbRepo);
	
	free(decoded);
	
	return output;
}

REPO_DATA ** loadRepo(ROOT_REPO_DATA ** root, uint nbRoot, uint * nbRepo)
{
	if(root == NULL || nbRepo == NULL || nbRoot == 0)
		return NULL;
	
	uint nbSubRepo = 0;
	
	for(uint pos = 0; pos < nbRoot; pos++)
	{
		if(root[pos] != NULL)
			nbSubRepo += root[pos]->nombreSubrepo;
	}
	
	REPO_DATA ** output = calloc(nbSubRepo + 1, sizeof(REPO_DATA *));
	if(output != NULL)
	{
		for(uint pos = 0, indexRoot = 0, posInRoot = 0; pos < nbSubRepo && indexRoot < nbRoot && root[indexRoot] != NULL; pos++)
		{
			if(posInRoot >= root[indexRoot]->nombreSubrepo)
			{
				indexRoot++;
				posInRoot = 0;
			}
			
			output[pos] = malloc(sizeof(REPO_DATA));
			if(output[pos] == NULL)
			{
				while (pos-- > 0)
					free(output[pos]);
				free(output);
				return NULL;
			}
			
			if(indexRoot < nbRoot && root[indexRoot] != NULL)
				*output[pos] = root[indexRoot]->subRepo[posInRoot++];
		}
		
		if(nbSubRepo == 0)
		{
			free(output);
			return NULL;
		}
		
		*nbRepo = nbSubRepo;
	}
	
	return output;
}

uint getFreeRootRepoID()
{
	if(rootRepoList == NULL)
		return ++maxRootID;
	
	uint i;
	while(1)
	{
		++maxRootID;
		
		for(i = 0; i < lengthRootRepo; i++)
		{
			if(rootRepoList[i]->repoID == maxRootID)
				break;
		}
		
		if(i == lengthRootRepo)
			return maxRootID;
	}
}

