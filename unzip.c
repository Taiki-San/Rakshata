/*********************************************
**	        	 Rakshata v1.1 		        **
**     Licence propriétaire, code source    **
**        confidentiel, distribution        **
**          formellement interdite          **
**********************************************/

#include "main.h"
#include "unzip/miniunzip.h"

#include "unzip/unz_memory.c"

int INSTALL_DONE;
int CURRENT_TOKEN;

int do_list(unzFile uf, int *encrypted, char filename_inzip[NOMBRE_PAGE_MAX][256])
{
    uLong i;
    unz_global_info64 gi;
    int err;

    err = unzGetGlobalInfo64(uf,&gi);
    if (err!=UNZ_OK)
	{
	    char temp[100];
		sprintf(temp, "error %d with zipfile in unzGetGlobalInfo \n",err);
		logR(temp);
		return -1;
	}

	for (i=0;i<gi.number_entry;i++)
    {
        char filename[256];
        unz_file_info64 file_info;

        crashTemp(filename, sizeof(filename));

        err = unzGetCurrentFileInfo64(uf, &file_info, filename, sizeof(filename), NULL, 0, NULL, 0); //Get name -> 99% of what I need
        if (err!=UNZ_OK)
        {
            char temp[100];
            sprintf(temp, "error %d with zipfile in unzGetCurrentFileInfo",err);
            logR(temp);
            break;
        }

        ustrcpy(filename_inzip[i], filename);

        if ((file_info.flag & 1) != 0) //Si crypté
            *encrypted = 1;

        if ((i+1)<gi.number_entry)
        {
            err = unzGoToNextFile(uf);
            if (err!=UNZ_OK)
            {
                char temp[100];
                sprintf(temp, "error %d with zipfile in unzGetCurrentFileInfo",err);
                logR(temp);
                break;
            }
        }
    }
    return 0;
}

int miniunzip (char *inputZip, char *outputZip, char *passwordZip, size_t size, size_t type) //Type définit si l'extraction est standard ou pas
{
    int i = 0, j = 0;
    int opt_do_extract_withoutpath = 0; //Extraire en crashant le path
	int opt_overwrite = 1; /*Overwrite*/
	int opt_encrypted = 0, ret_value=0, mytoken = CURRENT_TOKEN++;
   	char *zipFileName = NULL, *zipOutput = NULL, *password = NULL;

	FILE* test = NULL;
    unzFile uf = NULL;
    unzFile uf_tests = NULL;
    zlib_filefunc_def fileops;

	char *path = NULL;

	int nombreFichiers = 0, nombreFichiersDecompresses = 0;
    char filename_inzip[NOMBRE_PAGE_MAX][256]; //Recevra la liste de tous les fichiers

    if(size) //Si extraction d'un chapitre
        opt_do_extract_withoutpath = 1;
    else
        zipFileName = malloc((strlen(inputZip)+1) *2); //Input
    zipOutput = malloc((strlen(outputZip)+1) *2); //Output
    password = malloc((strlen(passwordZip)+1) *2);

    if((!size && zipFileName == NULL) || zipOutput == NULL || (password == NULL && passwordZip != NULL))
    {
        logR("Failed at allocate memory into miniunzip\n");
        return 1;
    }

    for(i = j = 0; (!size && i < (strlen(inputZip)+1)*2) || j < (strlen(outputZip)+1)*2; i++, j++)
    {
        if(!size)
        {
            if(i < (strlen(inputZip)+1)*2 && inputZip[i] != '\0')
                zipFileName[i] = inputZip[i];
            else
            {
                if(zipFileName[i - 1] != 'p' && i + 5 < (strlen(inputZip)+1)*2) //Si il manque .zip
                {
                    zipFileName[i++] = '.';
                    zipFileName[i++] = 'z';
                    zipFileName[i++] = 'i';
                    zipFileName[i++] = 'p';
                    zipFileName[i++] = '\0';
                }
                else if (i < (strlen(inputZip)+1)*2)
                {
                    zipFileName[i] = 0;
                    i = (strlen(inputZip)+1)*2;
                }
            }
        }

        if(j < (strlen(outputZip)+1)*2 && outputZip[j])
            zipOutput[j] = outputZip[j];
        else if(j <(strlen(outputZip)+1) *2)
        {
            zipOutput[j] = 0;
            j = (strlen(outputZip)+1) *2;
        }
    }

    for(i = 0; i < (strlen(passwordZip)+1)*2 && passwordZip != NULL && passwordZip[i] != '\0'; i++)
        password[i] = passwordZip[i];
    password[i] = 0;

    if(!size)
    {
        path = malloc(strlen(zipFileName) + strlen(outputZip) + strlen(REPERTOIREEXECUTION) + 3*1);

        test = fopenR(zipFileName, "r");
        if (test == NULL)
        {
            logR("Cannot open");
            logR(zipFileName);
            logR("\n");
            goto quit;
        }

        fclose(test);

        uf = unzOpen(zipFileName);
        uf_tests = unzOpen(zipFileName); //Ouverture du zip

        if(uf == NULL || uf_tests == NULL)
        {
            sprintf(path, "%s/%s", REPERTOIREEXECUTION, zipFileName);
#ifdef _WIN32
            applyWindowsPathCrap(path);
#endif
            uf = unzOpen(path);
            uf_tests = unzOpen(path);
        }
    }
    else
    {
        path = malloc(strlen(outputZip) + strlen(REPERTOIREEXECUTION) + 3*1);        init_zmemfile(&fileops, inputZip, size);
        uf = unzOpen2(NULL, &fileops);
        uf_tests = unzOpen2(NULL, &fileops);
    }

    while(mytoken != INSTALL_DONE) //Tant qu'une décompression est en cours, on va pas changer le cwd
        SDL_Delay(50);

    UNZIP_NEW_PATH = 1; //Changer le répertoire par défaut change beaucoup (trop) de trucs

    #ifdef _WIN32
        sprintf(path, "%s\\%s", REPERTOIREEXECUTION, outputZip);
    #else
        sprintf(path, "%s/%s", REPERTOIREEXECUTION, outputZip);
    #endif

    sprintf(FONTUSED, "%s/%s", REPERTOIREEXECUTION, FONT_USED_BY_DEFAULT);
    applyWindowsPathCrap(FONTUSED);


    if (size && chdir(path)) //On change le dossier courant
    {
        createPath(outputZip); //En cas d'échec, on réessaie de créer le dossier
        if (chdir(path)) //Si réechoue
        {
            sprintf(password, "Error changing into %s, aborting\n", outputZip);
            logR(password);
            exit(-1);
        }
    }
    if(checkFileExist(CONFIGFILE))
        goto quit;

    for(i = 0; i < NOMBRE_PAGE_MAX; i++) //Réinitialise les noms de page
    {
        for(j = 0; j < 256; j++)
            filename_inzip[i][j] = 0;
    }

    ret_value = do_list(uf_tests, &opt_encrypted, filename_inzip); //On lit l'archive
    unzClose(uf_tests);

    for(nombreFichiers = 0; nombreFichiers < NOMBRE_PAGE_MAX && filename_inzip[nombreFichiers][0] != 0; nombreFichiers++); //Nombre de fichier dans l'archive ZIP

    unsigned char pass[NOMBRE_PAGE_MAX][SHA256_DIGEST_LENGTH]; //Les pass des fichiers

    for(i=0; i<NOMBRE_PAGE_MAX;i++)
        for(j=0;j<SHA256_DIGEST_LENGTH; pass[i][j++] = 0);

    if(size)
    {
        if(opt_encrypted == 0)
            opt_encrypted = FULL_ENCRYPTION;
        else //chapitre crypté
        {
            getPasswordArchive(zipFileName, password);
            if(!password[0])
            {
                ret_value = -1;
                goto quit;
            }
        }
    }

    for(i = 0; i < nombreFichiers; i++)
    {
        if(checkNameFileZip(filename_inzip[i]))
        {
            if(opt_encrypted && password[0] != 0) //Si mot de passe
                do_extract_onefile(uf, filename_inzip[i], opt_do_extract_withoutpath, opt_overwrite, password, pass[i]);
            else if(opt_encrypted)
                do_extract_onefile(uf, filename_inzip[i], opt_do_extract_withoutpath, opt_overwrite, NULL, pass[i]);
            else
                ret_value = do_extract_onefile(uf, filename_inzip[i], opt_do_extract_withoutpath, opt_overwrite, NULL, NULL);
            nombreFichiersDecompresses++;
        }
        else
            for(j=0; j<256; filename_inzip[i][j++]=0);

        if ((i+1) < nombreFichiers)
        {
            if (unzGoToNextFile(uf) != UNZ_OK)
                break;
        }
    }

    if(opt_encrypted)
    {
        /*On va écrire les clées dans un config.enc
          Pour ça, on va classer les clées avec la longueur des clées
          il est un peu 3h du mat donc je vais pas m'attarder*/
        int nombreFichierDansConfigFile = 0;
        char nomPage[NOMBRE_PAGE_MAX][LONGUEUR_NOM_PAGE];
        unsigned char temp[256], temp2[256];

		unsigned char *hugeBuffer = NULL;

        crashTemp(temp, 256);
        crashTemp(temp2, 256);

        nombreFichiers--;

        for(i=0; i < NOMBRE_PAGE_MAX; i++) //On vire les paths des noms de fichiers
        {
            int k = 0;

            if(!filename_inzip[i][0])
                continue;
            j=strlen(filename_inzip[i]);
            for(; j>0 && filename_inzip[i][j] != '/'; j--);
            if(!j)
                continue;
            for(j++; filename_inzip[i][j] != 0 && j < 256; filename_inzip[i][k++] = filename_inzip[i][j++]);
            for(; k < 256; filename_inzip[i][k++] = 0);
        }

        /*On va classer les fichier et les clées en ce basant sur config.dat*/

        if(configFileLoader(CONFIGFILE, &nombreFichierDansConfigFile, nomPage) || (nombreFichierDansConfigFile != nombreFichiersDecompresses-2 && nombreFichierDansConfigFile != nombreFichiersDecompresses-1)) //-2 car -1 + un décallage de -1 du Ã  l'optimisation pour le lecteur
        {
            logR("config.dat invalid: encryption aborted.\n");
            for(i = 0; filename_inzip[i][0]; remove(filename_inzip[i++])); //On fais le ménage
            ret_value = -1;
            goto quit;
        }

        for(i = 0; strcmp(filename_inzip[i], CONFIGFILE) && i < NOMBRE_PAGE_MAX; i++);
        if(!strcmp(filename_inzip[i], CONFIGFILE)) //On vire les clées du config.dat
        {
            for(j=0; filename_inzip[i][j] && j < 256; j++)
                filename_inzip[i][j] = 0;
            for(j = NOMBRE_PAGE_MAX-1; j > i && filename_inzip[j][0] == 0; j--);
            if(j > i)
            {
                ustrcpy(filename_inzip[i], filename_inzip[j]);
                usstrcpy(pass[i], SHA256_DIGEST_LENGTH, pass[j]);
                for(i = 0; i < 256; filename_inzip[j][i++] = 0);
                for(i = 0; i < SHA256_DIGEST_LENGTH; pass[j][i++] = 0);
            }

        }
        nombreFichiers--;

        for(i = j = 0; i < NOMBRE_PAGE_MAX; i++, j++) //on consolide la liste des noms (fichiers invalides dégagés)
        {
            MajToMin(filename_inzip[j]); //On en profite pour tout passer en minuscule
            if(filename_inzip[i][0] < ' ')//Fichier manquant
            {
                for(; filename_inzip[i][0] < ' ' && i < NOMBRE_PAGE_MAX; i++);
                if(i >= NOMBRE_PAGE_MAX)
                    break;
                ustrcpy(filename_inzip[j], filename_inzip[i]);
                crashTemp(filename_inzip[i], 256);
                ustrcpy(pass[j], pass[i]);
                crashTemp(pass[i], SHA256_DIGEST_LENGTH);
                i--;
            }
        }

        for(i = 0; i <= nombreFichiers && filename_inzip[i][0] != 0; i++)
        {
            MajToMin(nomPage[i]);
            for(j=0; strcmp(nomPage[i], filename_inzip[j]) && filename_inzip[j][0] && j < nombreFichiers; j++);

            if(j != i && !strcmp(nomPage[i], filename_inzip[j])) //Mauvais classement
            {
                ustrcpy(temp, filename_inzip[i]); //On déplace les noms
                ustrcpy(filename_inzip[i], filename_inzip[j]);
                ustrcpy(filename_inzip[j], temp);
                crashTemp(temp, 256);

                memcpy(temp, pass[i], SHA256_DIGEST_LENGTH); //On déplace les clées
                memcpy(pass[i], pass[j], SHA256_DIGEST_LENGTH);
                memcpy(pass[j], temp, SHA256_DIGEST_LENGTH);
                crashTemp(temp, 256);
            }
        }
        hugeBuffer = malloc(((SHA256_DIGEST_LENGTH+1)*NOMBRE_PAGE_MAX + 15 ) * sizeof(unsigned char)); //+1 pour \n, +15 pour le nombre en tête et le \n qui suis
        if(hugeBuffer== NULL)
        {
            logR("Failed at allocate memore to hugeBuffer\n");
            exit(-1);
        }
        sprintf((char *) hugeBuffer, "%d", nombreFichiers);
        j = ustrlen(hugeBuffer)-1;
        for(i=0; i <= nombreFichiers; i++) //Write config.enc
        {
            int k = 0;
            hugeBuffer[j++] = ' ';
            for(; k < SHA256_DIGEST_LENGTH; hugeBuffer[j++] = pass[i][k++]);
        }
        hugeBuffer[j] = 0;

        if(getMasterKey(temp)) //delete chapter
            for(i=0; filename_inzip[i][0]; remove(filename_inzip[i++]));
        else
        {
            uint8_t hash[SHA256_DIGEST_LENGTH], chapter[15];
            sprintf((char *)chapter, "%d", type);

            pbkdf2((uint8_t *) temp, chapter, hash);

            crashTemp(temp, 256);
            AESEncrypt(hash, hugeBuffer, "config.enc", INPUT_IN_MEMORY);
            crashTemp(hash, SHA256_DIGEST_LENGTH);
            crashTemp(hugeBuffer, (SHA256_DIGEST_LENGTH+1)*NOMBRE_PAGE_MAX + 10); //On écrase pour que ça soit plus chiant Ã  lire
        }
        free(hugeBuffer);
    }

quit:

    unzClose(uf);
    if(size)
        destroy_zmemfile(&fileops);
    chdirR();

    free(path);
    free(zipFileName);
    free(zipOutput);
    free(password);

    UNZIP_NEW_PATH = 0;
    INSTALL_DONE++;

    return ret_value;
}

