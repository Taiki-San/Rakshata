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
 *********************************************************************************************/

#include "dbCache.h"

bool * getInstalledFromData(PROJECT_DATA * data, uint sizeData)
{
	if(data == NULL || sizeData == 0)
		return NULL;
	
	bool * output = calloc(sizeData, sizeof(bool));
	
	if(output != NULL)
	{
		bool canUseOptimization = true;			//Can we assume cacheDBID is sorted?
		uint pos = 0, ID = 0;
		
		while(pos < sizeData)
		{
			if(data[pos].cacheDBID < ID)		//There are holes, but numbers are still growing
				ID++;
			
			else if(data[pos].cacheDBID == ID)	//Standard case
			{
				ID++;	//We assume there is no cacheDBID duplicate
				pos++;
			}
			
			else								//We're not sorted
			{
				canUseOptimization = false;
				break;
			}
		}
		
		
		sqlite3_stmt* request = createRequest(cache, "SELECT * FROM "MAIN_CACHE" WHERE "DBNAMETOID(RDB_isInstalled)" = 1 ORDER BY "DBNAMETOID(RDB_ID)" ASC");
		
		//The SQL request gives us the list of all installed project, and we try to match them to the list we were passed.
		//If we find a hit, we set the bool in output to true
		//By using calloc, the default value is false
		while(sqlite3_step(request) == SQLITE_ROW)
		{
			const uint32_t installedID = (uint32_t) sqlite3_column_int(request, RDB_ID-1);
			
			//As ID are sorted, and the used ORDER BY in the request, the whole parsing only need to iterate once.
			//If something is missing, it's not far after and it's not before
			//O(n) FTW!
			if(canUseOptimization)
			{
				while(pos < sizeData && data[pos].cacheDBID < installedID)
					pos++;
				
				//We hit the end of our working set, not point going farther away
				if(pos >= sizeData)
					break;
				
				if(data[pos].cacheDBID == installedID)
					output[pos++] = true;
				
				//Were we to add to after this for the initial if(), we would need to add else break;
				//	as this would mean the entry we hit doesn't exist in our working set
			}

			//We must iterate the whole array every time as it's not sorted
			//It's probably worth sorting a copy before but this would require a significant overhead
			//	so, we cope with O(n^2) for now
			else
			{
				for(pos = 0; pos < sizeData && data[pos].cacheDBID != installedID; pos++);
				
				if(pos < sizeData && data[pos].cacheDBID == installedID)
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
	
	sqlite3_stmt* request = createRequest(cache, "SELECT "DBNAMETOID(RDB_isInstalled)" FROM "MAIN_CACHE" WHERE "DBNAMETOID(RDB_ID)" = ?1");

	if(cache != NULL)
	{
		sqlite3_bind_int(request, 1, (int32_t) ID);
		
		if(sqlite3_step(request) == SQLITE_ROW && sqlite3_column_int(request, 0) != 0)
			output = true;
		
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
				for(uint posRepo = 0, nbEntry = rootRepoList[i]->nbSubrepo; posRepo < nbEntry; posRepo++)
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
		
		notifyUpdateRepo(getEmptyRepoWithID(repoID));
	}

	destroyRequest(request);
}
