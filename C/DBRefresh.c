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

#include "db.h"

void updateDatabase(bool forced)
{
    MUTEX_LOCK(mutex);
    if(NETWORK_ACCESS != CONNEXION_DOWN && (forced || time(NULL) - alreadyRefreshed > DB_CACHE_EXPIRENCY))
	{
        MUTEX_UNLOCK(mutex);
	    updateRepo();
        updateProjects();
		consolidateCache();
        alreadyRefreshed = time(NULL);
	}
    else
        MUTEX_UNLOCK(mutex);
}

/************** UPDATE REPO	********************/

int getUpdatedRepo(char *buffer_repo, uint bufferSize, TEAMS_DATA teams)
{
	if(buffer_repo == NULL)
		return -1;
	
    int defaultVersion = VERSION_REPO;
	char temp[500];
	do
	{
        if(!strcmp(teams.type, TYPE_DEPOT_DB))
            snprintf(temp, 500, "https://dl.dropboxusercontent.com/u/%s/rakshata-repo-%d", teams.URLRepo, defaultVersion);
		
        else if(!strcmp(teams.type, TYPE_DEPOT_OTHER))
            snprintf(temp, 500, "http://%s/rakshata-repo-%d", teams.URLRepo, defaultVersion);
		
        else if(!strcmp(teams.type, TYPE_DEPOT_PAID)) //Payant
            snprintf(temp, 500, "https://"SERVEUR_URL"/ressource.php?editor=%s&request=repo&version=%d", teams.URLRepo, defaultVersion);
		
        else
        {
            snprintf(temp, 500, "Failed at understand what is the repo: %s", teams.type);
            logR(temp);
            return -1;
        }
		
        buffer_repo[0] = 0;
        download_mem(temp, NULL, buffer_repo, bufferSize, strcmp(teams.type, TYPE_DEPOT_OTHER) ? SSL_ON : SSL_OFF);
        defaultVersion--;
		
	} while(defaultVersion > 0 && !isDownloadValid(buffer_repo));
	return defaultVersion+1;
}

void updateRepo()
{
	uint nbTeamToRefresh;
	TEAMS_DATA **oldData = getCopyKnownTeams(&nbTeamToRefresh);

	if(oldData == NULL || nbTeamToRefresh == 0)
	{
		free(oldData);
		return;
	}
	
	char dataKS[NUMBER_MAX_TEAM_KILLSWITCHE][2*SHA256_DIGEST_LENGTH+1];
	TEAMS_DATA newData;
	
	loadKS(dataKS);
	
	int dataVersion;
	char * bufferDL = calloc(1, SIZE_BUFFER_UPDATE_DATABASE);

	if(bufferDL == NULL)
	{
		freeTeam(oldData);
		return;
	}

	for(int posTeam = 0; posTeam < nbTeamToRefresh; posTeam++)
	{
		if(oldData[posTeam] == NULL)
			continue;
		else if(checkKS(*oldData[posTeam], dataKS))
		{
			KSTriggered(*oldData[posTeam]);
			continue;
		}
		
		//Refresh effectif
		dataVersion = getUpdatedRepo(bufferDL, SIZE_BUFFER_UPDATE_DATABASE, *oldData[posTeam]);
		if(parseRemoteRepoLine(bufferDL, oldData[posTeam], dataVersion, &newData))
			memcpy(oldData[posTeam], &newData, sizeof(TEAMS_DATA));

	}
	free(bufferDL);
	updateTeamCache(oldData, -1);
	free(oldData);
}

/******************* UPDATE PROJECTS ****************************/

int getUpdatedProjectOfTeam(char *projectBuf, TEAMS_DATA* teams)
{
	int defaultVersion = VERSION_PROJECT;
	char URL[500];
    do
	{
	    if(!strcmp(teams->type, TYPE_DEPOT_DB))
            snprintf(URL, sizeof(URL), "https://dl.dropboxusercontent.com/u/%s/rakshata-project-%d", teams->URLRepo, defaultVersion);

        else if(!strcmp(teams->type, TYPE_DEPOT_OTHER))
            snprintf(URL, sizeof(URL), "http://%s/rakshata-project-%d", teams->URLRepo, defaultVersion);

        else if(!strcmp(teams->type, TYPE_DEPOT_PAID)) //Payant
            snprintf(URL, sizeof(URL), "https://"SERVEUR_URL"/ressource.php?editor=%s&request=project&version=%d", teams->URLRepo, defaultVersion);

        else
        {
            char temp[LENGTH_PROJECT_NAME + 100];
            snprintf(temp, sizeof(temp), "failed at read mode(project database): %s", teams->type);
            logR(temp);
            return -1;
        }
		
        projectBuf[0] = 0;
        download_mem(URL, NULL, projectBuf, SIZE_BUFFER_UPDATE_DATABASE, strcmp(teams->type, TYPE_DEPOT_OTHER)?SSL_ON:SSL_OFF);
        defaultVersion--;
		
	} while(defaultVersion > 0 && !isDownloadValid(projectBuf));

    return defaultVersion+1;
}

void refreshTeam(TEAMS_DATA * team, bool standalone)
{
	PROJECT_DATA project;
	
	project.isInitialized = false;
	project.team = team;
	
	updateProjectsFromTeam(&project, 0, 0, standalone);
}

void updateProjectsFromTeam(PROJECT_DATA* oldData, uint posBase, uint posEnd, bool standalone)
{
	TEAMS_DATA *globalTeam = oldData[posBase].team;
	uint magnitudeInput = posEnd - posBase, nbElem = 0;
	char * bufferDL = malloc(SIZE_BUFFER_UPDATE_DATABASE);
	
	if(bufferDL == NULL)
		return;

#ifdef PAID_CONTENT_ONLY_FOR_PAID_REPO
	bool paidTeam = !strcmp(globalTeam->type, TYPE_DEPOT_PAID);
#endif
	int version = getUpdatedProjectOfTeam(bufferDL, globalTeam);
	
	if(version != -1 && downloadedProjectListSeemsLegit(bufferDL, globalTeam))		//On a des données à peu près valide
	{
		PROJECT_DATA_EXTRA * projects = parseRemoteData(globalTeam, bufferDL, &nbElem);
		updatePageInfoForProjects(projects, nbElem);
		
		//On maintenant voir les nouveaux éléments, ceux MaJ, et les supprimés, et appliquer les changements
		if(projects != NULL)
		{
			PROJECT_DATA *projectShort = malloc(nbElem * sizeof(PROJECT_DATA));
			if(projectShort != NULL)
			{
				for (uint pos = 0; pos < nbElem; pos++)
				{
					memcpy(&projectShort[pos], &projects[pos], sizeof(PROJECT_DATA));
					
#ifdef PAID_CONTENT_ONLY_FOR_PAID_REPO
					if(projectShort[pos].isPaid && !paidTeam)
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
					notifyUpdateTeam(*globalTeam);
				
			}
			free(projects);
		}
	}
	
	free(bufferDL);
}

void updateProjects()
{
	uint nbElem, posBase = 0, posEnd, nbTeamRefreshed = 0;
	PROJECT_DATA * oldData = getCopyCache(RDB_LOADALL | SORT_TEAM, &nbElem);
	
	while(posBase != nbElem)
	{
		posEnd = defineBoundsTeamOnProjectDB(oldData, posBase, nbElem);
		if(posEnd != UINT_MAX)
		{
			updateProjectsFromTeam(oldData, posBase, posEnd, false);
			nbTeamRefreshed++;
		}
		else
			break;

		posBase = posEnd;
	}
	
	if(!isAppropriateNumberOfTeam(nbTeamRefreshed))
	{
		//We didn't refreshed every team, WTF?
		//Let's refresh them manually
		
		uint realNumberOfTeam;
		TEAMS_DATA ** teams = getCopyKnownTeams(&realNumberOfTeam);
		
		if(teams != NULL)
		{
			bool *refreshedTable = calloc(realNumberOfTeam, sizeof(bool));
			
			if(refreshedTable != NULL)
			{
				uint posTeam = 0;
				posBase = 0;
				
				//We're going to go through the updated repo and see if any is missing
				while(posBase != nbElem)
				{
					posEnd = defineBoundsTeamOnProjectDB(oldData, posBase, nbElem);
					if(posEnd != UINT_MAX)
					{
						//Now, find the team in our base, starting from where we left (it's supposed to be ordered)
						for(; posTeam < realNumberOfTeam && strcmp(teams[posTeam]->URLRepo, oldData[posBase].team->URLRepo); posTeam++);
						
						//Couldn't find the team, weird, let's recheck from the begining
						if(posTeam == realNumberOfTeam)
						{
							for(posTeam = 0; posTeam < realNumberOfTeam && strcmp(teams[posTeam]->URLRepo, oldData[posBase].team->URLRepo); posTeam++);
							
							if(posTeam == realNumberOfTeam)
							{
								char temp[100+LONGUEUR_URL];
								snprintf(temp, sizeof(temp), "Missing team, WTF? %s", oldData[posBase].team->URLRepo);
								logR(temp);
								
								continue;
							}
						}
						
						refreshedTable[posTeam] = true;
					}
					else
						break;
					
					posBase = posEnd;
				}
				
				//Okay, now, let's find the culprits
				for(uint i = 0; i < realNumberOfTeam; i++)
				{
					if(!refreshedTable[i])
						refreshTeam(teams[i], false);
				}
				
				free(refreshedTable);
			}
			
			freeTeam(teams);
		}
	}
	
	syncCacheToDisk(SYNC_TEAM | SYNC_PROJECTS);
	freeProjectData(oldData);
	notifyFullUpdate();
}

void deleteProject(PROJECT_DATA project, int elemToDel, bool isTome)
{
	if(elemToDel == VALEUR_FIN_STRUCT)	//On supprime tout
	{
		char path[2*LENGTH_PROJECT_NAME + 25], *encodedTeam = getPathForTeam(project.team->URLRepo);
		
		if(encodedTeam != NULL)
		{
			snprintf(path, sizeof(path), PROJECT_ROOT"%s/%d", encodedTeam, project.projectID);
			removeFolder(path);
		}
		free(encodedTeam);
	}
	else
		internalDeleteCT(project, isTome, elemToDel);
	
	notifyUpdateProject(project);
}

void setLastChapitreLu(PROJECT_DATA project, bool isTome, int dernierChapitre)
{
	char temp[5*LENGTH_PROJECT_NAME], *encodedTeam = getPathForTeam(project.team->URLRepo);
	FILE* fichier = NULL;
	
	if(encodedTeam == NULL)
		return;

    if(isTome)
        snprintf(temp, 5*LENGTH_PROJECT_NAME, PROJECT_ROOT"%s/%d/"CONFIGFILETOME, encodedTeam, project.projectID);
	else
        snprintf(temp, 5*LENGTH_PROJECT_NAME, PROJECT_ROOT"%s/%d/"CONFIGFILE, encodedTeam, project.projectID);

	fichier = fopen(temp, "w+");
	fprintf(fichier, "%d", dernierChapitre);
	fclose(fichier);
}

int databaseVersion(char* projectDB)
{
    if(*projectDB == ' ' && *(projectDB+1) >= '0' && *(projectDB+1) <= '9')
    {
        projectDB++;
        char buffer[10];
        int i = 0;
        for(; i < 9 && *projectDB >= '0' && *projectDB <= '9'; projectDB++)
            buffer[i++] = *projectDB;
        buffer[i] = 0;
        return charToInt(buffer);
    }
    return 0;
}

