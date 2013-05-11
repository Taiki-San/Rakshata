/*********************************************************************************************
**  __________         __           .__            __                  ____     ________    **
**  \______   \_____  |  | __  _____|  |__ _____ _/  |______    ___  _/_   |    \_____  \   **
**   |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \   \  \/ /|   |     /  ____/   **
**   |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   / |   |    /       \   **
**   |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /   \_/  |___| /\ \_______ \  **
**          \/      \/     \/     \/     \/     \/          \/               \/         \/  **
**                                                                                          **
**    Licence propriétaire, code source confidentiel, distribution formellement interdite   **
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

int sortMangasToDownload(const void *a, const void *b)
{
    int ptsA = 0, ptsB = 0;
    const DATA_LOADED *struc1 = *(DATA_LOADED**) a;
    const DATA_LOADED *struc2 = *(DATA_LOADED**) b;

    if(struc1 == NULL)
        return 1;
    else if(struc2 == NULL)
        return -1;

    if(struc1->datas == struc2->datas) //Si même manga, ils pointent vers la même structure, pas besoin de compter les points
        return struc1->chapitre - struc2->chapitre;

    if(struc1->datas->favoris)
        ptsA = 2;
    if(!strcmp(struc1->datas->team->type, TYPE_DEPOT_3))
        ptsA += 1;

    if(struc2->datas->favoris)
        ptsB = 2;
    if(!strcmp(struc2->datas->team->type, TYPE_DEPOT_3))
        ptsB += 1;

    if(ptsA > ptsB)
        return -1;
    else if(ptsA < ptsB)
        return 1;
    else
		return strcmp(struc1->datas->mangaName, struc2->datas->mangaName);
}

int sortNumbers(const void *a, const void *b)
{
    if(*(int*)a == VALEUR_FIN_STRUCTURE_CHAPITRE)
        return 1;
    else if(*(int*)b == VALEUR_FIN_STRUCTURE_CHAPITRE)
        return -1;
    return ( *(int*)a - *(int*)b );
}

int sortMangas(const void *a, const void *b)
{
    const MANGAS_DATA *struc1 = a;
    const MANGAS_DATA *struc2 = b;
    return strcmp(struc1->mangaName, struc2->mangaName);
}

int sortTomes(const void *a, const void *b)
{
    const META_TOME *struc1 = a;
    const META_TOME *struc2 = b;

    if(struc1->ID == VALEUR_FIN_STRUCTURE_CHAPITRE)
        return 1;
    else if(struc2->ID == VALEUR_FIN_STRUCTURE_CHAPITRE)
        return -1;

    return struc1->ID - struc2->ID;
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

void versionRak(char *output)
{
    int centaine = 0, dizaine = 0, unite = 0;

    centaine = CURRENTVERSION / 100;
    dizaine = CURRENTVERSION / 10 - (CURRENTVERSION / 100 * 10);
    unite = CURRENTVERSION - (CURRENTVERSION / 10 * 10);

    if(unite)
        snprintf(output, 10, "%d.%d.%d", centaine, dizaine, unite);

    else
        snprintf(output, 10, "%d.%d", centaine, dizaine);
}

void setupBufferDL(char *buffer, int size1, int size2, int size3, int size4)
{
    crashTemp(buffer, size1*size2*size3*size4);
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
    int i = 0;
    while(input[i])
    {
        for(; input[i] != stringToFind[0] && input[i] != 0; i++);
        if(input[i] == stringToFind[0])
        {
            int j = 0;
            for(; j < strlen(stringToFind) && input[j+i] && stringToFind[j] && input[j+i] == stringToFind[j]; j++);
            if(stringToFind[j] == 0)
            {
                i += j;
                for(; input[i] == '\r' || input[i] == '\n'; i++);
                break;
            }
            i++;
        }
    }
    if(!input[i])
        return 0;
    return i;
}

void teamOfProject(char nomProjet[LONGUEUR_NOM_MANGA_MAX], char nomTeam[LONGUEUR_NOM_MANGA_MAX])
{
	char temp[LONGUEUR_NOM_MANGA_MAX] = {0}, buffer[LONGUEUR_COURT] = {0};
    char* manga = loadLargePrefs(SETTINGS_MANGADB_FLAG), *mangaBak;
    mangaBak = manga;

    if(manga != NULL)
    {
        manga += sscanfs(manga, "%s", nomTeam, LONGUEUR_NOM_MANGA_MAX);
        for(; *manga && *manga != '\n'; manga++);
        for(; *manga == '\n'; manga++);
        while(strcmp(temp, nomProjet) && strcmp(buffer, nomProjet) && *manga)
        {
            if(*manga == '#')
            {
                manga++;
                for(;*manga == '\n'; manga++);
                manga += sscanfs(manga, "%s", nomTeam, LONGUEUR_NOM_MANGA_MAX);
                for(; *manga && *manga != '\n'; manga++);
                for(; *manga == '\n'; manga++);
            }
            else if(*manga)
            {
                manga += sscanfs(manga, "%s %s\n", temp, LONGUEUR_NOM_MANGA_MAX, buffer, LONGUEUR_COURT);
                for(; *manga && *manga != '\n'; manga++);
                for(; *manga == '\n'; manga++);
            }
            else
                break;
        }
        free(mangaBak);
    }
}

void createPath(char *output)
{
    int longueur_output = 0, i = 0;
    char folder[500];
    while(output[longueur_output])
    {
        for(; output[longueur_output] && output[longueur_output] != '/' && output[longueur_output] != '\\' && i < 500; folder[i++] = output[longueur_output++]);
        folder[i] = 0;
        if(output[longueur_output]) //On est pas au bout du path
        {
            mkdirR(folder);
            folder[i++] = '/'; //On ajoute un / au path Ã  construire
			longueur_output++;
#ifdef _WIN32
            if(output[longueur_output] == '\\')
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
    SDL_EventState(SDL_JOYAXISMOTION, SDL_DISABLE);
    SDL_EventState(SDL_JOYBALLMOTION, SDL_DISABLE);
    SDL_EventState(SDL_JOYHATMOTION, SDL_DISABLE);
    SDL_EventState(SDL_JOYBUTTONDOWN, SDL_DISABLE);
    SDL_EventState(SDL_JOYBUTTONUP, SDL_DISABLE);
    SDL_EventState(SDL_FINGERDOWN, SDL_DISABLE);
    SDL_EventState(SDL_FINGERUP, SDL_DISABLE);
    SDL_EventState(SDL_FINGERMOTION, SDL_DISABLE);
    SDL_EventState(SDL_CLIPBOARDUPDATE, SDL_DISABLE);
    SDL_EventState(SDL_DROPFILE, SDL_DISABLE);
    SDL_EventState(SDL_TEXTINPUT, SDL_ENABLE);
}

static int maxSize = 0;
int defineMaxTextureSize(int sizeIssue)
{
    if(maxSize)
        return maxSize;
    SDL_Texture *texture = NULL;
    for(sizeIssue = 16384; texture == NULL && sizeIssue > 0; sizeIssue -= 10)
    {
        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_UNKNOWN, SDL_TEXTUREACCESS_STATIC, sizeIssue, sizeIssue);
    }
    SDL_DestroyTexture(texture);
    maxSize = sizeIssue;
#ifdef DEV_BUILD
    char temp[100];
    snprintf(temp, 100, "Max size: %d\n", maxSize);
    logR(temp);
#endif
    return sizeIssue;
}

int isJPEG(void *input)
{
    unsigned char *_input = input;
    if(_input[0] == (unsigned char) '\xff' && _input[1] == (unsigned char) '\xd8')
        return 1;
    return 0;
}

int isPNG(void *input)
{
    unsigned char *_input = input;
    if(_input[0] == (unsigned char) '\x89' && _input[1] == (unsigned char) 'P' && _input[2] == (unsigned char) 'N' && _input[3] == (unsigned char) 'G')
        return 1;
    return 0;
}

