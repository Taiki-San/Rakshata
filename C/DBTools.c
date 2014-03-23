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

#include "db.h"

/**********		REFRESH TEAMS		***************/

bool isRemoteRepoLineValid(char * data, int version)
{
	if(version < 1 || version > 2)
		return false;
	
	uint basePos, nbrSpaces = 0;
	
	for(basePos = 0; data[basePos] && (data[basePos] < '!' || data[basePos] > '~'); basePos++);
	
	//Check the number of spaces
	nbrSpaces = countSpaces(&data[basePos]);
	
	if((version == 1 && nbrSpaces != 5) || (version == 2 && nbrSpaces != 5))
	{
#ifdef DEV_VERSION
		uint messageLength = 200 + strlen(data);
		char logMessage[messageLength];
		snprintf(logMessage, messageLength, "Incoherent number of spaces in downloaded repo file, dumping data: version = %d, nbrSpaces = %d\ndata: %s", version, nbrSpaces, data);
		logR(logMessage);
#endif
		return false;
	}
	
	return true;
}

bool parseRemoteRepoLine(char *data, TEAMS_DATA *previousData, int version, TEAMS_DATA *output)
{
	if(version == -1 || !isRemoteRepoLineValid(data, version))
		logR("An error occured");
	
	else if(version == 1)	//Legacy mode
	{
		char uselessID[10];
		sscanfs(data, "%s %s %s %s %s %s", uselessID, 10, output->teamLong, LONGUEUR_NOM_MANGA_MAX, output->teamCourt, LONGUEUR_COURT, output->type, LONGUEUR_TYPE_TEAM, output->URL_depot, LONGUEUR_URL, output->site, LONGUEUR_SITE);
		
		if(strcmp(output->type, TYPE_DEPOT_3) && strcmp(output->type, TYPE_DEPOT_2) && strcmp(output->type, TYPE_DEPOT_1))
			return false;
		
		output->openSite = (previousData == NULL) ? 1 : previousData->openSite;
		return true;
	}
	
	else if(version == 2)
	{
		sscanfs(data, "%s %s %s %s %s %d", output->teamLong, LONGUEUR_NOM_MANGA_MAX, output->teamCourt, LONGUEUR_COURT, output->type, LONGUEUR_TYPE_TEAM, output->URL_depot, LONGUEUR_URL, output->site, LONGUEUR_SITE, &output->openSite);
		
		if(strcmp(output->type, TYPE_DEPOT_3) && strcmp(output->type, TYPE_DEPOT_2) && strcmp(output->type, TYPE_DEPOT_1))
			return false;
		
		return true;
	}
	else
		logR("Unsupported repo, an update is probably required");
	
	if(previousData != NULL)
		memcpy(output, previousData, sizeof(TEAMS_DATA));
	
	return false;
}


/**********		REFRESH PROJECTS		***********/

uint defineBoundsTeamOnProjectDB(MANGAS_DATA * oldData, uint posBase, uint nbElem)
{
	if(oldData == NULL)
		return UINT_MAX;
	
	for(; posBase < nbElem && oldData[posBase].team == NULL; posBase++);
	
	void * ptrTeam = oldData[posBase].team;
	
	for (posBase++; oldData[posBase].team == ptrTeam; posBase++);
	
	return posBase;
}

bool downloadedProjectListSeemsLegit(char *data, TEAMS_DATA* team)
{
	if(strlen(data) < 20)	//Test pourris mais plutôt rapide
		return false;
	
	int pos = 0;
	for(; data[pos] && (data[pos] <= ' ' && data[pos] > '~'); pos++);
	
	if(!data[pos])
		return false;
	
	if(data[pos] == '<' || data[pos + 1] == '<' || data[pos + 2] == '<' || data[pos + 3] == '<')
		return false;
	
	char teamName[LONGUEUR_NOM_MANGA_MAX] = {0}, teamNameCourt[LONGUEUR_COURT] = {0};
	sscanfs(data, "%s %s", teamName, LONGUEUR_NOM_MANGA_MAX, teamNameCourt, LONGUEUR_COURT);
	
	if(teamName[0] == 0 || teamNameCourt[0] == 0)	//Header malformé
		return false;
	
	if(strcmp(team->teamLong, teamName) || strcmp(team->teamCourt, teamNameCourt))
		return false;
	
	return true;
}

uint getNumberLineReturn(char *input)
{
	uint output, pos;
	bool wasLastLineAReturn = true;
	
	for(output = pos = 0; input[pos] && input[pos] != '#'; pos++)
	{
		if(input[pos] == '\n' && !wasLastLineAReturn)
		{
			output++;
			wasLastLineAReturn = true;
		}
		else if(wasLastLineAReturn && input[pos] > ' ' && input[pos] <= '~')
			wasLastLineAReturn = false;
	}
	
	return output;
}

bool extractCurrentLine(char * input, uint *posInput, char * output, uint lengthOutput)
{
	//This function is an advanced sanitizer of the line. It will copy the right amount of data, strip every unexepected char and return a nice, sanitized string
	uint pos = 0, posInputLocal = *posInput;
	bool wasLastCharASpace = false;
	char curChar, rank = 0;
	
	//first, we jump spaces at the begining of the line
	for(; input[posInputLocal] == ' '; posInputLocal++);
	
	lengthOutput--;	//On évite ainsi d'avoir à faire un -1 à chaque itération

	if(input[posInputLocal] == '#')
		return false;
	
	for(output[pos] = 0; pos < lengthOutput && (curChar = input[posInputLocal]) && curChar != '\n' && curChar != '\r'; posInputLocal++)
	{
		if(curChar < ' ' || curChar > '~')
			continue;
			
		else if(curChar == ' ')
		{
			if(!wasLastCharASpace)
			{
				output[pos++] = input[posInputLocal];
				wasLastCharASpace = true;
				rank++;
			}
		}
		else if(rank < 2 || ((curChar >= '0' && curChar <= '9') || (wasLastCharASpace && curChar == '-')))
		{
			wasLastCharASpace = false;
			output[pos++] = input[posInputLocal];
		}
	}
	output[pos] = 0;

	//on déplace le curseur à la fin de la ligne si le buffer à été limitant
	if(pos == lengthOutput)
		for (; input[posInputLocal] && input[posInputLocal] != '\n' && input[posInputLocal] != '\r'; posInputLocal++);
	
	if(*input)
		for(; input[posInputLocal] == '\n' || input[posInputLocal] == '\r'; posInputLocal++);
	
	*posInput = posInputLocal;
	
	return (rank >= 5 && rank <= 9);
}

bool parseCurrentProjectLine(char * input, int version, MANGAS_DATA * output)
{
	int categorie = 11;
	if(version == 1)	//Legacy
	{
		sscanfs(input, "%s %s %d %d %d %d", output->mangaName, LONGUEUR_NOM_MANGA_MAX, output->mangaNameShort, LONGUEUR_COURT, &output->firstChapter, &output->lastChapter, &categorie, &output->pageInfos);
		output->firstTome = VALEUR_FIN_STRUCTURE_CHAPITRE;
		output->nombreChapitreSpeciaux = 0;
	}
	else if(version == 2)
	{
		int depreciated;
		sscanfs(input, "%s %s %d %d %d %d %d %d %d", output->mangaName, LONGUEUR_NOM_MANGA_MAX, output->mangaNameShort, LONGUEUR_COURT, &output->firstChapter, &output->lastChapter, &output->firstTome, &depreciated, &categorie, &output->pageInfos, &output->nombreChapitreSpeciaux);
	}
	
	//On met des defaults
	output->chapitres = NULL;
	output->tomes = NULL;
	output->favoris = 0;
	output->contentDownloadable = 0;
	
	output->genre = categorie > 10 ? categorie / 10 : 1;
	output->status = categorie % 10;

	return checkPathEscape(output->mangaName, LONGUEUR_NOM_MANGA_MAX);
}

void parseDetailsBlock(char * input, MANGAS_DATA *data, char *teamName, uint lengthOfBlock)
{
	bool isTome;
	uint index, pos;
	char projectName[LONGUEUR_NOM_MANGA_MAX];
	pos = sscanfs(input, "%s", projectName, LONGUEUR_NOM_MANGA_MAX);
	
	for(index = 0; data[index].team != NULL && strcmp(projectName, data[index].mangaName); index++);
	
	if(data[index].team == NULL)	//Le bloc n'est pas celui d'un projet
		return;
	
	for(; input[pos] && input[pos] != ' '; pos++);
	for(; input[pos] == ' '; pos++);
	
	//On regarde si on a un truc lisible
	if(input[pos] == 'C')
		isTome = false;

	else if(input[pos] == 'T')
		isTome = true;
	
	else
		return;
	
	//On va créer le fichier et écrire le contenu du bloc

	char path[LONGUEUR_NOM_MANGA_MAX * 2 + 100];
	snprintf(path, sizeof(path), "manga/%s/%s", teamName, projectName);
	
	if(!checkDirExist(path))	//La fonction marche aussi pour voir si un dossier existe
		createPath(path);
	
	snprintf(path, sizeof(path), "manga/%s/%s/%s", teamName, projectName, isTome ? TOME_INDEX : CHAPITRE_INDEX);
	
	pos += jumpLine(input);
	
	FILE *output = fopen(path, "w+");
	if(output != NULL)
	{
		if(fwrite(&input[pos], sizeof(char), lengthOfBlock - pos, output) != lengthOfBlock - pos)
		{
#ifdef DEV_VERSION
			logR("Something went wrong when parsing project data");
#endif
		}
		fclose(output);
	}
}

bool isProjectListSorted(MANGAS_DATA* data, uint length)
{
	int logData;
	for(uint i = 1; i < length; i++)
	{
		if((logData = sortMangas(&data[i-1], &data[i])) > 0)
			return false;
	}
	return true;
}

void applyChangesProject(MANGAS_DATA * oldData, uint magnitudeOldData, MANGAS_DATA * newData, uint magnitudeNewData)
{
	
	uint IDTeam = getDBTeamID(oldData[0].team);
	
	if(IDTeam == 0xffffffff)
		return;
	
	//On commence par reclasser les éléments
	if(!isProjectListSorted(oldData, magnitudeOldData))
		qsort(oldData, magnitudeOldData, sizeof(MANGAS_DATA), sortMangas);
	
	if(!isProjectListSorted(newData, magnitudeNewData))
		qsort(newData, magnitudeNewData, sizeof(MANGAS_DATA), sortMangas);
	
	bool newChapters;
	uint posOld = 0, posNew = 0;
	int outputSort;
	MANGAS_DATA internalBufferOld, internalBufferNew;
	sqlite3_stmt * request = getAddToCacheRequest();
	
	while(posOld < magnitudeOldData && posNew < magnitudeNewData)
	{
		outputSort = sortMangas(&oldData[posOld], &newData[posNew]);

		if(outputSort < 0)			//Projet dans oldData pas dans newData, on le delete
		{
			removeFromCache(oldData[posOld]);
#ifdef DELETE_UNLISTED_PROJECT
			char path[LONGUEUR_NOM_MANGA_MAX * 2 + 10];
			snprintf(path, sizeof(path), "manga/%s/%s", oldData[posOld].team->teamLong, oldData[posOld].mangaName);
			removeFolder(path);
#endif
			posOld++;
		}
		
		else if(outputSort == 0)	//On a trouvé une version mise à jour
		{
			internalBufferOld = oldData[posOld];
			internalBufferNew = newData[posNew];
			
			if(internalBufferOld.lastChapter != internalBufferNew.lastChapter || internalBufferOld.firstTome != internalBufferNew.firstTome || internalBufferOld.nombreChapitreSpeciaux != internalBufferNew.nombreChapitreSpeciaux || internalBufferOld.pageInfos != internalBufferNew.pageInfos || internalBufferOld.status != internalBufferNew.status || internalBufferOld.firstChapter != internalBufferNew.firstChapter || strcmp(internalBufferOld.mangaName, internalBufferNew.mangaName) || internalBufferOld.genre != internalBufferNew.genre)	//quelque chose à changé
			{
				newData[posNew].cacheDBID = oldData[posOld].cacheDBID;
				newData[posNew].favoris = oldData[posOld].favoris;

				if(internalBufferOld.firstChapter != internalBufferNew.firstChapter || internalBufferOld.lastChapter != internalBufferNew.lastChapter || internalBufferOld.nombreChapitreSpeciaux != internalBufferNew.nombreChapitreSpeciaux)
				{
					refreshChaptersList(&newData[posNew]);
					newChapters = true;
				}
				else
				{
					newData[posNew].chapitres = malloc((oldData[posOld].nombreChapitre + 1) * sizeof(int));
					
					if(newData[posNew].chapitres != NULL)
					{
						memcpy(newData[posNew].chapitres, oldData[posOld].chapitres, oldData[posOld].nombreChapitre * sizeof(int));
						newData[posNew].nombreChapitre = oldData[posOld].nombreChapitre;
						newData[posNew].chapitres[newData[posNew].nombreChapitre] = VALEUR_FIN_STRUCTURE_CHAPITRE;
					}
					
					newChapters = false;
				}
				
				if(newData[posNew].firstTome != VALEUR_FIN_STRUCTURE_CHAPITRE)
					refreshTomeList(&newData[posNew]);
				else
					newData[posNew].tomes = NULL;
				
				if(newChapters || newData[posNew].tomes != NULL)
				{
					newData[posNew].contentDownloadable = isAnythingToDownload(newData[posNew]);
				}
				else
				{
					newData[posNew].contentDownloadable = oldData[posOld].contentDownloadable;
				}
				
				updateCache(newData[posNew], RDB_UPDATE_ID, NULL);
				
				free(newData[posNew].chapitres);	//updateCache en fait une copie
				freeTomeList(newData[posNew].tomes, false);
			}
			
			posOld++;
			posNew++;
		}
		
		else						//Nouveau projet
		{
			newData[posNew].cacheDBID = 0;
			
			refreshChaptersList(&newData[posNew]);
			refreshTomeList(&newData[posNew]);
			newData[posNew].contentDownloadable = isAnythingToDownload(newData[posNew]);
			
			addToCache(request, newData[posNew], IDTeam, false);
			
			posNew++;
		}
	}
	
	while (posOld < magnitudeOldData)
	{
		removeFromCache(oldData[posOld]);
#ifdef DELETE_UNLISTED_PROJECT
		char path[LONGUEUR_NOM_MANGA_MAX * 2 + 10];
		snprintf(path, sizeof(path), "manga/%s/%s", oldData[posOld].team->teamLong, oldData[posOld].mangaName);
		removeFolder(path);
#endif
		posOld++;
	}
	
	while (posNew < magnitudeNewData)
	{
		newData[posNew].cacheDBID = 0;
		
		refreshChaptersList(&newData[posNew]);
		refreshTomeList(&newData[posNew]);
		newData[posNew].contentDownloadable = isAnythingToDownload(newData[posNew]);
		
		addToCache(request, newData[posNew], IDTeam, false);
		
		posNew++;
	}
	
	sqlite3_finalize(request);
}

/*****************		DIVERS		******************/

void resetUpdateDBCache()
{
    alreadyRefreshed = -DB_CACHE_EXPIRENCY;
}