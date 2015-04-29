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

#include "crypto.h"
#include "unzip/miniunzip.h"
#include "unzip/unz_memory.c"

#define NOMBRE_PAGE_MAX 500 //A dégager au prochain refactoring

static bool do_list(unzFile uf, char filenameInzip[NOMBRE_PAGE_MAX][256])
{
    unz_global_info64 gi;
    int err;

    err = unzGetGlobalInfo64(uf, &gi);
    if (err!=UNZ_OK)
	{
#ifdef DEV_VERSION
	    char temp[100];
		snprintf(temp, sizeof(temp), "error %d with zipfile in unzGetGlobalInfo", err);
		logR(temp);
#endif
		return false;
	}

	for (uint i = 0; i < gi.number_entry; i++)
    {
        char filename[256];
        unz_file_info64 file_info;

        crashTemp(filename, sizeof(filename));

        err = unzGetCurrentFileInfo64(uf, &file_info, filename, sizeof(filename), NULL, 0, NULL, 0); //Get name -> 99% of what I need
        if (err != UNZ_OK)
        {
            char temp[100];
			snprintf(temp, sizeof(temp), "error %d with zipfile in unzGetCurrentFileInfo", err);
            logR(temp);
            break;
        }

		usstrcpy(filenameInzip[i], ustrlen(filename) + 1, filename);

        if (i + 1 < gi.number_entry)
        {
            err = unzGoToNextFile(uf);
            if (err != UNZ_OK)
            {
                char temp[100];
                snprintf(temp, sizeof(temp), "error %d with zipfile in unzGetCurrentFileInfo", err);
                logR(temp);
                break;
            }
        }
    }
    return true;
}

bool miniunzip(void *inputData, char *outputZip, PROJECT_DATA project, size_t size, size_t type) //Type définit si l'extraction est standard ou pas
{
	bool extractWithoutPath = false, ret_value = true;
	char *zipInput = NULL, *zipFileName = NULL, *zipOutput = NULL;
    char *pathToConfigFile = NULL;
	size_t lengthPath, lengthInput = 0, lengthOutput = 0;

    unzFile uf = NULL;
    zlib_filefunc_def fileops;

	char *path = NULL;

	if(inputData == NULL || outputZip == NULL)
		return false;
	else
		lengthOutput = strlen(outputZip) + 1;

	uint nombreFichiers = 0, nombreFichiersDecompresses = 0;
    char filename[NOMBRE_PAGE_MAX][256]; //Recevra la liste de tous les fichiers

    if(size) //Si extraction d'un chapitre
        extractWithoutPath = true;
    else
	{
		zipInput = inputData;
		lengthInput = zipInput != NULL ? strlen(zipInput) : 0;
		zipFileName = ralloc(lengthInput + 5); //Input
	}
	
    zipOutput = ralloc(lengthOutput);

	//Allocation error
    if((!size && zipFileName == NULL) || zipOutput == NULL)
    {
        free(zipFileName);
        free(zipOutput);
#ifdef DEV_VERSION
        logR("Failed at allocate memory");
#endif
        return false;
    }
	else	//Copy data
		memcpy(zipOutput, outputZip, lengthOutput);
	
	if(zipInput != NULL)
	{
		memcpy(zipFileName, zipInput, lengthInput);
		if(zipFileName[lengthInput - 1] != 'p')
		{
			if(lengthInput + 5 < 5)
				return false;
			
			zipFileName[lengthInput++] = '.';
			zipFileName[lengthInput++] = 'z';
			zipFileName[lengthInput++] = 'i';
			zipFileName[lengthInput++] = 'p';
			zipFileName[lengthInput++] = 0;
		}
	}

    if(!size)
    {
		lengthPath = lengthInput + lengthOutput + 505;	//500 for currentWorkingDirectory + 2
        path = malloc(lengthPath);

        uf = unzOpen(zipFileName);

        if(uf == NULL)
        {
			char currentWorkingDirectory[512];
			getcwd(currentWorkingDirectory, sizeof(currentWorkingDirectory));
            snprintf(path, lengthPath, "%s/%s", currentWorkingDirectory, zipFileName);
            uf = unzOpen(path);
			
			if(uf == NULL)
			{
				char temp[lengthInput + 64];
				snprintf(temp, sizeof(temp), "Can't open %s", zipFileName);
				logR(temp);
				goto quit;
			}
        }
    }
    else
    {
		lengthPath = lengthOutput;
        path = ralloc(lengthOutput + 3);
        init_zmemfile(&fileops, ((DATA_DL_OBFS*)inputData)->data, ((DATA_DL_OBFS*)inputData)->mask, size);
        uf = unzOpen2(NULL, &fileops);
    }

	strncat(path, outputZip, lengthPath);
    if(size && !checkDirExist(path)) //On change le dossier courant
    {
        createPath(outputZip); //En cas d'échec, on réessaie de créer le dossier
        if (!checkDirExist(path)) //Si réechoue
        {
            char temp[lengthPath + 100];
            snprintf(temp, sizeof(temp), "Error changing into %s, aborting", outputZip);
            logR(temp);
            goto quit;
        }
    }

    pathToConfigFile = ralloc(strlen(path) + 50);
    snprintf(pathToConfigFile, strlen(path) + 50, "%s/%s", path, CONFIGFILE);
    if(checkFileExist(pathToConfigFile))
        goto quit;

	crashTemp(filename, sizeof(filename));
	
	//List files
    ret_value &= do_list(uf, filename);
	if(!ret_value)		goto quit;

	//Count files inside archive
    for(nombreFichiers = 0; nombreFichiers < NOMBRE_PAGE_MAX && filename[nombreFichiers][0] != 0; nombreFichiers++);

	//Passes
    unsigned char pass[NOMBRE_PAGE_MAX][SHA256_DIGEST_LENGTH];
	crashTemp(pass, sizeof(pass));

	for(uint i = 0; i < nombreFichiers; i++)
    {
        if(checkNameFileZip(filename[i]))
        {
			ret_value &= do_extract_onefile(uf, filename[i], path, extractWithoutPath, project.haveDRM ? pass[i] : NULL);
            nombreFichiersDecompresses++;

            if(i + 1 < nombreFichiers)
            {
                if(unzGoToNextFile(uf) != UNZ_OK)
                    break;
            }
        }
        else
        {
            if(filename[i][0] && filename[i][strlen(filename[i])-1] != '/' && i + 1 < nombreFichiers)
            {
                if(unzGoToNextFile(uf) != UNZ_OK)
                    break;
            }
			
			memcpy(&filename[i], &filename[i+1], (nombreFichiers - i - 1) * sizeof(filename[0]));
			nombreFichiers--;
			i--;
        }
    }

    if(project.haveDRM)
    {
        /*On va écrire les clées dans un config.enc
          Pour ça, on va classer les clées avec la longueur des clées, les nettoyer et les chiffrer*/
		
        int nombreFichierDansConfigFile = 0;
        char **nomPage = NULL;
		unsigned char temp[256], *hugeBuffer = NULL;
		
		for(uint i = 0, j, k; i < nombreFichiers; i++) //On vire les paths des noms de fichiers
        {
            if(!filename[i][0])
                continue;
			
			j = strlen(filename[i]);
            for(; j > 0 && filename[i][j] != '/'; j--);
            if(j)
			{
				for(k = 0, j++; filename[i][j] != 0 && j < 256; filename[i][k++] = filename[i][j++]);
				filename[i][k] = 0;
			}
        }

		for(uint i = 0; i < nombreFichiers; i++)
		{
			if(!strcmp(filename[i], CONFIGFILE)) //On vire les clées du config.dat
			{
				memcpy(&filename[i], &filename[i+1], (nombreFichiers - i - 1) * sizeof(filename[0]));
				memcpy(&pass[i], &pass[i+1], (nombreFichiers - i - 1) * sizeof(pass[0]));
				nombreFichiers--;
				break;
			}
		}
		
        /*On va classer les fichier et les clées en ce basant sur config.dat*/

        if((nomPage = loadChapterConfigDat(pathToConfigFile, &nombreFichierDansConfigFile)) == NULL || (nombreFichierDansConfigFile != nombreFichiersDecompresses-2 && nombreFichierDansConfigFile != nombreFichiersDecompresses-1)) //-2 car -1 + un décallage de -1 due à l'optimisation pour le lecteur
        {
#ifdef DEV_VERSION
            logR("config.dat invalid: encryption aborted.\n");
#endif
            for(uint i = 0; filename[i][0]; remove(filename[i++])); //On fais le ménage
            if(nomPage != NULL)
            {
                for(uint i = 0; nomPage[i]; free(nomPage[i++]));
                free(nomPage);
            }
            ret_value = false;
            goto quit;
        }
		
#ifdef MAINTAIN_SUPER_LEGACY_COMPATIBILITY
		//Some legacy archive contain complexe file name, so we have to decapitalize them
		for(uint i = 0; filename[i][0]; MajToMin(filename[i++]));
#endif
		
		//Classement
		for(uint j, i = 0; i <= nombreFichiers && filename[i][0] != 0; i++)
        {
			MajToMin(nomPage[i]);

			for(j = 0; strcmp(nomPage[i], filename[j]) && filename[j][0] && j < nombreFichiers; j++);

            if(j != i && !strcmp(nomPage[i], filename[j])) //Mauvais classement
            {
				strncpy((char*) temp, filename[i], sizeof(temp)); //On déplace les noms
                strncpy(filename[i], filename[j], sizeof(filename[i]));
				strncpy(filename[j], (char*) temp, sizeof(filename[j])); //On déplace les noms

                memcpy(temp, pass[i], SHA256_DIGEST_LENGTH); //On déplace les clées
                memcpy(pass[i], pass[j], SHA256_DIGEST_LENGTH);
                memcpy(pass[j], temp, SHA256_DIGEST_LENGTH);
                crashTemp(temp, SHA256_DIGEST_LENGTH);
            }
        }
        free(nomPage);
		
		hugeBuffer = malloc(((SHA256_DIGEST_LENGTH+1) * (nombreFichiers + 1) + 15) * sizeof(byte));
        if(hugeBuffer == NULL)
        {
#ifdef DEV_VERSION
            logR("Failed at allocate memory to buffer\n");
#endif
            quit_thread(0); //Libérer la mémoire serait pas mal
        }
        sprintf((char *) hugeBuffer, "%d", nombreFichiers);
        for(uint i = 0, j = ustrlen(hugeBuffer); i <= nombreFichiers; i++) //Write config.enc
        {
            hugeBuffer[j++] = ' ';
            for(short keyPos = 0; keyPos < SHA256_DIGEST_LENGTH; hugeBuffer[j++] = pass[i][keyPos++]);
			hugeBuffer[j] = 0;
        }

        if(getMasterKey(temp) == GMK_RETVAL_OK)
        {
            uint8_t hash[SHA256_DIGEST_LENGTH], chapter[15];
#ifndef __APPLE__
            snprintf((char *)chapter, 15, "%d", type);
#else
            snprintf((char *)chapter, 15, "%ld", type);
#endif

			internal_pbkdf2(SHA256_DIGEST_LENGTH, temp, SHA256_DIGEST_LENGTH, chapter, ustrlen(chapter), 512, PBKDF2_OUTPUT_LENGTH, hash);

            crashTemp(temp, 256);
            snprintf(pathToConfigFile, strlen(path) + 50, "%s/"DRM_FILE, path);
            _AESEncrypt(hash, hugeBuffer, pathToConfigFile, INPUT_IN_MEMORY, 1);
            crashTemp(hash, SHA256_DIGEST_LENGTH);
            crashTemp(hugeBuffer, (SHA256_DIGEST_LENGTH+1)*(nombreFichiers+1) + 15); //On écrase pour que ça soit plus chiant à lire
        }
		else //delete chapter
		{
			for(uint i = 0; filename[i][0]; remove(filename[i++]));
		}
		
        free(hugeBuffer);
    }

quit:

    unzClose(uf);
    if(size)
        destroy_zmemfile(&fileops);

    free(path);
    free(pathToConfigFile);
    free(zipFileName);
    free(zipOutput);
	
	return ret_value;
}

