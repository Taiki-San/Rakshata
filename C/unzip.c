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

static bool do_list(unzFile uf, bool *encrypted, char filename_inzip[NOMBRE_PAGE_MAX][256])
{
    unz_global_info64 gi;
    int err;

    err = unzGetGlobalInfo64(uf,&gi);
    if (err!=UNZ_OK)
	{
#ifdef DEV_VERSION
	    char temp[100];
		sprintf(temp, "error %d with zipfile in unzGetGlobalInfo \n",err);
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
			snprintf(temp, 100, "error %d with zipfile in unzGetCurrentFileInfo", err);
            logR(temp);
            break;
        }

        ustrcpy(filename_inzip[i], filename);

        if ((file_info.flag & 1) != 0) //Si chiffré
            *encrypted = true;

        if (i + 1 < gi.number_entry)
        {
            err = unzGoToNextFile(uf);
            if (err != UNZ_OK)
            {
                char temp[100];
                snprintf(temp, 100, "error %d with zipfile in unzGetCurrentFileInfo", err);
                logR(temp);
                break;
            }
        }
    }
    return true;
}

bool miniunzip (void *inputData, char *outputZip, char *passwordZip, size_t size, size_t type) //Type définit si l'extraction est standard ou pas
{
	bool extractWithoutPath = false, encrypted = false, ret_value = true;
	char *zipInput = NULL, *zipFileName = NULL, *zipOutput = NULL, *password = NULL;
    char *pathToConfigFile = NULL;
	size_t lengthPath;

    unzFile uf = NULL;
    zlib_filefunc_def fileops;

	char *path = NULL;

	int nombreFichiers = 0, nombreFichiersDecompresses = 0;
    char filename_inzip[NOMBRE_PAGE_MAX][256]; //Recevra la liste de tous les fichiers

    if(size) //Si extraction d'un chapitre
        extractWithoutPath = true;
    else
	{
		zipInput = inputData;
		zipFileName = ralloc((strlen(zipInput)+1) *2); //Input
	}
    zipOutput = ralloc((strlen(outputZip)+1) *2); //Output
	if(passwordZip != NULL)		password = ralloc((strlen(passwordZip)+1) *2);

    if((!size && zipFileName == NULL) || zipOutput == NULL || (password == NULL && passwordZip != NULL))
    {
        free(zipFileName);
        free(zipOutput);
        free(password);
#ifdef DEV_VERSION
        logR("Failed at allocate memory\n");
#endif
        return 1;
    }

    for(uint i = 0, j = 0; zipInput != NULL && i < (strlen(zipInput)+1)*2; i++, j++)
    {
        if(i < (strlen(zipInput)+1)*2 && zipInput[i] != '\0')
            zipFileName[i] = zipInput[i];
        else
        {
            if(zipFileName[i - 1] != 'p' && i + 5 < (strlen(zipInput)+1)*2) //Si il manque .zip
            {
                zipFileName[i++] = '.';
                zipFileName[i++] = 'z';
                zipFileName[i++] = 'i';
                zipFileName[i++] = 'p';
                zipFileName[i++] = '\0';
            }
            else if (i < (strlen(zipInput)+1)*2)
            {
                zipFileName[i] = 0;
                i = (strlen(zipInput)+1)*2;
            }
        }
    }
    memcpy(zipOutput, outputZip, strlen(outputZip)+1);
	
    if(password != NULL)
        memcpy(password, passwordZip, strlen(passwordZip)+1);
	
    if(!size)
    {
		lengthPath = strlen(zipFileName) + strlen(outputZip) + 505;	//500 for currentWorkingDirectory + 2
        path = malloc(lengthPath);

        if (checkFileExist(zipFileName))
        {
            char temp[500];
            snprintf(temp, 500, "Can't open %s\n", zipFileName);
            logR(temp);
            goto quit;
        }

        uf = unzOpen(zipFileName);

        if(uf == NULL)
        {
			char currentWorkingDirectory[512];
			getcwd(currentWorkingDirectory, sizeof(currentWorkingDirectory));
            snprintf(path, lengthPath, "%s/%s", currentWorkingDirectory, zipFileName);
            uf = unzOpen(path);
        }
    }
    else
    {
		lengthPath = strlen(outputZip);
        path = ralloc(strlen(outputZip) + 3);
        init_zmemfile(&fileops, ((DATA_DL_OBFS*)inputData)->data, ((DATA_DL_OBFS*)inputData)->mask, size);
        uf = unzOpen2(NULL, &fileops);
    }

	strncat(path, outputZip, lengthPath);
    if (size && !checkDirExist(path)) //On change le dossier courant
    {
        createPath(outputZip); //En cas d'échec, on réessaie de créer le dossier
        if (!checkDirExist(path)) //Si réechoue
        {
            char temp[1000];
            snprintf(temp, 1000, "Error changing into %s, aborting\n", outputZip);
            logR(temp);
            goto quit;
        }
    }

    pathToConfigFile = ralloc(strlen(path) + 50);
    snprintf(pathToConfigFile, strlen(path) + 50, "%s/%s", path, CONFIGFILE);
    if(checkFileExist(pathToConfigFile))
        goto quit;

    for(uint i = 0; i < NOMBRE_PAGE_MAX; i++) //Réinitialise les noms de page
        for(uint j = 0; j < 256; filename_inzip[i][j++] = 0);

    ret_value &= do_list(uf, &encrypted, filename_inzip); //On lit l'archive
	if(!ret_value)
		goto quit;

    for(nombreFichiers = 0; nombreFichiers < NOMBRE_PAGE_MAX && filename_inzip[nombreFichiers][0] != 0; nombreFichiers++); //Nombre de fichier dans l'archive ZIP

    unsigned char pass[NOMBRE_PAGE_MAX][SHA256_DIGEST_LENGTH]; //Les pass des fichiers

    for(uint i = 0; i < NOMBRE_PAGE_MAX; i++)
        for(uint j = 0; j < SHA256_DIGEST_LENGTH; pass[i][j++] = 0);

    if(size)
    {
        if(!encrypted)
            encrypted = FULL_ENCRYPTION;
    }
    for(uint i = 0; i < nombreFichiers; i++)
    {
        if(checkNameFileZip(filename_inzip[i]))
        {
			if(encrypted)	//password can be NULL
                ret_value &= do_extract_onefile(uf, filename_inzip[i], path, extractWithoutPath, 1, password, pass[i]);
            else
                ret_value &= do_extract_onefile(uf, filename_inzip[i], path, extractWithoutPath, 1, NULL, NULL);
			
            nombreFichiersDecompresses++;

            if (i + 1 < nombreFichiers)
            {
                if (unzGoToNextFile(uf) != UNZ_OK)
                    break;
            }
        }
        else
        {
            if (filename_inzip[0] && filename_inzip[i][strlen(filename_inzip[i])-1] != '/' && (i+1) < nombreFichiers)
            {
                if (unzGoToNextFile(uf) != UNZ_OK)
                    break;
            }
            for(uint j = 0; j < 256; filename_inzip[i][j++] = 0);
        }
    }

    if(encrypted)
    {
        /*On va écrire les clées dans un config.enc
          Pour ça, on va classer les clées avec la longueur des clées
          il est un peu 3h du mat donc je vais pas m'attarder*/
		
        int nombreFichierDansConfigFile = 0;
        char **nomPage = NULL;
		unsigned char temp[256], *hugeBuffer = NULL;

        nombreFichiers--;

        for(uint i = 0, j, k = 0; i < NOMBRE_PAGE_MAX; i++) //On vire les paths des noms de fichiers
        {
            if(!filename_inzip[i][0])
                continue;
			
			j = strlen(filename_inzip[i]);
            for(; j > 0 && filename_inzip[i][j] != '/'; j--);
            if(j)
			{
				for(j++; filename_inzip[i][j] != 0 && j < 256; filename_inzip[i][k++] = filename_inzip[i][j++]);
				for(; k < 256; filename_inzip[i][k++] = 0);
			}
        }

        /*On va classer les fichier et les clées en ce basant sur config.dat*/

        if((nomPage = loadChapterConfigDat(pathToConfigFile, &nombreFichierDansConfigFile)) == NULL || (nombreFichierDansConfigFile != nombreFichiersDecompresses-2 && nombreFichierDansConfigFile != nombreFichiersDecompresses-1)) //-2 car -1 + un décallage de -1 du Ã  l'optimisation pour le lecteur
        {
#ifdef DEV_VERSION
            logR("config.dat invalid: encryption aborted.\n");
#endif
            for(uint i = 0; filename_inzip[i][0]; remove(filename_inzip[i++])); //On fais le ménage
            if(nomPage != NULL)
            {
                for(uint i = 0; nomPage[i]; free(nomPage[i++]));
                free(nomPage);
            }
            ret_value = 1;
            goto quit;
        }

        for(uint i = 0; i < NOMBRE_PAGE_MAX; i++)
		{
			if(!strcmp(filename_inzip[i], CONFIGFILE)) //On vire les clées du config.dat
			{
				for(uint j = 0; filename_inzip[i][j] && j < 256; filename_inzip[i][j++] = 0);
				for(uint j = 0; j < SHA256_DIGEST_LENGTH; pass[i][j++] = 0);
			}
		}

		nombreFichiers--;

        for(uint j = 0, i = 0; i < NOMBRE_PAGE_MAX; i++, j++) //on consolide la liste des noms (fichiers invalides dégagés)
        {
            MajToMin(filename_inzip[i]);
            if(filename_inzip[i][0] < ' ')//Fichier manquant
            {
                for(; filename_inzip[i][0] < ' ' && i < NOMBRE_PAGE_MAX; i++);

				if(i >= NOMBRE_PAGE_MAX)
                    break;
				
				MajToMin(filename_inzip[i]);
                ustrcpy(filename_inzip[j], filename_inzip[i]);
                crashTemp(filename_inzip[i], 256);
                memcpy(pass[j], pass[i], SHA256_DIGEST_LENGTH);
                crashTemp(pass[i], SHA256_DIGEST_LENGTH);
                i--;
            }
        }

        for(uint j,  i = 0; i <= nombreFichiers && filename_inzip[i][0] != 0; i++)
        {
            MajToMin(nomPage[i]);
            for(j = 0; strcmp(nomPage[i], filename_inzip[j]) && filename_inzip[j][0] && j < nombreFichiers; j++);

            if(j != i && !strcmp(nomPage[i], filename_inzip[j])) //Mauvais classement
            {
				strncpy((char*) temp, filename_inzip[i], sizeof(temp)); //On déplace les noms
                strncpy(filename_inzip[i], filename_inzip[j], sizeof(filename_inzip[i]));
				strncpy(filename_inzip[j], (char*) temp, sizeof(filename_inzip[j])); //On déplace les noms

                memcpy(temp, pass[i], SHA256_DIGEST_LENGTH); //On déplace les clées
                memcpy(pass[i], pass[j], SHA256_DIGEST_LENGTH);
                memcpy(pass[j], temp, SHA256_DIGEST_LENGTH);
                crashTemp(temp, SHA256_DIGEST_LENGTH);
            }
        }
        free(nomPage);
		
        hugeBuffer = malloc(((SHA256_DIGEST_LENGTH+1) * (nombreFichiers + 1) + 15) * sizeof(unsigned char));
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
            int k = 0;
            hugeBuffer[j++] = ' ';
            for(; k < SHA256_DIGEST_LENGTH; hugeBuffer[j++] = pass[i][k++]);
			hugeBuffer[j] = 0;
        }

        if(!getMasterKey(temp))
        {
            uint8_t hash[SHA256_DIGEST_LENGTH], chapter[15];
#ifndef __APPLE__
            snprintf((char *)chapter, 15, "%d", type);
#else
            snprintf((char *)chapter, 15, "%ld", type);
#endif

			internal_pbkdf2(SHA256_DIGEST_LENGTH, temp, SHA256_DIGEST_LENGTH, chapter, ustrlen(chapter), 512, PBKDF2_OUTPUT_LENGTH, hash);

            crashTemp(temp, 256);
            snprintf(pathToConfigFile, strlen(path) + 50, "%s/config.enc", path);
            _AESEncrypt(hash, hugeBuffer, pathToConfigFile, INPUT_IN_MEMORY, 1);
            crashTemp(hash, SHA256_DIGEST_LENGTH);
            crashTemp(hugeBuffer, (SHA256_DIGEST_LENGTH+1)*(nombreFichiers+1) + 15); //On écrase pour que ça soit plus chiant à lire
        }
		else //delete chapter
		{
			for(uint i = 0; filename_inzip[i][0]; remove(filename_inzip[i++]));
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
    free(password);
	
	return ret_value;
}

