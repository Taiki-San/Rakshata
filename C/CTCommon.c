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


int autoSelectionChapitreTome(MANGAS_DATA *mangaDB, bool isTome, int contexte)
{
    if(contexte != CONTEXTE_DL) //Si une seul chapitre, on le séléctionne automatiquement
    {
        if(isTome)
        {
            if(mangaDB->tomes == NULL)
                getUpdatedCTList(mangaDB, isTome);

            if(mangaDB->tomes != NULL && mangaDB->tomes[0].ID == mangaDB->tomes[mangaDB->nombreTomes-1].ID && checkTomeReadable(*mangaDB, mangaDB->tomes[0].ID))
                return mangaDB->tomes[0].ID;
        }

        else
        {
            if(mangaDB->chapitres == NULL)
                getUpdatedCTList(mangaDB, isTome);

            if(mangaDB->chapitres != NULL && mangaDB->chapitres[0] == mangaDB->chapitres[mangaDB->nombreChapitre-1] && checkChapterReadable(*mangaDB, mangaDB->chapitres[0]))
                return mangaDB->chapitres[0];
        }
    }
    return VALEUR_FIN_STRUCTURE_CHAPITRE;
}

void displayTemplateChapitreTome(MANGAS_DATA* mangaDB, int contexte, int isTome, PREFS_ENGINE data, char texteTrad[SIZE_TRAD_ID_19][TRAD_LENGTH])
{
#ifdef IDENTIFY_MISSING_UI
	#warning "Add template rendering"
#endif
}

void displayIconeChapOrTome(bool isTome)
{
    char tempPath[450];
    if(!isTome)
        snprintf(tempPath, 450, "%s/%s", REPERTOIREEXECUTION, ICONE_SWITCH_CHAPITRE);
    else
        snprintf(tempPath, 450, "%s/%s", REPERTOIREEXECUTION, ICONE_SWITCH_TOME);
}

int askForCT(MANGAS_DATA* mangaDB, bool *isTome, int contexte)
{
    int outChoisis = 0, dernierLu, dernierLuTome = VALEUR_FIN_STRUCTURE_CHAPITRE, dernierLuChapitre = VALEUR_FIN_STRUCTURE_CHAPITRE, noChoice = 0;
    char temp[TAILLE_BUFFER], texteTrad[SIZE_TRAD_ID_19][TRAD_LENGTH];
    DATA_ENGINE *data = NULL;
	PREFS_ENGINE prefs;
    loadTrad(texteTrad, 19);

    snprintf(temp, TAILLE_BUFFER, "manga/%s/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName, CONFIGFILE);
    if(!checkFileExist(temp) && contexte != CONTEXTE_DL)
        return PALIER_MENU;

    if((dernierLuChapitre = autoSelectionChapitreTome(mangaDB, *isTome, contexte)) != VALEUR_FIN_STRUCTURE_CHAPITRE)
        return dernierLuChapitre;

    refreshTomeList(mangaDB);
    refreshChaptersList(mangaDB);
    if(contexte != CONTEXTE_DL)
    {
        checkTomeValable(mangaDB, &dernierLuTome);
        checkChapitreValable(mangaDB, &dernierLuChapitre);
        if(mangaDB->nombreTomes == 0 && mangaDB->nombreChapitre == 0)
            return PALIER_MENU;
    }

    do
    {
        if(contexte == CONTEXTE_DL && checkFileExist(temp))
            dernierLu = VALEUR_FIN_STRUCTURE_CHAPITRE; //Si un manga est déjà installé, on le met dans le sens décroissant)
        else
            dernierLu = 0;

        if(*isTome)
        {
            data = generateTomeList(mangaDB, (dernierLu == VALEUR_FIN_STRUCTURE_CHAPITRE), contexte, texteTrad[14], texteTrad[1], &prefs);
            if(dernierLu != VALEUR_FIN_STRUCTURE_CHAPITRE)
                dernierLu = dernierLuTome;
        }
        else
        {
            data = generateChapterList(mangaDB, (dernierLu == VALEUR_FIN_STRUCTURE_CHAPITRE), contexte, texteTrad[14], texteTrad[0], &prefs);
            if(dernierLu != VALEUR_FIN_STRUCTURE_CHAPITRE)
                dernierLu = dernierLuChapitre;
        }

        if(data == NULL) //Erreur de mémoire ou liste vide
        {
            if(noChoice)
                return errorEmptyCTList(contexte, texteTrad);
            else
            {
                noChoice++;
                *isTome = !*isTome; //On inverse le tome
                outChoisis = ENGINE_RETVALUE_SWITCH;
                continue;
            }
        }
        else
            prefs.IDDernierElemLu = dernierLu;

        displayTemplateChapitreTome(mangaDB, contexte, *isTome, prefs, texteTrad);
        if(!noChoice)
        {
            displayIconeChapOrTome(*isTome);
            prefs.switchAvailable = true;
        }

        if(outChoisis == ENGINE_RETVALUE_SWITCH)
        {
            if(!noChoice)
                *isTome = !*isTome;
        }

        free(data);
    }while (outChoisis == ENGINE_RETVALUE_SWITCH);
    return outChoisis;
}

void getUpdatedCTList(MANGAS_DATA *mangaDB, bool isTome)
{
    if(isTome)
        getUpdatedTomeList(mangaDB);
    else
        getUpdatedChapterList(mangaDB);
}

bool isAlreadyInstalled(MANGAS_DATA projectData, bool isCallerCtxTome, int IDChap)
{
	if(IDChap == -1)
		return false;

	if((!isCallerCtxTome && projectData.tomes == NULL) || (isCallerCtxTome && projectData.chapitres == NULL))
		return false;

	char pathToCheck[LONGUEUR_NOM_MANGA_MAX * 2 + 256];
	if(isCallerCtxTome)
	{
		if(IDChap % 10)
			snprintf(pathToCheck, sizeof(pathToCheck), "manga/%s/%s/Chapitre_%d.%d/%s", projectData.team->teamLong, projectData.mangaName, IDChap / 10, IDChap % 10, CONFIGFILE);
		else
			snprintf(pathToCheck, sizeof(pathToCheck), "manga/%s/%s/Chapitre_%d/%s", projectData.team->teamLong, projectData.mangaName, IDChap / 10, CONFIGFILE);
	}
	else
	{
		uint pos, pos2;
		pathToCheck[0] = 0;
		CONTENT_TOME * buf;
		for(pos = 0; pos < projectData.nombreTomes; pos++)
		{
			buf = projectData.tomes[pos].details;
			for(pos2 = 0; buf[pos2].ID != VALEUR_FIN_STRUCTURE_CHAPITRE; pos2++)
			{
				if(buf[pos2].ID == IDChap && buf[pos2].isNative)
				{
					if(IDChap % 10)
						snprintf(pathToCheck, sizeof(pathToCheck), "manga/%s/%s/Tome_%d/Chapitre_%d.%d/%s", projectData.team->teamLong, projectData.mangaName, projectData.tomes[pos].ID, IDChap / 10, IDChap % 10, CONFIGFILE);
					else
						snprintf(pathToCheck, sizeof(pathToCheck), "manga/%s/%s/Tome_%d/Chapitre_%d/%s", projectData.team->teamLong, projectData.mangaName, projectData.tomes[pos].ID, IDChap / 10, CONFIGFILE);
					break;
				}
			}
		}
		if(!pathToCheck[0])
			return false;
	}
	
	return checkFileExist(pathToCheck);
}

bool checkReadable(MANGAS_DATA mangaDB, bool isTome, void *data)
{
    if(isTome)
    {
        META_TOME *tome = data;
        return checkTomeReadable(mangaDB, tome->ID);
    }
    return checkChapterReadable(mangaDB, *(int *) data);
}


bool isAnythingToDownload(MANGAS_DATA mangaDB)
{
	bool ret_value = false;
    uint prevSize;
	
    if(mangaDB.firstChapter != VALEUR_FIN_STRUCTURE_CHAPITRE)
    {
		mangaDB.chapitres = NULL;
		
		refreshChaptersList(&mangaDB);
        prevSize = mangaDB.nombreChapitre;
        checkChapitreValable(&mangaDB, NULL);
		ret_value = prevSize != mangaDB.nombreChapitre;
		
		free(mangaDB.chapitres);
    }
	
    if(mangaDB.firstTome != VALEUR_FIN_STRUCTURE_CHAPITRE)
    {
		mangaDB.tomes = NULL;
		
		refreshTomeList(&mangaDB);
        prevSize = mangaDB.nombreTomes;
        checkTomeValable(&mangaDB, NULL);
        ret_value |= prevSize != mangaDB.nombreTomes;

		free(mangaDB.tomes);
	}
    return ret_value;
}

