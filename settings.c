/*********************************************************************************************
**      __________         __           .__            __                ____     ____      **
**      \______   \_____  |  | __  _____|  |__ _____ _/  |______    /\  /_   |   /_   |     **
**       |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \   \/   |   |    |   |     **
**       |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_ /\   |   |    |   |     **
**       |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  / \/   |___| /\ |___|     **
**              \/      \/     \/     \/     \/     \/          \/             \/           **
**                                                                                          **
**   Licence propriétaire, code source confidentiel, distribution formellement interdite    **
**                                                                                          **
*********************************************************************************************/

#include "main.h"

int affichageMenuGestion()
{
    int i = 0, j = 0;
    char menus[SIZE_TRAD_ID_3][LONGUEURTEXTE];
    SDL_Texture *texteAffiche = NULL;
    SDL_Rect position;
    TTF_Font *police;
    SDL_Color couleur = {palette.police.r, palette.police.g, palette.police.b};
    police = TTF_OpenFont(FONTUSED, POLICE_GROS);

    if(WINDOW_SIZE_H != HAUTEUR_SELECTION_REPO)
        updateWindowSize(LARGEUR, HAUTEUR_SELECTION_REPO);
    SDL_RenderClear(renderer);

    for(i = 0; i < SIZE_TRAD_ID_3; i++)
        for(j = 0; j < LONGUEURTEXTE; menus[i][j++] = 0);

    /*Remplissage des variables*/
    loadTrad(menus, 3);

    texteAffiche = TTF_Write(renderer, police, menus[0], couleur);
    position.x = WINDOW_SIZE_W / 2 - texteAffiche->w / 2;
    position.y = HAUTEUR_TEXTE;
    position.h = texteAffiche->h;
    position.w = texteAffiche->w;
    SDL_RenderCopy(renderer, texteAffiche, NULL, &position);
    SDL_DestroyTextureS(texteAffiche);
    TTF_CloseFont(police);

    return displayMenu(&(menus[1]) , NOMBRE_MENU_GESTION, HAUTEUR_CHOIX);
}

int menuGestion()
{
    int menu = 0;
    while(menu > -3)
    {
        menu = affichageMenuGestion();
        switch(menu)
        {
            case -2:
                menu = -3;
                break;

            case 1:
                /*Ajouter un dépot*/
                menu = ajoutRepo();
                if(menu == 1)
                    raffraichissmenent();
                break;

            case 2:
                /*Supprimer un dépot*/
                menu = deleteRepo();
                if(menu == 1)
                    raffraichissmenent();
                break;

            case 3:
                /*Supprimer des mangas*/
                menu = deleteManga();

                if(menu == -5)
                    menu = -2;
                break;

            case 4:
                /*Raffraîchissement de la BDD*/
                raffraichissmenent();
                break;

            case 5:
                /*mettre un mot de passe?*/
                break;

            case 6:
                menu = changementLangue(); //Changement de langue
                break;

            default:
                break;
        }
    }
    return menu;
}

char *loadPrefFile()
{
    char *output = NULL;
    FILE* pref = NULL;
    pref = fopenR(SETTINGS_FILE, "r");
    if(pref == NULL)
    {
#ifndef DEV_VERSION
        removeFolder("manga");
#endif
        return NULL;
    }
    fseek(pref, 0, SEEK_END);
    output = calloc(1, ftell(pref) + 10); //Set at 0

    if(output == NULL)
    {
        char temp[256];
        snprintf(temp, 256, "Failed at allocate memory for : %ld bytes\n", (ftell(pref) + 10));
        logR(temp);
        return NULL;
    }

    fclose(pref);
    AESDecrypt(SETTINGS_PASSWORD, SETTINGS_FILE, output, OUTPUT_IN_MEMORY);

    if(output[0] != '<' && output[1] != '<' && output[2] != '<' && output[3] != '<')
    {
        free(output);
        logR("Incorrect settings decryption\n");
        #ifdef DEV_VERSION
            logR(output);
        #endif
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
        length = i + strlen(stringToAdd);
        newPrefs = calloc(1, length+5);
        if(prefs == NULL)
            return;

        ustrcpy(newPrefs, prefs);
        free(prefs);
        prefs = newPrefs;
        for(j = 0; i < length && stringToAdd[j]; newPrefs[i++] = stringToAdd[j++]);
        newPrefs[i] = 0;
        AESEncrypt(SETTINGS_PASSWORD, newPrefs, SETTINGS_FILE, INPUT_IN_MEMORY);
        free(prefs);
    }
    else
        AESEncrypt(SETTINGS_PASSWORD, stringToAdd, SETTINGS_FILE, INPUT_IN_MEMORY);
}

void removeFromPref(char flag)
{
    int i = 0, length = 0;
    char *newPrefs = NULL;
	char *prefs = NULL;

    prefs = loadPrefFile();
    if(prefs == NULL)
    {
        removeR(SETTINGS_FILE);
        return;
    }
    length = strlen(prefs);
    newPrefs = calloc(1, length);
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
        sprintf(flag, "<%c>", SETTINGS_EMAIL_FLAG);
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
    char *prefs = loadPrefFile();
    if(prefs != NULL)
    {
        char flag[10];
        sprintf(flag, "<%c>", SETTINGS_LANGUE_FLAG);
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
    langue = LANGUE_PAR_DEFAUT;
    sprintf(temp, "<%c>\n%d\n</%c>\n", SETTINGS_LANGUE_FLAG, langue, SETTINGS_LANGUE_FLAG);
    AESEncrypt(SETTINGS_PASSWORD, temp, SETTINGS_FILE, INPUT_IN_MEMORY);
    return 1;
}

char* loadLargePrefs(char flag)
{
	char *prefs = loadPrefFile();
	if(prefs != NULL)
	{
		int i;
		size_t bufferSize = 0;
		char flag_db[10];
		sprintf(flag_db, "<%c>", flag);
		if((i = positionnementApresChar(prefs, flag_db)) && *(prefs+i) != '<' && *(prefs+i+1) != '/')
		{
			prefs += i;
			while(prefs[++bufferSize] && (prefs[bufferSize] != '<' || prefs[bufferSize+1] != '/' || prefs[bufferSize+2] != flag || prefs[bufferSize+3] != '>'));
			char* databaseRAW = calloc(1, bufferSize + 5);
			if(databaseRAW != NULL)
			{
				memccpy(databaseRAW, prefs, 0, bufferSize);
				databaseRAW[bufferSize] = 0;
				return databaseRAW;

			}
		}
		else if (flag == SETTINGS_MANGADB_FLAG || flag == SETTINGS_REPODB_FLAG)
		{
		    removeFromPref(flag);
			char temp[200], *buffer = NULL, *buffer2 = NULL;
			if(flag == SETTINGS_MANGADB_FLAG)
				snprintf(temp, 200, "http://www.%s/Recover/%d/%s", MAIN_SERVER_URL[0], CURRENTVERSION, MANGA_DATABASE);
			else
				snprintf(temp, 200, "http://www.%s/Recover/%d/%s", MAIN_SERVER_URL[0], CURRENTVERSION, REPO_DATABASE);

			buffer = calloc(1, 0x10000);
			setupBufferDL(buffer, 0x10, 0x10, 0x10, 0x10);
			download(temp, buffer, 0);

			buffer2 = calloc(1, strlen(buffer)+50);
			snprintf(buffer2, strlen(buffer)+50, "<%c>\n%s\n</%c>\n", flag, buffer, flag);

			addToPref(flag, buffer2);
			free(buffer2);
			free(buffer);
		}
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

        free(favsNew);
        mangaDB->favoris = mangaDB->favoris?0:1;
    }
}

