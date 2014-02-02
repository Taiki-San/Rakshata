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

void startCheckNewElementInRepo(MANGAS_DATA mangaDB, bool isTome, int CT, bool * fullscreen)
{
    MUTEX_LOCK(mutex);
    if(NETWORK_ACCESS == CONNEXION_DOWN || NETWORK_ACCESS == CONNEXION_TEST_IN_PROGRESS || checkDLInProgress())
    {
        MUTEX_UNLOCK(mutex);
        return;
    }
    MUTEX_UNLOCK(mutex);
	
	
    DATA_CK_LECTEUR * argument = malloc(sizeof(DATA_CK_LECTEUR));
    if(argument != NULL)
    {
        argument->mangaDB = mangaDB;
        argument->isTome = isTome;
        argument->CT = CT;
        argument->fullscreen = fullscreen;
		
        createNewThread(checkNewElementInRepo, argument);
    }
}

void checkNewElementInRepo(DATA_CK_LECTEUR *input)
{
    bool isTome = input->isTome, newStuffs = false, *fullscreen;
    int i = 0, j = 0, version, CT;
    char temp[LONGUEUR_NOM_MANGA_MAX], *bufferDL, teamCourt[LONGUEUR_COURT];
    MANGAS_DATA mangaDB = input->mangaDB;
    CT = input->CT;
    fullscreen = input->fullscreen;
    mangaDB.chapitres = NULL;
    mangaDB.tomes = NULL;
	
    free(input);
	
    bufferDL = calloc(1, SIZE_BUFFER_UPDATE_DATABASE);
    if(bufferDL == NULL)
        quit_thread(0);
	
    version = get_update_mangas(bufferDL, mangaDB.team);
	
    if(!bufferDL[i]) //On a DL quelque chose
    {
        free(bufferDL);
        quit_thread(0);
    }
	
    i += sscanfs(&bufferDL[i], "%s %s", temp, LONGUEUR_NOM_MANGA_MAX, teamCourt, LONGUEUR_COURT);
    if(version == 2)
        while(bufferDL[i++] != '\n');
	
    if(strcmp(temp, mangaDB.team->teamLong) || strcmp(teamCourt, mangaDB.team->teamCourt)) //Fichier ne correspond pas
    {
        free(bufferDL);
        quit_thread(0);
    }
	
    if(!isTome)
    {
        int firstChapter, lastChapter;
        while(bufferDL[i] && bufferDL[i] != '#' && strcmp(mangaDB.mangaName, temp))
            i += sscanfs(&bufferDL[i], "%s %s %d %d\n", temp, LONGUEUR_NOM_MANGA_MAX, teamCourt, LONGUEUR_COURT, &firstChapter, &lastChapter);
        if(!strcmp(mangaDB.mangaName, temp))
        {
            mangaDB.firstChapter = firstChapter;
            mangaDB.lastChapter = lastChapter;
        }
    }
	
    if(isTome || mangaDB.nombreChapitreSpeciaux)
    {
        if(!i) i = 1;
        for(; bufferDL[i] && (bufferDL[i] != '#' || bufferDL[i - 1] != '\n'); i++); //On cherche la fin du bloc
        if(bufferDL[i] == '#' && bufferDL[i - 1] == '\n')
        {
            char type[2];
            for(i++; bufferDL[i] == '\n' || bufferDL[i] == '\r'; i++);
            while(bufferDL[i])
            {
                j = sscanfs(&bufferDL[i], "%s %s\n", temp, LONGUEUR_NOM_MANGA_MAX, type, 2);
                if(strcmp(mangaDB.mangaName, temp) || type[0] != (isTome?'T':'C'))
                {
                    for(; bufferDL[i] && (bufferDL[i] != '#' || bufferDL[i - 1] != '\n'); i++); //On saute le bloc
                    for(i++; bufferDL[i] == '\n' || bufferDL[i] == '\r'; i++);
                }
                else
                {
                    for(i += j; bufferDL[i] == '\n' || bufferDL[i] == '\r'; i++); //On se positionne à la fin
                    for(j = 0; bufferDL[i+j] && (bufferDL[i+j] != '#' || bufferDL[i+j-1] != '\n'); j++);
                    if(j)
                    {
                        char path[500];
                        snprintf(path, 500, "manga/%s/%s/%s", mangaDB.team->teamLong, mangaDB.mangaName, CONFIGFILETOME);
                        FILE *database = fopen(path, "w+");
                        if(database != NULL)
                        {
                            fwrite(&bufferDL[i], 1, j, database);
                            fclose(database);
                        }
                    }
					
                    if(isTome)
                    {
                        refreshTomeList(&mangaDB);
                        if(mangaDB.tomes == NULL || mangaDB.tomes[mangaDB.nombreTomes-1].ID <= CT)
                        {
                            free(bufferDL);
                            free(mangaDB.tomes);
                            quit_thread(0);
                        }
                        else
                            newStuffs = true;
                    }
                    else
                    {
                        refreshChaptersList(&mangaDB);
                        if(mangaDB.chapitres == NULL || mangaDB.chapitres[mangaDB.nombreChapitre-1] <= CT)
                        {
                            free(bufferDL);
                            free(mangaDB.chapitres);
                            quit_thread(0);
                        }
                        else
                            newStuffs = true;
                    }
                }
            }
        }
        else
            mangaDB.nombreChapitreSpeciaux = 0;
    }
	
    if(!isTome && !mangaDB.nombreChapitreSpeciaux)
    {
        free(bufferDL);
        refreshChaptersList(&mangaDB);
        if(mangaDB.chapitres == NULL || mangaDB.chapitres[mangaDB.nombreChapitre-1] <= CT)
        {
            free(mangaDB.chapitres);
            quit_thread(0);
        }
        newStuffs = true;
    }
	
    if(!newStuffs || fullscreen == NULL || *fullscreen == 1)
        quit_thread(0);
	
    bool severalNewElems = false;
    int ret_value, firstNewElem, nombreElement;
    char localization[SIZE_TRAD_ID_30][TRAD_LENGTH], stringDisplayed[6*TRAD_LENGTH], title[3*TRAD_LENGTH];
    SDL_MessageBoxData alerte;
    SDL_MessageBoxButtonData bouton[2];
    loadTrad(localization, 30);
	
    if(isTome)
    {
        for(firstNewElem = mangaDB.nombreTomes-1; firstNewElem > 0 && mangaDB.tomes[firstNewElem].ID > CT; firstNewElem--);
        nombreElement = mangaDB.nombreTomes-1 - firstNewElem;
    }
    else
    {
        for(firstNewElem = mangaDB.nombreChapitre-1; firstNewElem > 0 && mangaDB.chapitres[firstNewElem] > CT; firstNewElem--);
        nombreElement = mangaDB.nombreChapitre-1 - firstNewElem;
    }
	
    severalNewElems = nombreElement > 1;
    snprintf(title, 3*TRAD_LENGTH, "%s %s %s", localization[1+severalNewElems], localization[5+isTome], localization[3+severalNewElems]);
    snprintf(stringDisplayed, 6*TRAD_LENGTH, "%s %d %s %s%s%s\n%s", localization[0], nombreElement, localization[1+severalNewElems], localization[5+isTome], severalNewElems?"s ":" ", localization[7], localization[8]);
    title[0] += 'A' - 'a';
	
    bouton[1].flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
    bouton[1].buttonid = 1; //Valeur retournée
    bouton[1].text = localization[9]; //Accepté
    bouton[0].flags = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
    bouton[0].buttonid = 0;
    bouton[0].text = localization[10]; //Refusé
	
    alerte.flags = SDL_MESSAGEBOX_INFORMATION;
    alerte.title = title;
    alerte.message = stringDisplayed;
    alerte.numbuttons = 2;
    alerte.buttons = bouton;
    alerte.window = window;
    alerte.colorScheme = NULL;
	
	MUTEX_UNIX_LOCK;
	
    SDL_ShowMessageBox(&alerte, &ret_value);
	
	MUTEX_UNIX_UNLOCK;
	
    if(ret_value == 1)
        addtoDownloadListFromReader(mangaDB, firstNewElem+1, isTome);
	
    quit_thread(0);
}

extern int INSTANCE_RUNNING;
void addtoDownloadListFromReader(MANGAS_DATA mangaDB, int firstElem, bool isTome)
{
    FILE* updateControler = fopenR(INSTALL_DATABASE, "a+");
	if(updateControler != NULL)
	{
	    if(!isTome)
        {
            for(; mangaDB.chapitres[firstElem] != VALEUR_FIN_STRUCTURE_CHAPITRE; fprintf(updateControler, "%s %s C %d\n", mangaDB.team->teamCourt, mangaDB.mangaNameShort, mangaDB.chapitres[firstElem++]));
        }
        else
        {
            for(; mangaDB.tomes[firstElem].ID != VALEUR_FIN_STRUCTURE_CHAPITRE; fprintf(updateControler, "%s %s T %d\n", mangaDB.team->teamCourt, mangaDB.mangaNameShort, mangaDB.tomes[firstElem++].ID));
        }
		fclose(updateControler);
		if(checkLancementUpdate())
			createNewThread(lancementModuleDL, NULL);
        else
            INSTANCE_RUNNING = -1;
	}
}