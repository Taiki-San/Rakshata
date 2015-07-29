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

#include "../unzip/unz_memory.c"

//Utils
bool checkNameFileZip(char * fileToTest)
{
	if(!strncmp(fileToTest, "__MACOSX", 8) || !strncmp(fileToTest, ".DS_Store", 9))	//Dossier parasite de OSX
		return false;
	
	//strlen(fileToTest) - 1 est le dernier caractère, strlen(fileToTest) donnant la longueur de la chaine
	uint posLastChar = strlen(fileToTest) - 1;
	
	if(fileToTest[posLastChar] == '/' 		//Si c'est un dossier, le dernier caractère est /
	   || (fileToTest[posLastChar - 3] == '.' && fileToTest[posLastChar - 2] == 'e' && fileToTest[posLastChar - 1] == 'x' && fileToTest[posLastChar] == 'e'))	//.exe
		return false;
	
	return true;
}

void minimizeString(char* input)
{
	for(; *input; input++)
	{
		if(*input >= 'A' && *input <= 'Z')
			*input += 'a' - 'A';
	}
}

//Zip routines
bool decompressChapter(void *inputData, size_t sizeInput, char *outputPath, PROJECT_DATA project, int entryDetail)
{
	if(inputData == NULL || outputPath == NULL)
		return false;

	bool ret_value = true;

	uint lengthOutput = strlen(outputPath) + 1, nombreFichiers = 0;
	char ** filename = NULL, *pathToConfigFile = malloc(lengthOutput + 50);

	//Init unzip file
	zlib_filefunc_def fileops;
	init_zmemfile(&fileops, ((DATA_DL_OBFS*)inputData)->data, ((DATA_DL_OBFS*)inputData)->mask, sizeInput);
	unzFile zipFile = unzOpen2(NULL, &fileops);

	if(pathToConfigFile == NULL || zipFile == NULL)
		goto quit;

	snprintf(pathToConfigFile, lengthOutput + 50, "%s/"CONFIGFILE, outputPath);

	//We create if required the path
	if(!checkDirExist(outputPath))
    {
        createPath(outputPath);
        if(!checkDirExist(outputPath))
        {
            char temp[lengthOutput + 100];
            snprintf(temp, sizeof(temp), "Error creating path %s", outputPath);
            logR(temp);
            goto quit;
        }
    }
	else if(checkFileExist(pathToConfigFile))		//Ensure the project is not already installed
        goto quit;

	//List files
    ret_value &= listArchiveContent(zipFile, &filename, &nombreFichiers);
	if(ret_value)
	{
		uint nombreFichierValide = 0;

		//Mot de pass des fichiers si la DRM est active
		unsigned char pass[nombreFichiers][SHA256_DIGEST_LENGTH];
		crashTemp(pass, sizeof(pass));

		//Decompress files
		unzGoToFirstFile(zipFile);
		for(uint i = 0; i < nombreFichiers && ret_value; i++)
		{
			//Name is valid
			if(checkNameFileZip(filename[i]))
			{
				ret_value &= extractOnefile(zipFile, filename[i], outputPath, STRIP_PATH_ALL, project.haveDRM ? pass[i] : NULL);
				nombreFichierValide++;
			}
			else
			{
				free(filename[i]);
				filename[i] = NULL;
			}

			//Go to next file if needed
			if(i + 1 < nombreFichiers)
			{
				if(unzGoToNextFile(zipFile) != UNZ_OK)
					break;
			}
		}

		if(ret_value & project.haveDRM)
		{
			/*On va écrire les clées dans un config.enc
			 Pour ça, on va classer les clées en fonction des pages, retirer les éléments invalides, puis on chiffre tout ce beau monde*/

			uint nombreFichierDansConfigFile = 0;
			char **nomPage = NULL;
			byte temp[256];

			//On vire les paths des noms de fichiers
			for(uint i = 0, j, k; i < nombreFichiers; i++)
			{
				if(filename[i] == NULL)
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
				if(filename[i] != NULL && !strcmp(filename[i], CONFIGFILE)) //On vire la clées du config.dat
				{
					free(filename[i]);
					filename[i] = NULL;

					do
					{
						memcpy(&(pass[i]), &(pass[i + 1]), sizeof(pass[0]));
					} while(++i < nombreFichiers - 1);

					nombreFichierValide--;

					break;
				}
			}

			//On va classer les fichier et les clées en ce basant sur config.dat
			if((nomPage = loadChapterConfigDat(pathToConfigFile, &nombreFichierDansConfigFile)) == NULL || (nombreFichierDansConfigFile != nombreFichierValide && nombreFichierDansConfigFile != nombreFichierValide-1))
			{
#ifdef DEV_VERSION
				logR("config.dat invalid: encryption aborted.\n");
#endif

				removeFolder(outputPath);

				if(nomPage != NULL)
				{
					for(uint i = 0; nombreFichierDansConfigFile; free(nomPage[i++]));
					free(nomPage);
				}
				ret_value = false;
				goto quit;
			}

#ifdef MAINTAIN_SUPER_LEGACY_COMPATIBILITY
			//Some legacy archive contain complexe file name, so we have to decapitalize them
			for(uint i = 0; filename[i][0]; minimizeString(filename[i++]));
#endif

			//Classement
			for(uint j, i = 0; i < nombreFichiers; i++)
			{
				if(filename[i] == NULL)
					continue;

				minimizeString(nomPage[i]);

				for(j = i; j < nombreFichiers; j++)
				{
					if(filename[j] != NULL && !strcmp(nomPage[i], filename[j]))
						break;
				}

				if(j != i && j < nombreFichiers) //Mauvais classement
				{
					void * entry = filename[i];
					filename[i] = filename[j];
					filename[j] = entry;

					char swapItem[SHA256_DIGEST_LENGTH];

					memcpy(swapItem, pass[i], SHA256_DIGEST_LENGTH); //On déplace les clées
					memcpy(pass[i], pass[j], SHA256_DIGEST_LENGTH);
					memcpy(pass[j], swapItem, SHA256_DIGEST_LENGTH);
					crashTemp(swapItem, SHA256_DIGEST_LENGTH);
				}
			}

			for(uint i = 0; i < nombreFichierDansConfigFile; free(nomPage[i++]));
			free(nomPage);

			//Global encryption buffer
			byte * hugeBuffer = malloc(((SHA256_DIGEST_LENGTH + 1) * nombreFichierValide + 15 + CRYPTO_BUFFER_SIZE) * sizeof(byte));
			if(hugeBuffer == NULL)
			{
#ifdef DEV_VERSION
				logR("Failed at allocate memory to buffer\n");
#endif
				memoryError((SHA256_DIGEST_LENGTH + 1) * nombreFichierValide + 15 + CRYPTO_BUFFER_SIZE);
				removeFolder(outputPath);
				ret_value = false;
				goto quit;
			}

			//Add the number of entries at the begining of the said buffer
			int sizeWritten = sprintf((char *) hugeBuffer, "%d", nombreFichierValide);
			uint posBlob = sizeWritten > 0 ? (uint) sizeWritten : 0;

			//Inject the keys in the buffer
			for(uint i = 0; i < nombreFichiers; i++)
			{
				if(filename[i] != NULL)
				{
					hugeBuffer[posBlob++] = ' ';
					for(short keyPos = 0; keyPos < SHA256_DIGEST_LENGTH; hugeBuffer[posBlob++] = pass[i][keyPos++]);
				}
			}

			//We generate the masterkey
			if(getMasterKey(temp) == GMK_RETVAL_OK && COMPTE_PRINCIPAL_MAIL != NULL)
			{
				uint lengthEmail = strlen(COMPTE_PRINCIPAL_MAIL);
				if(lengthEmail != 0)
				{
					//We want to copy COMPTE_PRINCIPAL_MAIL ASAP in order to prevent TOCTOU
					char * encodedEmail[lengthEmail * 2 + 1];
					decToHex((void*) COMPTE_PRINCIPAL_MAIL, lengthEmail, (char*) encodedEmail);

					snprintf(pathToConfigFile, lengthOutput + 50, "%s/"DRM_FILE, outputPath);
					FILE * output = fopen(pathToConfigFile, "wb");
					if(output != NULL)
					{
						fputs((char*) encodedEmail, output);
						fputc('\n', output);

						uint8_t hash[SHA256_DIGEST_LENGTH], chapter[10];
						snprintf((char *)chapter, sizeof(chapter), "%d", entryDetail);

						internal_pbkdf2(SHA256_DIGEST_LENGTH, temp, SHA256_DIGEST_LENGTH, chapter, ustrlen(chapter), 512, PBKDF2_OUTPUT_LENGTH, hash);

						crashTemp(temp, sizeof(temp));

						_AES(hash, hugeBuffer, posBlob, hugeBuffer, EVERYTHING_IN_MEMORY, AES_ENCRYPT, AES_ECB);
						crashTemp(hash, SHA256_DIGEST_LENGTH);

						//We want to write the end of the block
						if(posBlob % CRYPTO_BUFFER_SIZE)
							posBlob += CRYPTO_BUFFER_SIZE - (posBlob % CRYPTO_BUFFER_SIZE);

						fwrite(hugeBuffer, posBlob, 1, output);
						fclose(output);
					}
					else //delete chapter
					{
						crashTemp(temp, sizeof(temp));
						removeFolder(outputPath);
					}
				}
				else //delete chapter
				{
					crashTemp(temp, sizeof(temp));
					removeFolder(outputPath);
				}
			}
			else //delete chapter
			{
				crashTemp(temp, sizeof(temp));
				removeFolder(outputPath);
			}

			free(hugeBuffer);
		}
	}

quit:

	if(filename != NULL)
	{
		for(uint i = 0; i < nombreFichiers; free(filename[i++]));
		free(filename);
	}

    unzClose(zipFile);
	destroy_zmemfile(&fileops);
    free(pathToConfigFile);

	return ret_value;
}

