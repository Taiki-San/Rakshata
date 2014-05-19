/*********************************************************************************************
**	__________         __           .__            __                 ________   _______   	**
**	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
**	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
**	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
**	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
**	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
**                                                                                          **
**    Licence propriétaire, code source confidentiel, distribution formellement interdite   **
**                                                                                          **
*********************************************************************************************/

#include "MDLCache.h"

void tomeDBParser(MANGAS_DATA* mangaDB, unsigned char* buffer, size_t size)
{
    size_t pos, nombreMaxElems = 0;
    META_TOME *lines = NULL;
    for(pos = 0; pos < size && buffer[pos]; pos++)
    {
        if(buffer[pos] == '\n')
            nombreMaxElems++;
    }
    if(buffer[pos-1] != '\n' || buffer[pos] != '\r')
        nombreMaxElems++;
    lines = calloc(nombreMaxElems+1, sizeof(META_TOME));

    size_t ligneCourante = 0;
	int i;
    char ligne[15+MAX_TOME_NAME_LENGTH+2*TOME_DESCRIPTION_LENGTH];
    pos = 0;
    while(ligneCourante < nombreMaxElems && pos < size)
    {
        for(i = 0; i < sizeof(ligne)-1 && pos < size && buffer[pos] != '\n' && buffer[pos] != '\r'; ligne[i++] = buffer[pos++]); //Charge la première ligne
        
		ligne[i] = 0; //i <= longueur-1

        if(buffer[pos] != '\n') //Finis la ligne
            while(pos < size && buffer[++pos] != '\n');
        for(pos++; pos < size && (buffer[pos] == '\n' || buffer[pos] == '\r') ; pos++);

        for(i = 0; i < 10 && ligne[i] && isNbr(ligne[i]); i++);
        if((ligne[i] == ' ' || !ligne[i]) && i > 0 && i < 10) //Données saines
        {
            sscanfs(ligne, "%d %s %s %s", &lines[ligneCourante].ID, lines[ligneCourante].name, MAX_TOME_NAME_LENGTH, lines[ligneCourante].description1, TOME_DESCRIPTION_LENGTH, lines[ligneCourante].description2, TOME_DESCRIPTION_LENGTH);
            escapeTomeLineElement(&lines[ligneCourante]);
			lines[ligneCourante].details = NULL;
			parseTomeDetails(*mangaDB, lines[ligneCourante].ID, &(lines[ligneCourante].details));
			ligneCourante++;
        }
    }
    if(ligneCourante == nombreMaxElems) //Aucun element invalide
        mangaDB->tomes = lines;
    else
    {
        mangaDB->tomes = malloc((ligneCourante+1)*sizeof(META_TOME));
        memcpy(mangaDB->tomes, lines, ligneCourante*sizeof(META_TOME));
        free(lines);
    }

	memset(&mangaDB->tomes[ligneCourante], 0, sizeof(META_TOME));
    qsort(mangaDB->tomes, ligneCourante, sizeof(META_TOME), sortTomes);
    mangaDB->nombreTomes = ligneCourante;

    mangaDB->tomes[ligneCourante].name[0] = 0;
    mangaDB->tomes[ligneCourante].ID = VALEUR_FIN_STRUCTURE_CHAPITRE; //Un flag final, au cas où
}

void escapeTomeLineElement(META_TOME *ligne)
{
    ///Si l'élement ne contient que _, je l'ignore
    if(ligne->name[0] == '_' && ligne->name[1] == 0)
        ligne->name[0] = 0;
    else
        changeTo((char*) ligne->name, '_', ' ');

    if(ligne->description1[0] == '_' && ligne->description1[1] == 0)
        ligne->description1[0] = 0;
    else
        changeTo((char*) ligne->description1, '_', ' ');

    if(ligne->description2[0] == '_' && ligne->description2[1] == 0)
        ligne->description2[0] = 0;
    else
        changeTo((char*) ligne->description2, '_', ' ');
}

void refreshTomeList(MANGAS_DATA *mangaDB)
{
    if(mangaDB->tomes != NULL)
	{
		free(mangaDB->tomes);
		mangaDB->tomes = NULL;
	}
	
	mangaDB->nombreTomes = 0;

    /*On commence par énumérer les chapitres spéciaux*/
    char temp[TAILLE_BUFFER];
    snprintf(temp, TAILLE_BUFFER, "manga/%s/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName, TOME_INDEX);
    FILE* tomeDB = fopen(temp, "r");
    if(tomeDB != NULL)
    {
        unsigned char *buffer = NULL;
        size_t size = getFileSize(temp);
        if(size == 0)
			return;

		buffer = calloc(1, size+1);
		if(buffer == NULL)
			memoryError(size+1);
		else
		{
			rewind(tomeDB);
			fread(buffer, size, 1, tomeDB);
		}
		fclose(tomeDB);
        if(buffer != NULL)
        {
            tomeDBParser(mangaDB, buffer, size);
            free(buffer);
        }
    }
}

bool checkTomeReadable(MANGAS_DATA mangaDB, int ID)
{
	if(mangaDB.tomes == NULL || ID >= mangaDB.nombreTomes ||  mangaDB.tomes[ID].details == NULL)
		return false;
	
	uint pos = 0, nbTomes = mangaDB.nombreTomes, posDetails;
	for(; pos < nbTomes && mangaDB.tomes[pos].ID != VALEUR_FIN_STRUCTURE_CHAPITRE && mangaDB.tomes[pos].ID != ID; pos++);
	
	if(mangaDB.tomes[pos].ID != ID)
		return false;
	
	CONTENT_TOME * cache = mangaDB.tomes[pos].details;
	char basePath[2*LONGUEUR_NOM_MANGA_MAX + 50], intermediaryDirectory[300], fullPath[2*LONGUEUR_NOM_MANGA_MAX + 350];
	
	if (cache == NULL)
		return false;
	
	snprintf(basePath, sizeof(basePath), "manga/%s/%s/", mangaDB.team->teamLong, mangaDB.mangaName);
	
	for(posDetails = 0; cache[posDetails].ID != VALEUR_FIN_STRUCTURE_CHAPITRE; posDetails++)
	{
		if(cache[posDetails].isNative)
		{
			if(cache[posDetails].ID % 10)
				snprintf(intermediaryDirectory, sizeof(intermediaryDirectory), "Chapitre_%d.%d", cache[posDetails].ID / 10, cache[posDetails].ID % 10);
			else
				snprintf(intermediaryDirectory, sizeof(intermediaryDirectory), "Chapitre_%d", cache[posDetails].ID / 10);
			
			snprintf(fullPath, sizeof(fullPath), "%s/%s/%s", basePath, intermediaryDirectory, CONFIGFILE);
			if(!checkFileExist(fullPath))
			{
				if(cache[posDetails].ID % 10)
					snprintf(intermediaryDirectory, sizeof(intermediaryDirectory), "Tome_%d/native/Chapitre_%d.%d", ID, cache[posDetails].ID / 10, cache[posDetails].ID % 10);
				else
					snprintf(intermediaryDirectory, sizeof(intermediaryDirectory), "Tome_%d/native/Chapitre_%d", ID, cache[posDetails].ID / 10);
			}
			else
			{
				snprintf(fullPath, sizeof(fullPath), "%s/%s/shared", basePath, intermediaryDirectory);
				if(!checkFileExist(fullPath))
				{
					MDL_createSharedFile(mangaDB, cache[posDetails].ID, ID);
				}
			}
		}
		else
		{
			if(cache[posDetails].ID % 10)
				snprintf(intermediaryDirectory, sizeof(intermediaryDirectory), "Tome_%d/Chapitre_%d.%d", ID, cache[posDetails].ID / 10, cache[posDetails].ID % 10);
			else
				snprintf(intermediaryDirectory, sizeof(intermediaryDirectory), "Tome_%d/Chapitre_%d", ID, cache[posDetails].ID / 10);
		}
		
		snprintf(fullPath, sizeof(fullPath), "%s/%s/%s", basePath, intermediaryDirectory, CONFIGFILE);
        if(!checkFileExist(fullPath))
            return false;
		
		snprintf(fullPath, sizeof(fullPath), "%s/%s/installing", basePath, intermediaryDirectory);
        if(checkFileExist(fullPath))
            return false;
	}
	
    return true;
}

bool parseTomeDetails(MANGAS_DATA mangaDB, int ID, CONTENT_TOME ** output)
{
	//Sanitisation de base
	if(output == NULL || ID == VALEUR_FIN_STRUCTURE_CHAPITRE)
		return false;
	
	if(*output != NULL)
	{
		free(*output);
		*output = NULL;
	}
	
	uint bufferSize, posBuf;
	char pathConfigFile[LONGUEUR_NOM_MANGA_MAX*5+350], *fileBuffer;
    FILE* config;
	
	//On charge le fichier dans un buffer en mémoire pour accélérer les IO
	snprintf(pathConfigFile, sizeof(pathConfigFile), "manga/%s/%s/Tome_%d/%s", mangaDB.team->teamLong, mangaDB.mangaName, ID, CONFIGFILETOME);
	
	bufferSize = getFileSize(pathConfigFile);
	
	if(!bufferSize || (config = fopen(pathConfigFile, "r")) == NULL)
		return false;
		
	fileBuffer = malloc(bufferSize + 1);
	if(fileBuffer == NULL)
	{
		fclose(config);
		return false;
	}
	
	posBuf = fread(fileBuffer, sizeof(char), bufferSize, config);
	fileBuffer[posBuf] = 0;
	
	fclose(config);
	
	//On commence à parser
	uint elemMax = countSpaces(fileBuffer) + 1, posOut;
	int curID;
	CONTENT_TOME * workingBuffer = malloc((elemMax + 1) * sizeof(CONTENT_TOME));
	
	if(workingBuffer == NULL)
		return false;
	
	for(posOut = posBuf = 0; fileBuffer[posBuf] && posOut < elemMax;)
	{
		curID = extractNumFromConfigTome(&fileBuffer[posBuf], ID);
		if(curID != VALEUR_FIN_STRUCTURE_CHAPITRE)
		{
			workingBuffer[posOut].ID = curID;
			workingBuffer[posOut].isNative = (fileBuffer[posBuf] == 'C');
			posOut++;
		}
		
		while(fileBuffer[posBuf])
		{
			if(fileBuffer[posBuf++] == ' ' && (fileBuffer[posBuf] == 'C' || fileBuffer[posBuf] == 'T'))
				break;
		}
	}
	
	if(posOut == 0)	//Rien n'a été lu
	{
		free(workingBuffer);
		return false;
	}

	else if(posOut < elemMax)
	{
		void* buf = realloc(workingBuffer, (posOut + 1) * sizeof(CONTENT_TOME));
		if(buf != NULL)
			workingBuffer = buf;
	}
	
	workingBuffer[posOut].ID = VALEUR_FIN_STRUCTURE_CHAPITRE;
	*output = workingBuffer;
	free(fileBuffer);
	return true;
}

void checkTomeValable(MANGAS_DATA *mangaDB, int *dernierLu)
{
	if(mangaDB->tomes == NULL)
		return;
	
    int nbElem = 0;
    char temp[LONGUEUR_NOM_MANGA_MAX*2+100];
    FILE* config = NULL;

    if(dernierLu != NULL)
    {
		snprintf(temp, sizeof(temp), "manga/%s/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName, CONFIGFILETOME);
		if((config = fopen(temp, "r")) != NULL)
		{
			*dernierLu = VALEUR_FIN_STRUCTURE_CHAPITRE;
			fscanfs(config, "%d", dernierLu);
			fclose(config);
		}
    }

    for(nbElem = 0; mangaDB->tomes[nbElem].ID != VALEUR_FIN_STRUCTURE_CHAPITRE && nbElem < mangaDB->nombreTomes; nbElem++)
    {
		//Vérifie que le tome est bien lisible
        if(!checkTomeReadable(*mangaDB, mangaDB->tomes[nbElem].ID))
        {
            mangaDB->tomes[nbElem].ID = VALEUR_FIN_STRUCTURE_CHAPITRE;
            mangaDB->tomes[nbElem].name[0] = 0;
			
			if(mangaDB->tomes[nbElem].details != NULL)
			{
				free(mangaDB->tomes[nbElem].details);
				mangaDB->tomes[nbElem].details = NULL;
			}
        }
    }

    qsort(mangaDB->tomes, nbElem, sizeof(META_TOME), sortTomes);
    for(; nbElem > 0 && mangaDB->tomes[nbElem-1].ID == VALEUR_FIN_STRUCTURE_CHAPITRE; nbElem--);
    mangaDB->nombreTomes = nbElem;
}

void getUpdatedTomeList(MANGAS_DATA *mangaDB)
{
    refreshTomeList(mangaDB);
    checkTomeValable(mangaDB, NULL);
}

void freeTomeList(META_TOME * data, bool includeDetails)
{
	if(data == NULL)
		return;

	//On ne free pas details car c'est une ressource qu'une copie du cache ne duplique pas
	if(includeDetails)
		for(uint i = 0; data[i].details != NULL; free(data[i++].details));
	
	free(data);
}

void printTomeDatas(MANGAS_DATA mangaDB, char *bufferDL, int tome)
{
    size_t length = strlen(mangaDB.team->teamLong) + strlen(mangaDB.mangaName) + 100;
    char *bufferPath = malloc(length);
    FILE* out = NULL;
    if(bufferPath != NULL)
    {
        //I create the path to the file
        snprintf(bufferPath, length, "manga/%s/%s/Tome_%d/%s.tmp", mangaDB.team->teamLong, mangaDB.mangaName, tome, CONFIGFILETOME);
        out = fopen(bufferPath, "w+");
        if(out == NULL)
        {
            createPath(bufferPath); //If I can't create the file, I try to create its path, then retry
            out = fopen(bufferPath, "w+");
            if(out == NULL)
                return;
        }
        if(fwrite(bufferDL, strlen(bufferDL), 1, out) != 1) //Write data then check if everything went fine
        {
            logR("Failed at write tome infos");
#ifdef DEV_VERSION
            logR(bufferDL);
#endif
        }
        fclose(out);
        free(bufferPath);
    }
}

int extractNumFromConfigTome(char *input, int ID)
{
    int output = VALEUR_FIN_STRUCTURE_CHAPITRE, posDebut = 0;
    char basePath[100];

    if(!strncmp(input, "Chapitre_", 9))
        posDebut = 9;
    else
	{
		snprintf(basePath, 100, "Tome_%d/Chapitre_", ID);
		if(!strncmp(input, basePath, strlen(basePath)))
			posDebut = strlen(basePath);
	}

    if(posDebut)
    {
        int i = sscanfs(&input[posDebut], "%d", &output);
        output *= 10;

        if(input[posDebut+i] == '.' && isNbr(input[posDebut+i+1]))
            output += (int) input[posDebut+i+1] - '0';
    }
    return output;
}

void internalDeleteTome(MANGAS_DATA mangaDB, int tomeDelete, bool careAboutLinkedChapters)
{
	uint length = strlen(mangaDB.team->teamLong) + strlen(mangaDB.mangaName) + 50, position = tomeDelete, limit;
    char dir[length];
	
	if(!mangaDB.nombreTomes)	//Si pas de tome dispo, cette fonction a aucun intérêt
	{
#ifdef DEV_VERSION
		logR("Incoherency when deleting volumes");
#endif
		return;
	}
	
	if(mangaDB.tomes[tomeDelete].ID != tomeDelete)
	{
		if(tomeDelete >= mangaDB.nombreTomes)
		{
			position = tomeDelete = mangaDB.nombreTomes - 1;
			limit = 0;
		}
		else if(mangaDB.tomes[tomeDelete].ID > tomeDelete)
			limit = 0;
		else
			limit = mangaDB.nombreTomes;
		
		if(limit == 0)
			for(position++; position > 0 && mangaDB.tomes[--position].ID > tomeDelete;);	//Gérer l'unsigned
		else
			for (; position < limit && mangaDB.tomes[position].ID < tomeDelete; position++);
	}
	
	if(mangaDB.tomes[position].ID == tomeDelete)
	{
		int curID;
		char basePath[2*LONGUEUR_NOM_MANGA_MAX + 50], dirToChap[2*LONGUEUR_NOM_MANGA_MAX + 100];
		CONTENT_TOME * details = mangaDB.tomes[position].details;
		
		snprintf(basePath, sizeof(basePath), "manga/%s/%s", mangaDB.team->teamLong, mangaDB.mangaName);
		
		for(uint posDetails = 0; details[posDetails].ID != VALEUR_FIN_STRUCTURE_CHAPITRE; posDetails++)
		{
			if(details[posDetails].isNative)
			{
				curID = details[posDetails].ID;
				if (curID % 10)
					snprintf(dirToChap, sizeof(dirToChap), "%s/Chapitre_%d.%d/shared", basePath, curID / 10, curID % 10);
				else
					snprintf(dirToChap, sizeof(dirToChap), "%s/Chapitre_%d/shared", basePath, curID / 10);
				
				if(checkFileExist(dirToChap))
					remove(dirToChap);
			}
		}
	}
	
    snprintf(dir, length, "manga/%s/%s/Tome_%d/", mangaDB.team->teamLong, mangaDB.mangaName, tomeDelete);
	removeFolder(dir);
}
