/*********************************************
**	        	 Rakshata v1.1 		        **
**     Licence propriétaire, code source    **
**        confidentiel, distribution        **
**          formellement interdite          **
**********************************************/

#include "main.h"

void crashTemp(void *temp, int longueur)
{
    memset(temp, 0, longueur);
}

void conversionAvant(char mangaDispo[NOMBRE_MANGA_MAX][LONGUEUR_NOM_MANGA_MAX])
{
    int i = 0;
    for(; i < NOMBRE_MANGA_MAX; i++)
        changeTo(mangaDispo[i], '_', ' ');
}

void conversionApres(char mangaDispo[NOMBRE_MANGA_MAX][LONGUEUR_NOM_MANGA_MAX])
{
    int i = 0;
    for(; i < NOMBRE_MANGA_MAX; i++)
        changeTo(mangaDispo[i], ' ', '_');
}

void changeTo(char *string, int toFind, int toPut)
{
    while(*string)
    {
        if (*string == toFind)
            *string = toPut;
        string++;
    }
}

int plusOuMoins(int compare1, int compare2, int tolerance) //Estime si la différence entre les deux comparés est inférieur Ã  tolerance
{
    if(compare1 + tolerance > compare2 && compare1 - tolerance < compare2)
        return 1;

    return 0;
}

int compare(const void *a, const void *b)
{
    const char *s1 = a;
    const char *s2 = b;
    return strcmp(s1, s2);
}

void restartEcran()
{
    if(WINDOW_SIZE_W != LARGEUR || WINDOW_SIZE_H != HAUTEUR)
        updateWindowSize(LARGEUR, HAUTEUR);

    applyBackground(0, 0, WINDOW_SIZE_W, WINDOW_SIZE_H);
    SDL_RenderPresent(renderer);
}

void chargement()
{
	/*Initialisateurs graphique*/
    SDL_Texture *texteAffiche = NULL;
    SDL_Rect position;
    TTF_Font *police = NULL;
    SDL_Color couleur = {POLICE_R, POLICE_G, POLICE_B};

	char texte[SIZE_TRAD_ID_8][100];

    police = TTF_OpenFont(FONTUSED, POLICE_GROS);

    applyBackground(0, 0, WINDOW_SIZE_W, WINDOW_SIZE_H);

    if(police == NULL)
    {
        SDL_RenderFillRect(renderer, NULL);
        SDL_RenderPresent(renderer);
        return;
    }

    if(tradAvailable())
        loadTrad(texte, 8);
    else
        sprintf(texte[0], "Chargement - Loading");

    texteAffiche = TTF_Write(renderer, police, texte[0], couleur);

    if(texteAffiche == NULL)
        return;

    position.x = WINDOW_SIZE_W / 2 - texteAffiche->w / 2;
    position.y = WINDOW_SIZE_H / 2 - texteAffiche->h / 2;
    position.h = texteAffiche->h;
    position.w = texteAffiche->w;
    SDL_RenderCopy(renderer, texteAffiche, NULL, &position);
    SDL_DestroyTextureS(texteAffiche);
    TTF_CloseFont(police);
    SDL_RenderPresent(renderer);
}

void applyWindowsPathCrap(void *input)
{
    #ifdef _WIN32
    int i = 0;
    unsigned char *temp = input;

    for(; temp[i] != '\0'; i++)
    {
        if(temp[i] == '/')
            temp[i] = '\\';
    }
    #endif
}

void applyBackground(int x, int y, int w, int h)
{
    SDL_Rect positionBack;
    positionBack.x = x;
    positionBack.y = y;
    positionBack.w = w;
    positionBack.h = h;
    SDL_RenderFillRect(renderer, &positionBack);
}

void nameWindow(const int value)
{
    char windowsName[128], trad[SIZE_TRAD_ID_25][100], versionOfSoftware[6];

    if(!tradAvailable())
    {
        if(langue == 1) //Français
            SDL_SetWindowTitle(window, "Rakshata - Environnement corrompu");
        else
            SDL_SetWindowTitle(window, "Rakshata - Environment corrupted");
        return;
    }

    version(versionOfSoftware);
    loadTrad(trad, 25);
    crashTemp(windowsName, 128);

    if(!value) //Si on affiche le nom de la fenetre standard
        sprintf(windowsName, "%s - %s - v%s", PROJECT_NAME, trad[value], versionOfSoftware); //Windows name

    else if (value == 1)
        sprintf(windowsName, "%s - %s - v%s", PROJECT_NAME, trad[1], versionOfSoftware); //Windows name

    else
        sprintf(windowsName, "%s - %s - v%s - (%d)", PROJECT_NAME, trad[1], versionOfSoftware, value - 1); //Windows name

    SDL_SetWindowTitle(window, windowsName);
}

void version(char *output)
{
    int centaine = 0, dizaine = 0, unite = 0;

    centaine = CURRENTVERSION / 100;
    dizaine = CURRENTVERSION / 10 - (CURRENTVERSION / 100 * 10);
    unite = CURRENTVERSION - (CURRENTVERSION / 10 * 10);

    if(unite)
        sprintf(output, "%d.%d.%d", centaine, dizaine, unite);

    else
        sprintf(output, "%d.%d", centaine, dizaine);
}

void setupBufferDL(char *buffer, int size1, int size2, int size3, int size4)
{
    crashTemp(buffer, size1*size2*size3);
    buffer[1] = size1;
    buffer[2] = size2;
    buffer[3] = size3;
    buffer[4] = size4;
}

int positionnementApres(FILE* stream, char *stringToFind)
{
    int i = 0;
    char temp[200];
    while((i = fgetc(stream)) != EOF && strcmp(temp, stringToFind))
    {
        fseek(stream, -1, SEEK_CUR);
        fscanfs(stream, "%s", temp, 200);
    }
    if(i == EOF)
        return 0;
    return 1;
}

void teamOfProject(char nomProjet[LONGUEUR_NOM_MANGA_MAX], char nomTeam[LONGUEUR_NOM_MANGA_MAX])
{
    int i = 0;

    char temp[LONGUEUR_NOM_MANGA_MAX], buffer[LONGUEUR_COURT];
    FILE* ressource = fopenR(MANGA_DATABASE, "r");

    fscanfs(ressource, "%s\n", nomTeam, LONGUEUR_NOM_MANGA_MAX);
    while(strcmp(temp, nomProjet) && strcmp(buffer, nomProjet))
    {
        if((i = fgetc(ressource)) == '#')
            fscanfs(ressource, "\n%s\n", nomTeam, LONGUEUR_NOM_MANGA_MAX);
        else if(i != EOF)
        {
            fseek(ressource, -1, SEEK_CUR);
            fscanfs(ressource, "%s %s\n", temp, LONGUEUR_NOM_MANGA_MAX, buffer, LONGUEUR_COURT);
        }
        else
            break;
    }
    fclose(ressource);
}

void createPath(char output[])
{
    int longueur_output = 0, i = 0;
    char folder[200];
    while(output[longueur_output])
    {
        for(; output[longueur_output] && output[longueur_output] != '/' && output[longueur_output] != '\\' && i < 200; folder[i++] = output[longueur_output++]);
        folder[i] = 0;
        mkdirR(folder);
        if(output[longueur_output]) //On est pas au bout du path
        {
            folder[i++] = '/'; //On ajoute un / au path Ã  construire
			longueur_output++;
#ifdef _WIN32
            for(; output[longueur_output] && output[longueur_output] == '\\' ; longueur_output++); //Sous windows, il y a deux \\ .
#endif
        }
    }

}

void getResolution()
{
    /*Define screen resolution*/
    SDL_DisplayMode data;
    SDL_GetCurrentDisplayMode(0, &data);
    RESOLUTION[0] = data.w;
    RESOLUTION[1] = data.h;
    HAUTEUR_MAX = RESOLUTION[1];
}

int isHexa(char caract)
{
    if((caract >= '0' && caract <= '9') || (caract >= 'a' && caract <= 'f') || (caract >= 'A' && caract <= 'F'))
        return 1;

    return 0;
}

void MajToMin(char* input)
{
    for(; *input; input++)
    {
        if(*input >= 'A' && *input <= 'Z')
            *input += 'a' - 'A';
    }
}

void minToMaj(char* input)
{
    for(; *input; input++)
    {
        if(*input >= 'a' && *input <= 'z')
            *input += 'A' - 'a';
    }
}

void hexToDec(const char *input, unsigned char *output)
{
    int i = 0, j = 0, c = 0, temp = 0;
    for(; *input; i++)
    {
        for(j = 0; j < 2; j++)
        {
            temp = 0;
            c = *input++;
            if(c >= '0' && c <= '9')
                temp = c - '0';

            else if(c >= 'a' && c <= 'f')
                temp = 10 + (c - 'a');

            else if(c >= 'A' && c <= 'F')
                temp = 10 + (c - 'A');

            else
                return;

            output[i] = output[i]*16 + temp;
        }
    }
}

void decToHex(const unsigned char *input, size_t length, char *output)
{
    int i = 0, j = 0, c = 0, temp = 0;
    for(; i < length*2;)
    {
        c = *input++;
        temp = c / 0x10;
        for(j = 0; j < 2; j++)
        {
            if(temp <= 0x9)
                output[i++] = temp + '0';
            else if (temp <= 0xF)
                output[i++] = (temp - 0xA) + 'A';
            else
                return;
            temp = c % 0x10;
        }
    }
}

void restrictEvent()
{
    SDL_EventState(SDL_SYSWMEVENT, SDL_DISABLE);
    SDL_EventState(SDL_TEXTEDITING, SDL_DISABLE);
    SDL_EventState(SDL_INPUTMOTION, SDL_DISABLE);
    SDL_EventState(SDL_INPUTBUTTONDOWN, SDL_DISABLE);
    SDL_EventState(SDL_INPUTBUTTONUP, SDL_DISABLE);
    SDL_EventState(SDL_INPUTWHEEL, SDL_DISABLE);
    SDL_EventState(SDL_INPUTPROXIMITYIN, SDL_DISABLE);
    SDL_EventState(SDL_INPUTPROXIMITYOUT, SDL_DISABLE);
    SDL_EventState(SDL_JOYAXISMOTION, SDL_DISABLE);
    SDL_EventState(SDL_JOYBALLMOTION, SDL_DISABLE);
    SDL_EventState(SDL_JOYHATMOTION, SDL_DISABLE);
    SDL_EventState(SDL_JOYBUTTONDOWN, SDL_DISABLE);
    SDL_EventState(SDL_JOYBUTTONUP, SDL_DISABLE);
    SDL_EventState(SDL_FINGERDOWN, SDL_DISABLE);
    SDL_EventState(SDL_FINGERUP, SDL_DISABLE);
    SDL_EventState(SDL_FINGERMOTION, SDL_DISABLE);
    SDL_EventState(SDL_TOUCHBUTTONDOWN, SDL_DISABLE);
    SDL_EventState(SDL_TOUCHBUTTONUP, SDL_DISABLE);
    SDL_EventState(SDL_CLIPBOARDUPDATE, SDL_DISABLE);
    SDL_EventState(SDL_DROPFILE, SDL_DISABLE);
    SDL_EventState(SDL_TEXTINPUT, SDL_ENABLE);
}

