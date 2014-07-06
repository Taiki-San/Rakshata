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
        if(!strcmp(teams.type, TYPE_DEPOT_1))
            snprintf(temp, 500, "https://dl.dropboxusercontent.com/u/%s/rakshata-repo-%d", teams.URLRepo, defaultVersion);
		
        else if(!strcmp(teams.type, TYPE_DEPOT_2))
            snprintf(temp, 500, "http://%s/rakshata-repo-%d", teams.URLRepo, defaultVersion);
		
        else if(!strcmp(teams.type, TYPE_DEPOT_3)) //Payant
            snprintf(temp, 500, "https://"SERVEUR_URL"/ressource.php?editor=%s&request=repo&user=%s&version=%d", teams.URLRepo, COMPTE_PRINCIPAL_MAIL, defaultVersion);
		
        else
        {
            snprintf(temp, 500, "Failed at understand what is the repo: %s", teams.type);
            logR(temp);
            return -1;
        }
		
        buffer_repo[0] = 0;
        download_mem(temp, NULL, buffer_repo, bufferSize, strcmp(teams.type, TYPE_DEPOT_2) ? SSL_ON : SSL_OFF);
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

int getUpdatedProjectOfTeam(char *buffer_manga, TEAMS_DATA* teams)
{
	int defaultVersion = VERSION_PROJECT;
	char URL[500];
    do
	{
	    if(!strcmp(teams->type, TYPE_DEPOT_1))
            snprintf(URL, sizeof(URL), "https://dl.dropboxusercontent.com/u/%s/rakshata-project-%d", teams->URLRepo, defaultVersion);

        else if(!strcmp(teams->type, TYPE_DEPOT_2))
            snprintf(URL, sizeof(URL), "http://%s/rakshata-project-%d", teams->URLRepo, defaultVersion);

        else if(!strcmp(teams->type, TYPE_DEPOT_3)) //Payant
            snprintf(URL, sizeof(URL), "https://"SERVEUR_URL"/ressource.php?editor=%s&request=mangas&user=%s&version=%d", teams->URLRepo, COMPTE_PRINCIPAL_MAIL, defaultVersion);

        else
        {
            char temp[LENGTH_PROJECT_NAME + 100];
            snprintf(temp, sizeof(temp), "failed at read mode(project database): %s", teams->type);
            logR(temp);
            return -1;
        }
		
        buffer_manga[0] = 0;
        download_mem(URL, NULL, buffer_manga, SIZE_BUFFER_UPDATE_DATABASE, strcmp(teams->type, TYPE_DEPOT_2)?SSL_ON:SSL_OFF);
        defaultVersion--;
		
	} while(defaultVersion > 0 && !isDownloadValid(buffer_manga));

    return defaultVersion+1;
}

void updateProjectsFromTeam(PROJECT_DATA* oldData, uint posBase, uint posEnd)
{
	TEAMS_DATA *globalTeam = oldData[posBase].team;
	uint magnitudeInput = posEnd - posBase, nbElem = 0;
	char * bufferDL = malloc(SIZE_BUFFER_UPDATE_DATABASE);
	
	if(bufferDL == NULL)
		return;

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
					memcpy(&projectShort[pos], &projects[pos], sizeof(PROJECT_DATA));
				
				applyChangesProject(&oldData[posBase], magnitudeInput, projectShort, nbElem);
				free(projectShort);
			}
			free(projects);
		}
	}
	
	free(bufferDL);
}

void updateProjects()
{
	uint nbElem, posBase = 0, posEnd;
	PROJECT_DATA * oldData = getCopyCache(RDB_LOADALL | SORT_TEAM, &nbElem);
	
	while(posBase != nbElem)
	{
		posEnd = defineBoundsTeamOnProjectDB(oldData, posBase, nbElem);
		if(posEnd != UINT_MAX)
			updateProjectsFromTeam(oldData, posBase, posEnd);
		else
			break;

		posBase = posEnd;
	}
	freeMangaData(oldData);
}

void deleteProject(PROJECT_DATA project, int elemToDel, bool isTome)
{
	if(elemToDel == VALEUR_FIN_STRUCT)	//On supprime tout
	{
		char path[2*LENGTH_PROJECT_NAME + 25];
		snprintf(path, sizeof(path), "manga/%s/%d", project.team->teamLong, project.projectID);
		removeFolder(path);
	}
	else
	{
		internalDeleteCT(project, isTome, elemToDel);
	}
}

void setLastChapitreLu(PROJECT_DATA mangasDB, bool isTome, int dernierChapitre)
{
	char temp[5*LENGTH_PROJECT_NAME];
	FILE* fichier = NULL;

    if(isTome)
        snprintf(temp, 5*LENGTH_PROJECT_NAME, "manga/%s/%d/%s", mangasDB.team->teamLong, mangasDB.projectID, CONFIGFILETOME);
	else
        snprintf(temp, 5*LENGTH_PROJECT_NAME, "manga/%s/%d/%s", mangasDB.team->teamLong, mangasDB.projectID, CONFIGFILE);

	fichier = fopen(temp, "w+");
	fprintf(fichier, "%d", dernierChapitre);
	fclose(fichier);
}

int databaseVersion(char* mangaDB)
{
    if(*mangaDB == ' ' && *(mangaDB+1) >= '0' && *(mangaDB+1) <= '9')
    {
        mangaDB++;
        char buffer[10];
        int i = 0;
        for(; i < 9 && *mangaDB >= '0' && *mangaDB <= '9'; mangaDB++)
            buffer[i++] = *mangaDB;
        buffer[i] = 0;
        return charToInt(buffer);
    }
    return 0;
}

