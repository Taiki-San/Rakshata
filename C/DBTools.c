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
	
	uint basePos, pos, nbrSpaces = 0;
	
	for(basePos = 0; data[basePos] && (data[basePos] < '!' || data[basePos] > '~'); basePos++);
	
	//Check the number of spaces
	for(pos = basePos; data[pos];)
	{
		if(data[pos++] == ' ')
		{
			while(data[pos++] == ' ');
			
			if(data[pos] != 0)		//Si des espaces à la fin, on s'en fout
				nbrSpaces++;
			
		}
	}
	
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
		
		output->openSite = (previousData == NULL) ? 1 : previousData->openSite;
		return true;
	}
	
	else if(version == 2)
	{
		sscanfs(data, "%s %s %s %s %s %d", output->teamLong, LONGUEUR_NOM_MANGA_MAX, output->teamCourt, LONGUEUR_COURT, output->type, LONGUEUR_TYPE_TEAM, output->URL_depot, LONGUEUR_URL, output->site, LONGUEUR_SITE, &output->openSite);
		
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
	
	for(; posBase < nbElem && oldData[posBase].team != NULL; posBase++);
	
	void * ptrTeam = oldData[posBase].team;
	
	for (posBase++; oldData[posBase].team == ptrTeam; posBase++);
	
	return posBase;
}

bool downloadedProjectListSeemsLegit(char *data, MANGAS_DATA reference)
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
	
	if(strcmp(reference.team->teamLong, teamName) || strcmp(reference.team->teamCourt, teamNameCourt))
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

bool extractCurrentLine(char * input, char * output, uint lengthOutput)
{
	//This function is an advanced sanitizer of the line. It will copy the right amount of data, strip every unexepected char and return a nice, sanitized string
	uint pos = 0;
	bool wasLastCharASpace = false;
	char curChar, rank = 0;
	
	//first, we jump spaces at the begining of the line
	for(; *input == ' '; input++);
	
	lengthOutput--;	//On évite ainsi d'avoir à faire un -1 à chaque itération

	if(*input == '#')
		return false;
	
	for(output[pos] = 0; pos < lengthOutput && (curChar = *input) && curChar != '\n' && curChar != '\r'; input++)
	{
		if(curChar < ' ' || curChar > '~')
			continue;
			
		else if(curChar == ' ')
		{
			if(wasLastCharASpace)
			{
				output[pos++] = *input;
				wasLastCharASpace = true;
				rank++;
			}
		}
		else if(rank < 2 || (curChar >= '0' && curChar <= '9'))
		{
			wasLastCharASpace = false;
			output[pos++] = *input;
		}
	}
	output[pos] = 0;

	//on déplace le curseur à la fin de la ligne
	if(pos == lengthOutput)
		for (; *input && *input != '\n' && *input != '\r'; input++);
	
	if(*input)
		for(; *input == '\n' && *input == '\r'; input++);
	
	return (rank >= 6 && rank <= 10);
}

bool parseCurrentProjectLine(char * input, int version, MANGAS_DATA * output)
{
	int categorie = 11;
	if(version == 0)	//Legacy
	{
		sscanfs(input, "%s %s %d %d %d %d", output->mangaName, LONGUEUR_NOM_MANGA_MAX, output->mangaNameShort, LONGUEUR_COURT, &output->firstChapter, &output->lastChapter, &categorie, &output->pageInfos);
		output->firstTome = -1;
		output->nombreChapitreSpeciaux = -1;
	}
	else if(version == 1)
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

/*****************		DIVERS		******************/

void resetUpdateDBCache()
{
    alreadyRefreshed = -DB_CACHE_EXPIRENCY;
}