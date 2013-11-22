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

#include "main.h"
#include "lecteur.h"

int configFileLoader(MANGAS_DATA *mangaDB, bool isTome, int chapitre_tome, DATA_LECTURE* dataReader)
{
    int i, prevPos = 0, nombrePages = 0, posID = 0, nombreTours = 1, lengthBasePath, lengthFullPath;
    char name[LONGUEUR_NOM_PAGE];
    FILE* config = NULL;
	
    void * intermediaryPtr;
    bool allocError;
	
    dataReader->nombrePageTotale = 1;
	
    dataReader->nomPages = dataReader->path = NULL;
    dataReader->pathNumber = dataReader->pageCouranteDuChapitre = dataReader->chapitreTomeCPT = NULL;
	
    if(isTome)
    {
        snprintf(name, LONGUEUR_NOM_PAGE, "manga/%s/%s/Tome_%d/%s", mangaDB->team->teamLong, mangaDB->mangaName, chapitre_tome, CONFIGFILETOME);
        config = fopen(name, "r");
        if(config == NULL)
            return 1;
        name[0] = 0;
        fscanfs(config, "%s", name, LONGUEUR_NOM_PAGE);
    }
    else
    {
        if(chapitre_tome%10)
            snprintf(name, LONGUEUR_NOM_PAGE, "Chapitre_%d.%d", chapitre_tome/10, chapitre_tome%10);
        else
            snprintf(name, LONGUEUR_NOM_PAGE, "Chapitre_%d", chapitre_tome/10);
    }
	
    do
    {
        char input_path[LONGUEUR_NOM_PAGE], **nomPagesTmp = NULL;
		
        snprintf(input_path, LONGUEUR_NOM_PAGE, "manga/%s/%s/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName, name, CONFIGFILE);
        allocError = false;
		
        nomPagesTmp = loadChapterConfigDat(input_path, &nombrePages);
        if(nomPagesTmp != NULL)
        {
            /*On réalloue la mémoire en utilisant un buffer intermédiaire*/
            dataReader->nombrePageTotale += nombrePages;
			
            ///pathNumber
            intermediaryPtr = realloc(dataReader->pathNumber, (dataReader->nombrePageTotale+1) * sizeof(int));
            if(intermediaryPtr != NULL)
                dataReader->pathNumber = intermediaryPtr;
            else
                allocError = true;
			
            ///pageCouranteDuChapitre
            intermediaryPtr = realloc(dataReader->pageCouranteDuChapitre, (dataReader->nombrePageTotale+1) * sizeof(int));
            if(intermediaryPtr != NULL)
                dataReader->pageCouranteDuChapitre = intermediaryPtr;
            else
                allocError = true;
			
            ///nomPages
            intermediaryPtr = realloc(dataReader->nomPages, (dataReader->nombrePageTotale+1) * sizeof(char*));
            if(intermediaryPtr != NULL)
                dataReader->nomPages = intermediaryPtr;
            else
                allocError = true;
			
            ///chapitreTomeCPT
            intermediaryPtr = realloc(dataReader->chapitreTomeCPT, (++nombreTours) * sizeof(int));
            if(intermediaryPtr != NULL)
                dataReader->chapitreTomeCPT = intermediaryPtr;
            else
                allocError = true;
			
            ///path
            intermediaryPtr = realloc(dataReader->path, nombreTours * sizeof(char*));
            if(intermediaryPtr != NULL)
            {
                dataReader->path = intermediaryPtr;
                dataReader->path[nombreTours-2] = malloc(LONGUEUR_NOM_PAGE);
                
                if(dataReader->path[nombreTours-2] == NULL)
                    allocError = true;
            }
            else
                allocError = true;
			
            if(allocError)  //Si on a eu un problème en allouant de la mémoire
            {
				if(dataReader->pathNumber == NULL || dataReader->pageCouranteDuChapitre == NULL || dataReader->nomPages == NULL || dataReader->chapitreTomeCPT == NULL || dataReader->path == NULL || dataReader->path[nombreTours-2] == NULL)
				{
					free(dataReader->pathNumber);				dataReader->pathNumber = NULL;
					free(dataReader->pageCouranteDuChapitre);	dataReader->pageCouranteDuChapitre = NULL;
					free(dataReader->nomPages);					dataReader->nomPages = NULL;
					free(dataReader->chapitreTomeCPT);			dataReader->chapitreTomeCPT = NULL;
					if(dataReader->path != NULL)
						free(dataReader->path[nombreTours-2]);
					
					free(dataReader->path);						dataReader->path = NULL;
				}
                dataReader->nombrePageTotale -= nombrePages;
                nombreTours--;
            }
            else
            {
                dataReader->path[nombreTours-1] = NULL;
				
                snprintf(dataReader->path[posID], LONGUEUR_NOM_PAGE, "manga/%s/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName, name);
                if(isTome)
                    dataReader->chapitreTomeCPT[posID] = extractNumFromConfigTome(name, chapitre_tome);
                else
                    dataReader->chapitreTomeCPT[posID] = chapitre_tome;
				
                lengthBasePath = strlen(dataReader->path[posID]);
				
                for(i = 0; prevPos < dataReader->nombrePageTotale; prevPos++) //Réinintialisation
                {
                    lengthFullPath = lengthBasePath + strlen(nomPagesTmp[i]) + 0x10; // '/' + \0 + margin
                    dataReader->nomPages[prevPos] = malloc(lengthFullPath);
                    if(dataReader->nomPages[prevPos] != NULL)
                    {
                        snprintf(dataReader->nomPages[prevPos], lengthFullPath, "%s/%s", dataReader->path[posID], nomPagesTmp[i]);
                        dataReader->pathNumber[prevPos] = posID;
                        dataReader->pageCouranteDuChapitre[prevPos] = i++;
                    }
                    else    //Si problème d'allocation
                    {
                        prevPos--;
                    }
                }
                posID++;
                for(i = 0; nomPagesTmp[i] != NULL; free(nomPagesTmp[i++]));
                free(nomPagesTmp);
            }
        }
		
        if(isTome)
        {
            if(fgetc(config) == EOF)
            {
                fclose(config);
                break;
            }
            fseek(config, -1, SEEK_CUR);
            fscanfs(config, "%s", name, LONGUEUR_NOM_PAGE);
        }
    } while(isTome && posID < LONGUEUR_NOM_PAGE);
	
    if(dataReader->pathNumber != NULL && dataReader->nomPages != NULL)
    {
        dataReader->IDDisplayed = chapitre_tome;
        dataReader->pathNumber[prevPos] = VALEUR_FIN_STRUCTURE_CHAPITRE;
        dataReader->nomPages[dataReader->nombrePageTotale] = NULL; //On signale la fin de la structure
        dataReader->nombrePageTotale--; //Décallage pour l'utilisation dans le lecteur
    }
    if(dataReader->pageCourante > dataReader->nombrePageTotale)
        dataReader->pageCourante = dataReader->nombrePageTotale;
    return 0;
}

char ** loadChapterConfigDat(char* input, int *nombrePage)
{
    char ** output;
    int i, j, scriptUtilise = 0;
    FILE* file_input = fopenR(input, "r");
	if(file_input == NULL)
        return NULL;
	
    fscanfs(file_input, "%d", nombrePage);
	
    if(fgetc(file_input) != EOF)
    {
        fseek(file_input, -1, SEEK_CUR);
        if(fgetc(file_input) == 'N')
            scriptUtilise = 1;
        else
            fseek(file_input, -1, SEEK_CUR);
		
        output = calloc(5+*nombrePage, sizeof(char*));
		
        for(i = 0; i < *nombrePage; i++)
        {
            output[i] = malloc(LONGUEUR_NOM_PAGE+1);
            if(!scriptUtilise)
                fscanfs(file_input, "%d %s\n", &j, output[i], LONGUEUR_NOM_PAGE);
			
            else
                fscanfs(file_input, "%d %s", &j, output[i], LONGUEUR_NOM_PAGE);
            changeTo(output[i], '&', ' ');
        }
        output[i] = malloc(LONGUEUR_NOM_PAGE);
        output[i][0] = 0;
    }
	
    else
    {
        (*nombrePage)++;
        output = calloc(5+*nombrePage, sizeof(char*));
        for(i = 0; i < *nombrePage; i++)
        {
            output[i] = malloc(LONGUEUR_NOM_PAGE+1);
            snprintf(output[i], LONGUEUR_NOM_PAGE, "%d.jpg", i);
        }
        output[i] = malloc(LONGUEUR_NOM_PAGE);
        output[i][0] = 0;
    }
    fclose(file_input);
    for(i = strlen(input); i > 0 && input[i] != '/'; input[i--] = 0);
	
    char temp[300];
    for(i = *nombrePage; i >= 0; i--)
    {
        if(output[i][0])
        {
            snprintf(temp, 300, "%s%s", input, output[i]);
            if(checkFileExist(temp))
            {
                *nombrePage = i;
                break;
            }
        }
    }
    return output;
}

void slideOneStepDown(SDL_Surface *chapitre, SDL_Rect *positionSlide, SDL_Rect *positionPage, int ctrlPressed, int pageTropGrande, int move, int *noRefresh)
{
    if(!ctrlPressed)
    {
        if(positionSlide->y > move)
        {
            positionSlide->y -= move;
        }
        else
        {
            positionSlide->y = 0;
        }
		
        if(chapitre->h - positionSlide->y > positionSlide->h && positionPage->h != chapitre->h - positionSlide->y && chapitre->h - positionSlide->y <= getPtRetinaH(renderer))
        {
            positionPage->h = positionSlide->h = chapitre->h - positionSlide->y;
        }
        else
        {
            positionPage->h = positionSlide->h = (chapitre->h < getPtRetinaH(renderer)) ? chapitre->h : getPtRetinaH(renderer);
        }
    }
	
    else if(pageTropGrande)
    {
        if(positionSlide->x >= move)
        {
            positionPage->x = 0;
            positionSlide->x -= move;
            if(chapitre->w - positionSlide->x - positionPage->x < getPtRetinaW(renderer))
                positionPage->w = positionSlide->w = chapitre->w - positionSlide->x - positionPage->x;
            else
                positionPage->w = positionSlide->w = getPtRetinaW(renderer);
        }
        else if (positionSlide->x != 0)
        {
            positionPage->x = BORDURE_LAT_LECTURE < positionSlide->x - move ? positionSlide->x - move : BORDURE_LAT_LECTURE;
            positionSlide->x = 0;
            positionPage->w = positionSlide->w = getPtRetinaW(renderer) - positionPage->x;
        }
        else
        {
            if(positionPage->x == BORDURE_LAT_LECTURE)
                *noRefresh = 1;
            else
            {
                positionSlide->x = 0;
                if(positionPage->x + move > BORDURE_LAT_LECTURE)
                    positionPage->x = BORDURE_LAT_LECTURE;
                else
                    positionPage->x += move;
                positionPage->w = positionSlide->w = getPtRetinaW(renderer) - positionPage->x;
            }
        }
    }
}

void slideOneStepUp(SDL_Surface *chapitre, SDL_Rect *positionSlide, SDL_Rect *positionPage, int ctrlPressed, int pageTropGrande, int move, int *noRefresh)
{
    if(!ctrlPressed)
    {
        if(positionSlide->y < chapitre->h - (getPtRetinaH(renderer) - BORDURE_CONTROLE_LECTEUR - BORDURE_HOR_LECTURE) - move)
        {
            positionSlide->y += move;
        }
        else if(chapitre->h > getPtRetinaH(renderer) - BORDURE_CONTROLE_LECTEUR - BORDURE_HOR_LECTURE)
        {
            positionSlide->y = chapitre->h - (getPtRetinaH(renderer) - BORDURE_CONTROLE_LECTEUR - BORDURE_HOR_LECTURE);
        }
		
        if(chapitre->h - positionSlide->y < positionSlide->h && positionPage->h != chapitre->h - positionSlide->y)
        {
            positionPage->h = positionSlide->h = chapitre->h - positionSlide->y;
        }
        else if (positionPage->h == chapitre->h - positionSlide->y)
            *noRefresh = 1;
    }
    else if(pageTropGrande)
    {
        if(positionPage->x != 0)
        {
            positionPage->x -= move;
            if(positionPage->x <= 0)
                positionSlide->x = positionPage->x = 0;
            positionPage->w = positionSlide->w = getPtRetinaW(renderer) - positionPage->x;
        }
		
        else if(positionSlide->x < chapitre->w - getPtRetinaW(renderer) - move)
        {
            positionSlide->x += move;
            positionPage->w = positionSlide->w = getPtRetinaW(renderer);
        }
        else
        {
            if(positionSlide->w != getPtRetinaW(renderer) - BORDURE_LAT_LECTURE)
            {
                positionSlide->x += move;
                if(positionSlide->x > chapitre->w - getPtRetinaW(renderer) + BORDURE_LAT_LECTURE)
                    positionSlide->x = chapitre->w - getPtRetinaW(renderer) + BORDURE_LAT_LECTURE;
                positionPage->w = positionSlide->w = chapitre->w - positionSlide->x;
            }
            else if(positionPage->x == 0)
                *noRefresh = 1;
        }
    }
}

int changementDePage(MANGAS_DATA *mangaDB, DATA_LECTURE* dataReader, bool isTome, bool goToNextPage, int *changementPage, int *finDuChapitre, int *chapitreChoisis, int currentPosIntoStructure)
{
    int ret_value = 0;
	
    if(goToNextPage) //Page suivante
    {
        if (dataReader->pageCourante < dataReader->nombrePageTotale) //Changement de page
        {
            dataReader->pageCourante += 1;
            *changementPage = 1;
            *finDuChapitre = 0;
        }
        else if(changementDeChapitre(mangaDB, isTome, currentPosIntoStructure+1, chapitreChoisis)) //On envois l'ordre de quitter
        {
            ret_value = -1;
        }
        else if(!*finDuChapitre) //On met le menu en rouge
        {
            *finDuChapitre = 1;
        }
        else
            ret_value = 1; //Ne raffraichis pas la page
    }
    else
    {
        if (dataReader->pageCourante > 0)
        {
            dataReader->pageCourante -= 1;
            *changementPage = -1;
            *finDuChapitre = 0;
        }
        else if(changementDeChapitre(mangaDB, isTome, currentPosIntoStructure-1, chapitreChoisis)) //On envois l'ordre de quitter
        {
            ret_value = -1;
        }
        else if(!*finDuChapitre) //On met le menu en rouge
        {
            *finDuChapitre = 1;
        }
        else
            ret_value = 1;
    }
	SDL_FlushEvent(SDL_KEYDOWN);
    return ret_value;
}

int changementDeChapitre(MANGAS_DATA* mangaDB, bool isTome, int posIntoStructToTest, int *chapitreChoisis)
{
    if(posIntoStructToTest < 0)
        return 0;
	
    getUpdatedCTList(mangaDB, isTome);
    if((isTome && mangaDB->tomes[posIntoStructToTest].ID != VALEUR_FIN_STRUCTURE_CHAPITRE) || (!isTome && mangaDB->chapitres[posIntoStructToTest] != VALEUR_FIN_STRUCTURE_CHAPITRE))
    {
        if(isTome)
            *chapitreChoisis = mangaDB->tomes[posIntoStructToTest].ID;
        else
            *chapitreChoisis = mangaDB->chapitres[posIntoStructToTest];
        return 1;
    }
    return 0;
}

