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

bool * getInstalledFromData(PROJECT_DATA * data, uint sizeData)
{
	if(data == NULL || sizeData == 0)
		return NULL;
	
	bool * output = calloc(sizeData, sizeof(bool));
	
	if(output != NULL)
	{
		bool canUseOptimization = true;		//Can we assume cacheDBID is sorted?
		uint pos = 0, ID = 0;
		
		while(pos < sizeData)
		{
			if(data[pos].cacheDBID < ID)	//There is holes, but numbers are still growing
				ID++;
			
			else if(data[pos].cacheDBID == ID)	//Standard case
				pos++;
			
			else							//We're not sorted
			{
				canUseOptimization = false;
				break;
			}
		}
		
		
		sqlite3_stmt* request = createRequest(cache, "SELECT * FROM "MAIN_CACHE" WHERE "DBNAMETOID(RDB_isInstalled)" = 1 ORDER BY "DBNAMETOID(RDB_ID)" ASC");
		
		while(sqlite3_step(request) == SQLITE_ROW)
		{
			if(canUseOptimization)
			{
				while(pos < nbElemInCache && data[pos].cacheDBID < (uint32_t) sqlite3_column_int(request, RDB_ID-1))
					pos++;
				
				if(data[pos].cacheDBID == (uint32_t) sqlite3_column_int(request, RDB_ID-1))
					output[pos++] = true;
				
				else if(pos < nbElemInCache)		//Élément supprimé
					continue;
				
				else
					break;
			}
			else
			{
				for(pos = 0; pos < nbElemInCache && data[pos].cacheDBID != (uint32_t) sqlite3_column_int(request, RDB_ID-1); pos++);
				
				if(data[pos].cacheDBID == (uint32_t) sqlite3_column_int(request, RDB_ID-1))
					output[pos] = true;
			}
		}
		
		destroyRequest(request);
	}
	
	return output;
}

bool isProjectInstalledInCache (uint ID)
{
	bool output = false;
	
	sqlite3_stmt* request = createRequest(cache, "SELECT * FROM "MAIN_CACHE" WHERE "DBNAMETOID(RDB_ID)" = ?1");

	if(cache != NULL)
	{
		sqlite3_bind_int(request, 1, (int32_t) ID);
		
		if(sqlite3_step(request) == SQLITE_ROW)
		{
			if(sqlite3_column_int(request, RDB_isInstalled-1))
				output = true;
		}
			
		destroyRequest(request);
	}
	
	return output;
}

void setInstalled(uint cacheID)
{
	if(cache == NULL)
		setupBDDCache();
	
	sqlite3_stmt * request = createRequest(cache, "UPDATE "MAIN_CACHE" SET "DBNAMETOID(RDB_isInstalled)" = 1 WHERE "DBNAMETOID(RDB_ID)" = ?1");
	if(request != NULL)
	{
		sqlite3_bind_int(request, 1, (int32_t) cacheID);
		sqlite3_step(request);
		destroyRequest(request);
	}
}

void setUninstalled(bool isRoot, uint64_t repoID)
{
	sqlite3_stmt * request = createRequest(cache, "UPDATE "MAIN_CACHE" SET "DBNAMETOID(RDB_isInstalled)" = 0 WHERE "DBNAMETOID(RDB_repo)" = ?1");
	if(request == NULL)
		return;
	
	if(isRoot)
	{
		for(uint i = 0; i < lengthRootRepo; i++)
		{
			if(rootRepoList[i] != NULL && rootRepoList[i]->repoID == repoID)
			{
				for(uint posRepo = 0, nbEntry = rootRepoList[i]->nombreSubrepo; posRepo < nbEntry; posRepo++)
				{
					REPO_DATA repo = rootRepoList[i]->subRepo[posRepo];
					
					if(repo.active)
					{
						sqlite3_bind_int64(request, 1, (int64_t) getRepoID(&repo));
						sqlite3_step(request);
						sqlite3_reset(request);
						
						notifyUpdateRepo(repo);
					}
				}
				
				notifyUpdateRootRepo(*rootRepoList[i]);
				break;
			}
		}
	}
	else
	{
		sqlite3_bind_int64(request, 1, (int64_t) repoID);
		sqlite3_step(request);
		
		REPO_DATA emptyRepo = getEmptyRepo();	emptyRepo.repoID = getSubrepoFromRepoID(repoID);	emptyRepo.parentRepoID = getRootFromRepoID(repoID);
		notifyUpdateRepo(emptyRepo);
	}

	destroyRequest(request);
}
