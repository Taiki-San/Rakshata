/*********************************************************************************************
**	__________         __           .__            __                 ________   _______   	**
**	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
**	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
**	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
**	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
**	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
**                                                                                          **
**    Licence propriŽtaire, code source confidentiel, distribution formellement interdite   **
**                                                                                          **
*********************************************************************************************/


bool addRepoByFileInProgress;
void mainRakshata()
{
    int continuer = PALIER_DEFAULT, restoringState = 0, sectionChoisis;
	
    if(checkEvnt() == PALIER_QUIT) //Check envt
    {
        continuer = PALIER_QUIT;
    }

    else if(addRepoByFileInProgress)
    {
        if(ajoutRepo(true) > 0)
            raffraichissmenent(true);
        continuer = PALIER_QUIT;
    }

    while(continuer > PALIER_QUIT)
    {
		sectionChoisis = 1;
		restoringState = 0;

        switch(sectionChoisis)
        {
            case 1:
                continuer = mainLecture();
                break;

            case 2:
                continuer = mainChoixDL();
                break;

            default:
                continuer = sectionChoisis;
                break;
        }
    }

    quit_thread(0);
}

extern int curPage; //Too lazy to use an argument
int mainLecture()
{
    int continuer = PALIER_DEFAULT, mangaChoisis, chapitreChoisis, retourLecteur, pageManga = 1, pageChapitre = 1;
    bool retry, fullscreen, restoringState = 0;

    if(checkRestore())
        restoringState = 1;

    while(continuer > PALIER_MENU)
    {
        mangaChoisis = chapitreChoisis = 0;

        MANGAS_DATA *mangaDB = miseEnCache(LOAD_DATABASE_INSTALLED);

        /*Appel des selectionneurs*/
        if(!restoringState)
        {
            curPage = pageManga;
            mangaChoisis = controleurManga(mangaDB, CONTEXTE_LECTURE, 0, NULL);
            pageManga = curPage;
        }
        if(mangaChoisis <= PALIER_CHAPTER)
        {
            if(mangaChoisis == PALIER_CHAPTER)
                continuer = PALIER_MENU;

            else
                continuer = mangaChoisis;
        }

        if(mangaChoisis > -1 || restoringState == 1)
        {
            if(!restoringState)
                retourLecteur = 1;//checkProjet(mangaDB[mangaChoisis]);

            else
                retourLecteur = 1;

            if(retourLecteur < -1)
            {
                continuer = retourLecteur;
            }

            else if(retourLecteur == 1)
            {
                bool isTome = false;
                chapitreChoisis = PALIER_DEFAULT;
                pageChapitre = 1;
                do
                {
                    retry = false; //Si on doit relancer la boucle
					fullscreen = false;
					
                    if(!restoringState)
                    {
                        curPage = pageChapitre;
                        chapitreChoisis = controleurChapTome(&mangaDB[mangaChoisis], &isTome, CONTEXTE_LECTURE);
                        pageChapitre = curPage;
                    }

                    if (chapitreChoisis <= PALIER_CHAPTER)
                        continuer = chapitreChoisis;
                    else
                    {
                        /*Lancement Lecteur*/
                        retourLecteur = 0;
                        while(!retourLecteur)
                        {
                            if(restoringState == 1)
                            {
                                char temp[LONGUEUR_NOM_MANGA_MAX], type[2] = {0, 0};
                                FILE* test = NULL;

                                test = fopen("data/laststate.dat", "r");
                                fscanfs(test, "%s %s %d", temp, LONGUEUR_NOM_MANGA_MAX, type, 2, &chapitreChoisis);
                                fclose(test);

                                for(mangaChoisis = 0; strcmp(temp, mangaDB[mangaChoisis].mangaName) != 0; mangaChoisis++);
                                if(type[0] == 'T')
                                {
                                    isTome = true;
                                    getUpdatedTomeList(&mangaDB[mangaChoisis]);
                                }
                                else
                                {
                                    isTome = false;
                                    getUpdatedChapterList(&mangaDB[mangaChoisis]);
                                }
                                restoringState = 0;
                            }

                            setLastChapitreLu(&mangaDB[mangaChoisis], isTome, chapitreChoisis); //On Žcrit le dernier chapitre lu
                            retourLecteur = lecteur(&mangaDB[mangaChoisis], &chapitreChoisis, isTome, &fullscreen);

                            if(retourLecteur != 0)
                            {
                                if(fullscreen != 0)
                                {
#ifdef IDENTIFY_MISSING_UI
									#warning "Leave fullscreen"
#endif
                                }
                            }
                        }
                        pageChapitre = 1;
                        if(retourLecteur < PALIER_CHAPTER)
                            continuer = retourLecteur;
                        else if(chapitreChoisis != VALEUR_FIN_STRUCTURE_CHAPITRE && chapitreChoisis != autoSelectionChapitreTome(&mangaDB[mangaChoisis], isTome, CONTEXTE_LECTURE))
                        {
                            getUpdatedCTList(&mangaDB[mangaChoisis], isTome);
                            retry = true;
                        }
                    }
                }while(retry);
            }
        }
        freeMangaDataLegacy(mangaDB, NOMBRE_MANGA_MAX);
    }
    return continuer;
}

