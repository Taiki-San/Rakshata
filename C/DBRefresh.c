/*********************************************************************************************
**	__________         __           .__            __                 ________   _______   	**
**	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
**	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
**	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
**	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
**	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
**                                                                                          **
**		Source code and assets are property of Taiki, distribution is stricly forbidden		**
**                                                                                          **
*********************************************************************************************/

#include "dbCache.h"

void updateDatabase(bool forced)
{
    if(!checkNetworkState(CONNEXION_DOWN) && (forced || time(NULL) - alreadyRefreshed > DB_CACHE_EXPIRENCY))
	{
	    updateRepo();
        updateProjects();
		consolidateCache();
        alreadyRefreshed = time(NULL);
	}
}

/************** UPDATE REPO	********************/

int getUpdatedRepo(char **buffer_repo, size_t * bufferSize, ROOT_REPO_DATA repo)
{
	if(buffer_repo == NULL)
		return -1;
	
    int defaultVersion = VERSION_REPO;
	char temp[500];
	
	if(repo.type == TYPE_DEPOT_DB)
		snprintf(temp, 500, "https://dl.dropboxusercontent.com/u/%s/rakshata-repo-%d", repo.URL, defaultVersion);
	
	else if(repo.type == TYPE_DEPOT_OTHER)
		snprintf(temp, 500, "http://%s/rakshata-repo-%d", repo.URL, defaultVersion);
	
	else if(repo.type == TYPE_DEPOT_PAID) //Payant
		snprintf(temp, 500, SERVEUR_URL"/ressource.php?editor=%s&request=repo&version=%d", repo.URL, defaultVersion);
	
	else
	{
		snprintf(temp, 500, "Failed at understand what is the repo: %d", repo.type);
		logR(temp);
		return -1;
	}
	
	download_mem(temp, NULL, buffer_repo, bufferSize, repo.type != TYPE_DEPOT_OTHER ? SSL_ON : SSL_OFF);
	
	return isDownloadValid(*buffer_repo) ? defaultVersion : -1;
}

void updateRepo()
{
	uint nbRepoToRefresh;
	ROOT_REPO_DATA **oldRootData = (ROOT_REPO_DATA **) getCopyKnownRepo(&nbRepoToRefresh, true);
	ICONS_UPDATE * iconsData = NULL, * newIcons, * endIcons;

	if(oldRootData == NULL || nbRepoToRefresh == 0)
	{
		freeRootRepo(oldRootData);
		return;
	}
	
	char dataKS[NUMBER_MAX_REPO_KILLSWITCHE][2*SHA256_DIGEST_LENGTH+1];
	ROOT_REPO_DATA * newData;
	
	loadKS(dataKS);
	
	int dataVersion;
	size_t downloadLength;
	char * bufferDL = NULL;

	for(uint posRepo = 0; posRepo < nbRepoToRefresh; posRepo++)
	{
		if(oldRootData[posRepo] == NULL)
			continue;
		
		else if(checkKS(*oldRootData[posRepo], dataKS))
		{
			for(uint i = 0, length = oldRootData[posRepo]->nombreSubrepo; i < length; i++)
				KSTriggered(oldRootData[posRepo]->subRepo[i]);
	
			continue;
		}
		
		//Refresh effectif
		dataVersion = getUpdatedRepo(&bufferDL, &downloadLength, *oldRootData[posRepo]);
		if(bufferDL != NULL && downloadLength > 0 && parseRemoteRepoEntry(bufferDL, oldRootData[posRepo], dataVersion, &newData))
		{
			removeNonInstalledSubRepo(&(newData->subRepo), newData->nombreSubrepo, true);

			newIcons = enforceRepoExtra(newData, true);
			
			if(newIcons != NULL)
			{
				if(iconsData == NULL)
					endIcons = iconsData = newIcons;
				else
					endIcons->next = newIcons;
				
				while(endIcons->next != NULL)
					endIcons = endIcons->next;
			}
			
			memcpy(oldRootData[posRepo], newData, sizeof(ROOT_REPO_DATA));
		}
		
		free(bufferDL);
		bufferDL = NULL;
	}
	
	if(iconsData != NULL)
		createNewThread(updateProjectImages, iconsData);
	
	updateRootRepoCache(oldRootData);
	free(oldRootData);
	
	notifyFullUpdateRepo();
}

/******************* UPDATE PROJECTS ****************************/

int getUpdatedProjectOfRepo(char **projectBuf, REPO_DATA* repo)
{
	int defaultVersion = VERSION_PROJECT;
	char URL[500];
	size_t length;
	
    do
	{
	    if(repo->type == TYPE_DEPOT_DB)
            snprintf(URL, sizeof(URL), "https://dl.dropboxusercontent.com/u/%s/rakshata-project-%d", repo->URL, defaultVersion);

        else if(repo->type == TYPE_DEPOT_OTHER)
            snprintf(URL, sizeof(URL), "http://%s/rakshata-project-%d", repo->URL, defaultVersion);

        else if(repo->type == TYPE_DEPOT_PAID) //Payant
            snprintf(URL, sizeof(URL), SERVEUR_URL"/ressource.php?editor=%s&request=project&version=%d", repo->URL, defaultVersion);

        else
        {
            char temp[LENGTH_PROJECT_NAME + 100];
            snprintf(temp, sizeof(temp), "Failed at read mode (project database): %d", repo->type);
            logR(temp);
            return -1;
        }
		
		if(download_mem(URL, NULL, projectBuf, &length, repo->type != TYPE_DEPOT_OTHER ? SSL_ON : SSL_OFF) != CODE_RETOUR_OK || length == 0)
		{
			free(*projectBuf);
			*projectBuf = NULL;
		}
		
        defaultVersion--;
		
	} while(defaultVersion > 0 && !isDownloadValid(*projectBuf));

    return defaultVersion+1;
}

void refreshRepo(REPO_DATA * repo)
{
	ICONS_UPDATE * iconData = refreshRepoHelper(repo, true);
	
	createNewThread(updateProjectImages, iconData);
	syncCacheToDisk(SYNC_REPO | SYNC_PROJECTS);
	notifyUpdateRepo(*repo);
}

void * refreshRepoHelper(REPO_DATA * repo, bool standalone)
{
	uint nbElem, posBase = 0, posEnd = 0;
	uint64_t requestedID = getRepoID(repo);
	PROJECT_DATA * project = getCopyCache(SORT_REPO, &nbElem);
	
	while(posBase < nbElem && getRepoID(project[posBase].repo) != requestedID)		posBase++;
	
	//Repo couldn't be found :|
	if(posBase == nbElem)
	{
		freeProjectData(project);
		
		PROJECT_DATA emptyProject = getEmptyProject();
		emptyProject.repo = repo;
		
		return updateProjectsFromRepo(&emptyProject, 0, 0, standalone);
	}
	
	posEnd = posBase + 1;
	while(posEnd < nbElem && requestedID == getRepoID(project[posEnd].repo))			posEnd++;
	
	void * output = updateProjectsFromRepo(project, posBase, posEnd, standalone);
	
	freeProjectData(project);
	
	return output;
}

void * updateProjectsFromRepo(PROJECT_DATA* oldData, uint posBase, uint posEnd, bool standalone)
{
	REPO_DATA *globalRepo = oldData[posBase].repo;
	uint magnitudeInput = posEnd - posBase, nbElem = 0;
	char * bufferDL = NULL;
	void * output = NULL;
	
#ifdef PAID_CONTENT_ONLY_FOR_PAID_REPO
	bool paidRepo = globalRepo->type == TYPE_DEPOT_PAID;
#endif
	int version = getUpdatedProjectOfRepo(&bufferDL, globalRepo);
	
	if(version != -1 && downloadedProjectListSeemsLegit(bufferDL))		//On a des données à peu près valide
	{
		PROJECT_DATA_EXTRA * projects = parseRemoteData(globalRepo, bufferDL, &nbElem);
		output = updateImagesForProjects(projects, nbElem);
		
		//On maintenant voir les nouveaux éléments, ceux MaJ, et les supprimés, et appliquer les changements
		if(projects != NULL)
		{
			PROJECT_DATA *projectShort = malloc(nbElem * sizeof(PROJECT_DATA));
			if(projectShort != NULL)
			{
				for (uint pos = 0; pos < nbElem; pos++)
				{
					moveProjectExtraToStandard(projects[pos], &projectShort[pos]);
					
#ifdef PAID_CONTENT_ONLY_FOR_PAID_REPO
					if(projectShort[pos].isPaid && !paidRepo)
					{
						projectShort[pos].isPaid = false;
						free(projectShort[pos].chapitresPrix);
						projectShort[pos].chapitresPrix = NULL;
					}
#endif
				}
			
				applyChangesProject(&oldData[posBase], magnitudeInput, projectShort, nbElem);
				free(projectShort);
				
				if(standalone)
					notifyUpdateRepo(*globalRepo);
				
			}
			free(projects);
		}
	}
	
	free(bufferDL);
	
	return output;
}

void updateProjects()
{
	uint nbElem, posBase = 0, posEnd, nbRepoRefreshed = 0;
	PROJECT_DATA * oldData = getCopyCache(RDB_LOADALL | SORT_REPO, &nbElem);
	ICONS_UPDATE * iconData = NULL, * endIcon, * newIcon;
	
	while(posBase != nbElem)
	{
		posEnd = defineBoundsRepoOnProjectDB(oldData, posBase, nbElem);
		if(posEnd != UINT_MAX)
		{
			newIcon = updateProjectsFromRepo(oldData, posBase, posEnd, false);
			nbRepoRefreshed++;
			
			if(newIcon != NULL)
			{
				if(iconData == NULL)
					iconData = endIcon = newIcon;
				else
					endIcon->next = newIcon;
				
				while(endIcon->next != NULL)
					endIcon = endIcon->next;
			}
		}
		else
			break;

		posBase = posEnd;
	}
	
	if(!isAppropriateNumberOfRepo(nbRepoRefreshed))
	{
		//We didn't refreshed every repo, WTF?
		//Let's refresh them manually
		
		uint realNumberOfRepo;
		REPO_DATA ** repo = (REPO_DATA **)getCopyKnownRepo(&realNumberOfRepo, false);
		
		if(repo != NULL)
		{
			bool *refreshedTable = calloc(realNumberOfRepo, sizeof(bool));
			
			if(refreshedTable != NULL)
			{
				uint posRepo = 0;
				posBase = 0;
				
				//We're going to go through the updated repo and see if any is missing
				while(posBase != nbElem)
				{
					posEnd = defineBoundsRepoOnProjectDB(oldData, posBase, nbElem);
					if(posEnd != UINT_MAX)
					{
						//Now, find the repo in our base, starting from where we left (it's supposed to be ordered)
						for(; posRepo < realNumberOfRepo && getRepoID(repo[posRepo]) != getRepoID(oldData[posBase].repo); posRepo++);
						
						//Couldn't find the repo, weird, let's recheck from the begining
						if(posRepo == realNumberOfRepo)
						{
							for(posRepo = 0; posRepo < realNumberOfRepo && getRepoID(repo[posRepo]) != getRepoID(oldData[posBase].repo); posRepo++);
							
							//Okay, the repo was probably deleted
							if(posRepo == realNumberOfRepo)
							{
#ifdef DEV_VERSION
								char temp[100+LONGUEUR_URL];
								snprintf(temp, sizeof(temp), "Repo deleted during refresh? %d - %d", oldData[posBase].repo->parentRepoID, oldData[posBase].repo->repoID);
								logR(temp);
#endif
								posBase = posEnd;								
								continue;
							}
						}
						
						refreshedTable[posRepo] = true;
					}
					else
						break;
					
					posBase = posEnd;
				}
				
				//Okay, now, let's find the culprits
				for(uint i = 0; i < realNumberOfRepo; i++)
				{
					if(!refreshedTable[i])
					{
						newIcon = refreshRepoHelper(repo[i], false);
						
						if(newIcon != NULL)
						{
							if(iconData == NULL)
								iconData = endIcon = newIcon;
							else
								endIcon->next = newIcon;
							
							while(endIcon->next != NULL)
								endIcon = endIcon->next;
						}
					}
				}
				
				free(refreshedTable);
			}
			
			freeRepo(repo);
		}
	}
	
	createNewThread(updateProjectImages, iconData);
	syncCacheToDisk(SYNC_REPO | SYNC_PROJECTS);
	freeProjectData(oldData);
	notifyFullUpdate();
}

void deleteProject(PROJECT_DATA project, int elemToDel, bool isTome)
{
	if(elemToDel == INVALID_SIGNED_VALUE)	//On supprime tout
	{
		char path[2*LENGTH_PROJECT_NAME + 25], *encodedRepo = getPathForProject(project);
		
		if(encodedRepo != NULL)
		{
			snprintf(path, sizeof(path), PROJECT_ROOT"%s", encodedRepo);
			removeFolder(path);
		}
		free(encodedRepo);
	}
	else
		internalDeleteCT(project, isTome, elemToDel);
	
	notifyUpdateProject(project);
}

void setLastChapitreLu(PROJECT_DATA project, bool isTome, int dernierChapitre)
{
	char temp[5*LENGTH_PROJECT_NAME], *encodedRepo = getPathForProject(project);
	FILE* fichier = NULL;
	
	if(encodedRepo == NULL)
		return;

    if(isTome)
        snprintf(temp, 5*LENGTH_PROJECT_NAME, PROJECT_ROOT"%s/"CONFIGFILETOME, encodedRepo);
	else
        snprintf(temp, 5*LENGTH_PROJECT_NAME, PROJECT_ROOT"%s/"CONFIGFILE, encodedRepo);

	fichier = fopen(temp, "w+");
	fprintf(fichier, "%d", dernierChapitre);
	fclose(fichier);
}
