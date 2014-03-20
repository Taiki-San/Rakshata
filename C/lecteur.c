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

#include "lecteur.h"

int lecteur(MANGAS_DATA *mangaDB, int *chapitreChoisis, bool isTome, bool *fullscreen)
{
    int i, changementPage = READER_ETAT_DEFAULT;
	int curPosIntoStruct = 0;
    bool pageCharge = false, changementEtat = false, setTopInfosToWarning, redrawScreen, noRefresh;
    char texteTrad[SIZE_TRAD_ID_21][TRAD_LENGTH], infos[300];
    DATA_LECTURE dataReader;
    loadTrad(texteTrad, 21);

	curPosIntoStruct = reader_getPosIntoContentIndex(*mangaDB, *chapitreChoisis, isTome);	//Check the chapter can be read
	if(curPosIntoStruct == -1)
	{
		return PALIER_CHAPTER;
	}

	setLastChapitreLu(*mangaDB, isTome, *chapitreChoisis);
	if(reader_isLastElem(*mangaDB, isTome, *chapitreChoisis))
        startCheckNewElementInRepo(*mangaDB, isTome, *chapitreChoisis, fullscreen);

	//On met la page courante par défaut
	dataReader.pageCourante = reader_getCurrentPageIfRestore(texteTrad);
	
    if(configFileLoader(*mangaDB, isTome, *chapitreChoisis, &dataReader))
    {
        i = showError();
		return i > PALIER_MENU ? PALIER_CHAPTER : i;
    }
	
	//Main init

    while(1)
    {
		if(!changementEtat)	//Switch to fullscreen
        {
			if(changementPage == READER_ETAT_NEXTPAGE)// && nextPage != NULL)
			{
				//load next page
			}
			
			else if(changementPage == READER_ETAT_PREVPAGE)// && prevPage != NULL)
			{
				//load prev page
			}
			
			else if(changementPage == READER_ETAT_DEFAULT) //Premier chargement
			{
				//load first page + previous for caching
				changementPage = READER_ETAT_NEXTPAGE; //Mettra en cache la page n+1
			}
			
			if(0)//page == NULL)		//Si erreur
			{
				internalDeleteCT(*mangaDB, isTome, *chapitreChoisis);
				i = showError();
				return i > PALIER_MENU ? PALIER_CHAPTER : i;
			}
			else
				pageCharge = false;
		}
		
		generateMessageInfoLecteurChar(*mangaDB, dataReader, texteTrad, isTome, *fullscreen, curPosIntoStruct, infos, sizeof(infos));

        /*Phase finale de l'initialisation de la page*/
		
        redrawScreen = noRefresh = setTopInfosToWarning = false;

        while(!redrawScreen)
        {
            if(!noRefresh)
			{
				//REFRESH_SCREEN();
			}

            else
                noRefresh = false;

            if(!pageCharge) //Bufferisation
            {
				if(changementPage == READER_ETAT_NEXTPAGE)
                {
                    //nextPage = reader_bufferisePages(dataReader, true);
                }
                else if (changementPage == READER_ETAT_PREVPAGE)
                {
                    //prevPage = reader_bufferisePages(dataReader, false);
                }
#ifdef _WIN32
				REFRESH_SCREEN();	//If the password was asked, the perf impact is too important on OSX
#endif
                pageCharge = true;
                changementPage = READER_ETAT_DEFAULT;
            }

        }
    }
    return PALIER_QUIT;	//Shouldn't be reached
}

