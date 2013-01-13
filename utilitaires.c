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

void changeTo(char *string, int toFind, int toPut)
{
    while(*string)
    {
        if (*string == toFind)
            *string = toPut;
        string++;
    }
}

int plusOuMoins(int compare1, int compare2, int tolerance)
{
    if(compare1 + tolerance > compare2 && compare1 - tolerance < compare2)
        return 1;
    return 0;
}

int compare(const void *a, const void *b)
{
    const MANGAS_DATA *struc1 = a;
    const MANGAS_DATA *struc2 = b;
    return strcmp(struc1->mangaName, struc2->mangaName);
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
    int i = 0, length = strlen(stringToFind) + 10;
    char *temp = malloc(length);
    while((i = fgetc(stream)) != EOF && strcmp(temp, stringToFind))
    {
        fseek(stream, -1, SEEK_CUR);
        fscanfs(stream, "%s", temp, length);
    }
    free(temp);
    if(i == EOF)
        return 0;
    return 1;
}

int positionnementApresChar(char* input, char *stringToFind)
{
    int length = strlen(stringToFind) + 100, i = 0;//, j = 0;
    char *temp = malloc(length);

    while(input[i] && strcmp(temp, stringToFind))
    {
        for(; input[i] == '\r' || input[i] == '\n'; i++);
        i += sscanfs(&input[i], "%s", temp, length);
    }
    free(temp);
    if(!input[i])
        return 0;
    for(; input[i] == '\r' || input[i] == '\n'; i++);
    return i;
}

void teamOfProject(char nomProjet[LONGUEUR_NOM_MANGA_MAX], char nomTeam[LONGUEUR_NOM_MANGA_MAX])
{
    char temp[LONGUEUR_NOM_MANGA_MAX], buffer[LONGUEUR_COURT];
    char* manga = loadLargePrefs(SETTINGS_MANGADB_FLAG), *mangaBak;
    mangaBak = manga;

    manga += sscanfs(manga, "%s\n", nomTeam, LONGUEUR_NOM_MANGA_MAX);
    while(manga != NULL && strcmp(temp, nomProjet) && strcmp(buffer, nomProjet) && *manga)
    {
        if(*manga == '#')
        {
            manga++;
            manga += sscanfs(manga, "\n%s\n", nomTeam, LONGUEUR_NOM_MANGA_MAX);
        }
        else if(*manga)
            manga += sscanfs(manga, "%s %s\n", temp, LONGUEUR_NOM_MANGA_MAX, buffer, LONGUEUR_COURT);
        else
            break;
    }
    free(mangaBak);
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

