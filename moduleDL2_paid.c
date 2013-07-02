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
#include "moduleDL.h"

void MDLPHandle(DATA_LOADED ** data, int length)
{
    int *index = NULL;
    if(!MDLPCheckAnythingPayable(data, length))
        return;

    MDLPDispCheckingIfPaid();

    index = MDLPGeneratePaidIndex(data, length);
    if(index != NULL)
    {
        int sizeIndex;
        char * POSTRequest = MDLPCraftPOSTRequest(data, index);

        if(POSTRequest != NULL)
        {
            char URL[200], *bufferOut, *bufferOutBak;
            for(sizeIndex = 0; index[sizeIndex] != VALEUR_FIN_STRUCTURE_CHAPITRE; sizeIndex++);

            bufferOut = calloc(sizeIndex*2+10, sizeof(char)); //sizeIndex * 2 pour les espaces suivants les 0/1
            if(bufferOut != NULL)
            {
                bufferOutBak = bufferOut;
                snprintf(URL, 200, "https://rsp.%s/checkPaid.php", MAIN_SERVER_URL[0]);
                if(download_mem(URL, POSTRequest, bufferOut, sizeIndex*2+10, 1) == CODE_RETOUR_OK && isNbr(bufferOut[0]))
                {
                    int prix = 0;
                    sscanfs(bufferOut, "%d",&prix);
                    if(prix != 0)
                    {
                        for(; *bufferOut && *bufferOut != '\n'; bufferOut++);
                        for(; *bufferOut == '\n' || *bufferOut == '\r'; bufferOut++);
                        #warning "ajouter dans un tableau si paye (1) ou non (2)"
                    }
                }
                free(bufferOutBak);
            }
            free(POSTRequest);
        }
        free(index);
    }

    MDLPEraseDispChecking();
    return;
}

char *MDLPCraftPOSTRequest(DATA_LOADED ** data, int *index)
{
    int pos, length = strlen(COMPTE_PRINCIPAL_MAIL) + 50, compteur;
    char *output = NULL, buffer[500];
    void *buf;

    output = malloc(length * sizeof(char));
    if(output != NULL)
    {
        snprintf(output, length-1, "ver=1&mail=%s", COMPTE_PRINCIPAL_MAIL);
        for(pos = compteur = 0; index[pos] != VALEUR_FIN_STRUCTURE_CHAPITRE; compteur++)
        {
            snprintf(buffer, 500, "&editor%d=%s&proj%d=%s&isTome%d=%d&ID%d=%d", compteur, data[index[pos]]->datas->team->URL_depot, compteur, data[index[pos]]->datas->mangaName,
                                                                                compteur, data[index[pos]]->partOfTome != VALEUR_FIN_STRUCTURE_CHAPITRE,
                                                                                compteur, data[index[pos]]->partOfTome != VALEUR_FIN_STRUCTURE_CHAPITRE ? data[index[pos]]->partOfTome : data[index[pos]]->chapitre);
            if(data[index[pos]]->partOfTome != VALEUR_FIN_STRUCTURE_CHAPITRE)
            {
                int oldPos = pos;
                while(index[++pos] != VALEUR_FIN_STRUCTURE_CHAPITRE && data[index[oldPos]]->partOfTome == data[index[pos]]->partOfTome);
            }
            else
                pos++;

            length += strlen(buffer);
            buf = realloc(output, length * sizeof(char));
            if(buf != NULL)
            {
                output = buf;
                strend(output, length, buffer);
            }
        }
    }
    return output;
}

/** Checks **/

bool MDLPCheckAnythingPayable(DATA_LOADED ** data, int length)
{
    int i;
    for(i = 0; i < length; i++)
    {
        if(data[i] != NULL && data[i]->datas != NULL && data[i]->datas->team != NULL && !strcmp(data[i]->datas->team->type, TYPE_DEPOT_3))
            return true;
    }
    return false;
}

int * MDLPGeneratePaidIndex(DATA_LOADED ** data, int length)
{
    /*Optimisation possible: réduire la taille du tableau alloué*/
    int * output = malloc((length +1) * sizeof(int));
    if(output != NULL)
    {
        int i, posDansOut;
        for(i = posDansOut = 0; i < length; i++)
        {
            if(data[i] != NULL && data[i]->datas != NULL && data[i]->datas->team != NULL && !strcmp(data[i]->datas->team->type, TYPE_DEPOT_3))
                output[posDansOut++] = i;
        }
        output[posDansOut] = VALEUR_FIN_STRUCTURE_CHAPITRE;
    }
    return output;
}

/** UI **/

void MDLPDispCheckingIfPaid()
{
    char trad[SIZE_TRAD_ID_31][TRAD_LENGTH];
    SDL_Texture *texture = NULL;
    SDL_Rect position;
    SDL_Color couleur = {palette.police.r, palette.police.g, palette.police.b};
    TTF_Font *police = NULL;

    loadTrad(trad, 31);

    MUTEX_LOCK(mutexDispIcons);
    police = TTF_OpenFont(FONTUSED, MDL_SIZE_FONT_USED);

    if(police != NULL)
    {
        texture = TTF_Write(rendererDL, police, trad[0], couleur);
        if(texture != NULL)
        {
            position.h = texture->h;
            position.w = texture->w;
            position.x = rendererDL->window->w / 2 - position.w / 2;
            position.y = HAUTEUR_POURCENTAGE;
            SDL_RenderCopy(rendererDL, texture, NULL, &position);
            SDL_DestroyTexture(texture);
        }
        TTF_CloseFont(police);
    }
    MUTEX_UNLOCK(mutexDispIcons);
}

void MDLPEraseDispChecking()
{
    applyBackground(rendererDL, 0, HAUTEUR_POURCENTAGE-1, rendererDL->window->w, rendererDL->window->h - HAUTEUR_POURCENTAGE);
}

