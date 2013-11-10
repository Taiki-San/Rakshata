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
#include <locale.h> //Pour définir la langue

int affichageMenuGestion()
{
    char menus[SIZE_TRAD_ID_3][TRAD_LENGTH];
    SDL_Texture *texteAffiche = NULL;
    SDL_Rect position;
    TTF_Font *police;
    SDL_Color couleur = {palette.police.r, palette.police.g, palette.police.b};
    loadTrad(menus, 3);

    if(WINDOW_SIZE_H != HAUTEUR_SELECTION_REPO)
        updateWindowSize(LARGEUR, HAUTEUR_SELECTION_REPO);

    MUTEX_UNIX_LOCK;
    police = OpenFont(renderer, FONTUSED, POLICE_GROS);
    SDL_RenderClear(renderer);

    texteAffiche = TTF_Write(renderer, police, menus[0], couleur);
    if(texteAffiche != NULL)
    {
        position.x = WINDOW_SIZE_W / 2 - texteAffiche->w / 2;
        position.y = HAUTEUR_TEXTE;
        position.h = texteAffiche->h;
        position.w = texteAffiche->w;
        SDL_RenderCopy(renderer, texteAffiche, NULL, &position);
        SDL_DestroyTextureS(texteAffiche);
    }
    TTF_CloseFont(police);
    MUTEX_UNIX_UNLOCK;

    return displayMenu(&(menus[1]) , NOMBRE_MENU_GESTION, HAUTEUR_CHOIX, false);
}

int menuGestion()
{
    int menu = 0;
    while(menu > PALIER_MENU)
    {
        menu = affichageMenuGestion();
        switch(menu)
        {
            case PALIER_CHAPTER:
                menu = PALIER_MENU;
                break;

            case 1:
                menu = ajoutRepo(false); //Ajouter un dépot
                break;

            case 2:
                menu = deleteRepo(); //Supprimer un dépot
                if(menu == 1)
                {
                    resetUpdateDBCache();
                    raffraichissmenent(true);
                }
                break;

            case 3:
                menu = deleteManga(); //Supprimer des mangas
                break;

            case 4:
                raffraichissmenent(true); //Raffraichissement de la BDD
                break;

            case 5:
                menu = changementLangue(); //Changement de langue
                break;

            case 6:
                menu = PALIER_MENU;
                break;

            default:
                break;
        }
    }
    return menu;
}

char *loadPrefFile()
{
    size_t filesize;
    char *output = NULL;
    FILE* pref = fopen(SETTINGS_FILE, "r");
    if(pref == NULL)
    {
#ifdef DEV_VERSION
        logR("Failed at open settings");
#endif
        return NULL;
    }
    fseek(pref, 0, SEEK_END);
    filesize = ftell(pref);
    fclose(pref);

    if(filesize == 0)
    {
#ifdef DEV_VERSION
        logR("Empty file");
#endif
        return NULL;
    }
    output = calloc(filesize+10, sizeof(char));
    if(output == NULL)
    {
        return NULL;
    }
    AESDecrypt(SETTINGS_PASSWORD, SETTINGS_FILE, output, OUTPUT_IN_MEMORY);

    if(output[0] != '<' && output[1] != '<' && output[2] != '<' && output[3] != '<')
    {
        logR("Incorrect settings decryption\n");
        #ifdef DEV_VERSION
            logR(output);
        #endif
        free(output);
        return NULL;
    }
    return output;
}

void addToPref(char flag, char *stringToAdd)
{
    int i, j, length;
    char setFlag[10], *prefs = NULL, *newPrefs = NULL;
    snprintf(setFlag, 10, "<%c>", flag);

    prefs = loadPrefFile();
    if(prefs != NULL)
    {
        if(positionnementApresChar(prefs, setFlag))
            removeFromPref(flag);

        i = strlen(prefs);
        length = i + strlen(stringToAdd)+2;
        newPrefs = calloc(1, length+5);
        if(newPrefs == NULL)
        {
            free(prefs);
            return;
        }

        snprintf(newPrefs, length, "%s\n%s", prefs, stringToAdd);

        for(i = j = 2; i < length && newPrefs[j] != 0; i++, j++)
        {
            if(newPrefs[i-2] == '>' && newPrefs[i-1] == '\n' && newPrefs[j] == '\n')
                for(; newPrefs[j] == '\n'; j++);
            if(i != j)
                newPrefs[i] = newPrefs[j];
        }
        newPrefs[i] = 0;
        AESEncrypt(SETTINGS_PASSWORD, newPrefs, SETTINGS_FILE, INPUT_IN_MEMORY);
        free(newPrefs);
        free(prefs);
    }
    else
        AESEncrypt(SETTINGS_PASSWORD, stringToAdd, SETTINGS_FILE, INPUT_IN_MEMORY);
}

void removeFromPref(char flag)
{
    int i = 0, length = 0;
    char *newPrefs = NULL;
	char *prefs = NULL, *prefsBak;

    prefsBak = prefs = loadPrefFile();
    if(prefs == NULL)
    {
        removeR(SETTINGS_FILE);
        return;
    }
    length = strlen(prefs);
    newPrefs = calloc(length+1, sizeof(char));
    if(newPrefs == NULL)
    {
        free(prefs);
        return;
    }

    while(*prefs && i < length)
    {
        if(*prefs == '<' && *(prefs+1) == flag && *(prefs+2) == '>')
        {
            prefs += 3;
            while(*prefs && (*(prefs++) != '<' || *prefs != '/' || *(prefs+1) != flag || *(prefs+2) != '>')); //Balise suivante
            while(*prefs && *(prefs++) != '<'); //Balise suivante
            if(*prefs)
                prefs--;
        }
        else
            newPrefs[i++] = *(prefs++);
    }
    newPrefs[i] = 0;
    AESEncrypt(SETTINGS_PASSWORD, newPrefs, SETTINGS_FILE, INPUT_IN_MEMORY);
    free(newPrefs);
    free(prefsBak);
}

void updatePrefs(char flag, char *stringToAdd)
{
    removeFromPref(flag);
    addToPref(flag, stringToAdd);
}

int loadEmailProfile()
{
    char *prefs = loadPrefFile();
    if(prefs != NULL)
    {
		int i = 0;
        char flag[10];
        snprintf(flag, 10, "<%c>", SETTINGS_EMAIL_FLAG);
        if((i = positionnementApresChar(prefs, flag)))
        {
            sscanfs(&(prefs[i]), "%s", COMPTE_PRINCIPAL_MAIL, 100);
            free(prefs);
            for(i = 0; COMPTE_PRINCIPAL_MAIL[i] && COMPTE_PRINCIPAL_MAIL[i] != '@'; i++);
            if(COMPTE_PRINCIPAL_MAIL[i] && i != 0)
                return 1;
        }
        else
            free(prefs);
    }
    crashTemp(COMPTE_PRINCIPAL_MAIL, 100);
    return 0;
}

int loadLangueProfile()
{
    int i = 0;

    if(langue != 0)
        return 0;

    char *prefs = loadPrefFile();
    if(prefs != NULL)
    {
        char flag[10];
        snprintf(flag, 10, "<%c>", SETTINGS_LANGUE_FLAG);
        if((i = positionnementApresChar(prefs, flag)))
        {
            sscanfs(prefs+i, "%d", &langue);
            free(prefs);
            if(langue > 0 && langue <= NOMBRE_LANGUE)
                return 0;
            langue = 0;
        }
        else
            free(prefs);
    }
    mkdirR("data");
    char temp[100];

    char *langue_char = setlocale(LC_ALL, "");
    if(!strcmp(langue_char, "French_France.1252") || !strcmp(langue_char, "French_Belgium.1252") || !strcmp(langue_char, "French_Canada.1252") || !strcmp(langue_char, "French_Luxembourg.1252") || !strcmp(langue_char, "French_Principality of Monaco.1252") || !strcmp(langue_char, "French_Switzerland.1252"))
        langue = 1;
    else if(!strcmp(langue_char, "German_Austria.1252") || !strcmp(langue_char, "German_Liechtenstein.1252") || !strcmp(langue_char, "German_Luxembourg.1252") || !strcmp(langue_char, "German_Switzerland.1252") || !strcmp(langue_char, "German_Germany.1252"))
        langue = 3;
    else
        langue = LANGUE_PAR_DEFAUT;
    snprintf(temp, 100, "<%c>\n%d\n</%c>\n", SETTINGS_LANGUE_FLAG, langue, SETTINGS_LANGUE_FLAG);
    AESEncrypt(SETTINGS_PASSWORD, temp, SETTINGS_FILE, INPUT_IN_MEMORY);
    return 1;
}

char* loadLargePrefs(char flag)
{
    char *prefs, *basePtr;
    basePtr = prefs = loadPrefFile();
    if(prefs != NULL)
	{
		int i;
		size_t bufferSize = 0;
		char flag_db[10];
		snprintf(flag_db, 10, "<%c>", flag);
		if((i = positionnementApresChar(prefs, flag_db)) && *(prefs+i) != '<' && *(prefs+i+1) != '/')
		{
			prefs += i;
			while(prefs[++bufferSize] && (prefs[bufferSize] != '<' || prefs[bufferSize+1] != '/' || prefs[bufferSize+2] != flag || prefs[bufferSize+3] != '>'));
			char* databaseRAW = ralloc(bufferSize + 5);
			if(databaseRAW != NULL)
			{
				memccpy(databaseRAW, prefs, 0, bufferSize);
				databaseRAW[bufferSize] = 0;
                free(basePtr);
				return databaseRAW;

			}
		}
        free(basePtr);
	}
	if (flag == SETTINGS_MANGADB_FLAG || flag == SETTINGS_REPODB_FLAG)
    {
        removeFromPref(flag);
        char temp[200], buffer[65000], buffer2[65100];
        if(flag == SETTINGS_MANGADB_FLAG)
            snprintf(temp, 200, "https://%s/rec/%d/%s", SERVEUR_URL, CURRENTVERSION, MANGA_DATABASE);
        else
            snprintf(temp, 200, "https://%s/rec/%d/%s", SERVEUR_URL, CURRENTVERSION, REPO_DATABASE);

        crashTemp(buffer, 65000);
        download_mem(temp, NULL, buffer, 65000, 1);
        snprintf(buffer2, 65100, "<%c>\n%s\n</%c>\n", flag, buffer, flag);
        addToPref(flag, buffer2);
	}
    return NULL;
}

void setPrefs(MANGAS_DATA* mangaDB)
{
    int i = 1, length = 0, alreadyExist = 0, pos = 0, posBuf = 0, addOrRemove = mangaDB->favoris;
    char *favs = loadLargePrefs(SETTINGS_FAVORITE_FLAG), *favsBak = NULL, *favsNew = NULL;
    char mangaLong[LONGUEUR_NOM_MANGA_MAX], teamLong[LONGUEUR_NOM_MANGA_MAX];

    if(mangaDB == NULL)
        return;

    length = (favs!=NULL?strlen(favs):0) + strlen(mangaDB->mangaName) + strlen(mangaDB->team->teamLong) + 64;
    favsNew = malloc(length);
    if(favsNew == NULL)
    {
        char temp[256];
        snprintf(temp, 256, "Failed at allocate %d bytes\n", length);
        logR(temp);
        return;
    }
    else
        favsBak = favs;

    while(favs != NULL && !alreadyExist && *favs)
    {
        favs += sscanfs(favs, "%s %s", teamLong, LONGUEUR_NOM_MANGA_MAX, mangaLong, LONGUEUR_NOM_MANGA_MAX);
        for(; favs != NULL && *favs && (*favs == '\n' || *favs == '\r'); favs++);
        if(!strcmp(mangaDB->team->teamLong, teamLong) && !strcmp(mangaDB->mangaName, mangaLong))
            alreadyExist = !addOrRemove?1:i;
        i++;
    }
    if(!alreadyExist || addOrRemove)
    {
        favsNew[pos++] = '<';
        favsNew[pos++] = SETTINGS_FAVORITE_FLAG;
        favsNew[pos++] = '>';
        favsNew[pos++] = '\n';
        if(favs != NULL)
        {
            favs = favsBak;
            alreadyExist--;
            while(*favs && pos < length)
            {
                if(addOrRemove && (*favs == '\n' || favs == favsBak)) //Premier tour
                {
                    if(alreadyExist > 0)
                        alreadyExist--;
                    else if(alreadyExist == 0)
                    {
                        alreadyExist = -1;
                        for(; *favs == '\n'; favs++);
                        for(; *favs && *favs != '\n'; favs++);
                    }
                }
                if(*favs)
                    favsNew[pos++] = *favs++;
            }
            free(favsBak);
        }
        if(!addOrRemove) //On ajoute le manga seulement si on doit l'ajouter
        {
            for(posBuf = 0; posBuf < LONGUEUR_NOM_MANGA_MAX && mangaDB->team->teamLong[posBuf] && pos < length; favsNew[pos++] = mangaDB->team->teamLong[posBuf++]);
            favsNew[pos++] = ' ';
            for(posBuf = 0; posBuf < LONGUEUR_NOM_MANGA_MAX && mangaDB->mangaName[posBuf] && pos < length; favsNew[pos++] = mangaDB->mangaName[posBuf++]);
        }
        if(pos > 5) //Si il y a quelque chose
        {
            if(favsNew[pos-1] != '\n')
                favsNew[pos++] = '\n';
            favsNew[pos++] = '<';
            favsNew[pos++] = '/';
            favsNew[pos++] = SETTINGS_FAVORITE_FLAG;
            favsNew[pos++] = '>';
            favsNew[pos++] = '\n';
            favsNew[pos] = 0;
            updatePrefs(SETTINGS_FAVORITE_FLAG, favsNew);
        }
        else
            removeFromPref(SETTINGS_FAVORITE_FLAG);
        
        mangaDB->favoris = mangaDB->favoris?0:1;
    }
    free(favsNew);
}

