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

extern volatile bool quit;
extern int **status;
char password[100];

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
                /*Interrogration du serveur*/
                bufferOutBak = bufferOut;
                snprintf(URL, 200, "https://rsp.%s/checkPaid.php", MAIN_SERVER_URL[0]);
                if(download_mem(URL, POSTRequest, bufferOut, sizeIndex*2+10, 1) == CODE_RETOUR_OK && isNbr(bufferOut[0]))
                {
                    int prix = 0, pos = 0;
                    sscanfs(bufferOut, "%d",&prix);
                    if(prix != 0)
                    {
                        int posStatusLocal = 0;
                        int ** statusLocal = calloc(sizeIndex+1, sizeof(int*));
                        if(statusLocal != NULL)
                        {
                            bool somethingToPay = false, needLogin = false;

                            /*Chargement du fichier*/
                            for(; *bufferOut && *bufferOut != '\n'; bufferOut++);
                            for(; *bufferOut == '\n' || *bufferOut == '\r'; bufferOut++);

                            while(pos < sizeIndex && *bufferOut)
                            {
                                for(; *bufferOut && !isNbr(*bufferOut); bufferOut++);
                                if(*bufferOut - '0' <= MDLP_HIGHEST_CODE)
                                {
                                    /*Sachant que la liste peut être réorganisée, on va copier les adresses
                                    des données dont on a besoin dans un tableau qui sera envoyé au thread*/

                                    switch(*bufferOut - '0')
                                    {
                                        case MDLP_CODE_ERROR:
                                        {
                                            *status[index[pos]] = MDL_CODE_INTERNAL_ERROR;
                                            break;
                                        }
                                        case MDLP_CODE_PAID:
                                        {
                                            *status[index[pos]] = MDL_CODE_WAITING_LOGIN;
                                            statusLocal[posStatusLocal++] = status[index[pos]]; //on assume que posStatusLocal <= pos donc check limite supérieure inutile
                                            needLogin = true;
                                            break;
                                        }
                                        case MDLP_CODE_TO_PAY:
                                        {
                                            *status[index[pos]] = MDL_CODE_WAITING_PAY;
                                            statusLocal[posStatusLocal++] = status[index[pos]]; //on assume que posStatusLocal <= pos donc check limite supérieure inutile
                                            needLogin = somethingToPay = true;
                                            break;
                                        }
                                    }
                                    pos++;
                                }
                            }
                            if(needLogin)
                            {
                                DATA_PAY * arg = malloc(sizeof(DATA_PAY));
                                if(arg != NULL)
                                {
                                    arg->prix = prix;
                                    arg->somethingToPay = somethingToPay;
                                    arg->sizeStatusLocal = posStatusLocal;
                                    arg->statusLocal = statusLocal;
                                    createNewThread(MDLPHandlePayProcedure, arg);
                                }
                                else
                                    free(statusLocal);
                            }
                        }
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
        snprintf(output, length-1, "ver=%d&mail=%s", CURRENTVERSION, COMPTE_PRINCIPAL_MAIL);
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

void MDLPHandlePayProcedure(DATA_PAY * arg)
{
    bool toPay = arg->somethingToPay, cancel = false;
    int prix = arg->prix, sizeStatusLocal = arg->sizeStatusLocal, **statusLocal = arg->statusLocal;
    free(arg);

    SDL_Window * windowAuth = NULL;
    SDL_Renderer *rendererAuth = NULL;

    MUTEX_LOCK(mutexRS);

    windowAuth = SDL_CreateWindow(PROJECT_NAME, RESOLUTION[0] / 2 - LARGEUR / 2, 25, LARGEUR, SIZE_WINDOWS_AUTHENTIFICATION, SDL_WINDOW_OPENGL|SDL_WINDOW_SHOWN|SDL_WINDOW_INPUT_FOCUS);
    loadIcon(windowAuth);
    nameWindow(windowAuth, 1);
    rendererAuth = setupRendererSafe(windowAuth);

    MUTEX_UNLOCK(mutexRS);

    if(getPassword(rendererAuth, password) == 1)
    {
        int i = 0;
        for(; i < sizeStatusLocal; i++)
        {
            if(*statusLocal[i] == MDL_CODE_WAITING_LOGIN)
                *statusLocal[i] = MDL_CODE_DEFAULT;
        }
        MDLUpdateIcons(false);

        if(toPay)
        {
            int out = 0;
            MDLPDispAskToPay(rendererAuth, prix);
            out = MDLPWaitEvent(rendererAuth);
            if(out == 1)   //Nop
            {
                for(i = 0; i < sizeStatusLocal; i++)
                {
                    if(*statusLocal[i] == MDL_CODE_WAITING_PAY)
                        *statusLocal[i] = MDL_CODE_UNPAID;
                }
                cancel = true;
            }
            else
            {
                char URLStore[300];
                snprintf(URLStore, 300, "http://store.rakshata.com/?mail=%s", COMPTE_PRINCIPAL_MAIL);
                ouvrirSite(URLStore);
            }
        }
    }
    else
        cancel = true;

    MUTEX_LOCK(mutexRS);

    SDL_DestroyRenderer(rendererAuth);
    SDL_DestroyWindow(windowAuth);

    MUTEX_UNLOCK(mutexRS);

    if(!cancel && toPay)
    {
        if(waitForGetPaid() == true)
        {
            int i;
            for(i = 0; i < sizeStatusLocal; i++)
            {
                if(*statusLocal[i] == MDL_CODE_WAITING_PAY)
                    *statusLocal[i] = MDL_CODE_DEFAULT;
            }
            //vérifier que le thread de DL tourne encore et si non, le relancer
        }
    }

    free(statusLocal);

    if(cancel)
        MDLPDestroyCache();

    quit_thread(0);
}

bool waitForGetPaid()
{
    do
    {
        SDL_Delay(500);
    } while(!MDLPCheckIfPaid() && quit == false);

    if(quit == false)
        return true;
    return false;
}

void MDLPDestroyCache()
{
    char output[100], URL[0x100], POST[120];

    snprintf(URL, 0x100, "https://rsp.%s/cancelOrder.php", MAIN_SERVER_URL[0]);
    snprintf(POST, 120, "mail=%s", COMPTE_PRINCIPAL_MAIL);
    download_mem(URL, POST, output, 100, 1);
}

/** Checks **/

bool MDLPCheckAnythingPayable(DATA_LOADED ** data, int length)
{
    int i;
    for(i = 0; i < length; i++)
    {
        if(data[i] != NULL && data[i]->datas != NULL && data[i]->datas->team != NULL && !strcmp(data[i]->datas->team->type, TYPE_DEPOT_3) && *status[i] == MDL_CODE_DEFAULT)
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
            if(data[i] != NULL && data[i]->datas != NULL && data[i]->datas->team != NULL && !strcmp(data[i]->datas->team->type, TYPE_DEPOT_3) && *status[i] == MDL_CODE_DEFAULT)
                output[posDansOut++] = i;
        }
        output[posDansOut] = VALEUR_FIN_STRUCTURE_CHAPITRE;
    }
    return output;
}

bool MDLPCheckIfPaid()
{
    char URL[300], POST[120], output[50];
    snprintf(URL, 300, "https://rsp.%s/checkOrder.php", MAIN_SERVER_URL[0]);
    snprintf(POST, 120, "mail=%s", COMPTE_PRINCIPAL_MAIL);
    if(download_mem(URL, POST, output, 50, 1) == CODE_RETOUR_OK)
    {
        if(output[0] == '1')
            return true;
    }
    return false;
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

    SDL_RenderPresent(rendererDL);
    MUTEX_UNLOCK(mutexDispIcons);
}

void MDLPDispAskToPay(SDL_Renderer * renderVar, int prix)
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
        SDL_RenderClear(renderVar);
        char buffer[TRAD_LENGTH+10];
        position.y = 20;

        snprintf(buffer, TRAD_LENGTH+10, trad[1], prix/100, prix%100);
        texture = TTF_Write(renderVar, police, buffer, couleur);
        if(texture != NULL)
        {
            position.h = texture->h;
            position.w = texture->w;
            position.x = renderVar->window->w / 2 - position.w / 2;
            SDL_RenderCopy(renderVar, texture, NULL, &position);
            SDL_DestroyTexture(texture);
        }

        position.y += MDL_INTERLIGNE;
        texture = TTF_Write(renderVar, police, trad[2], couleur);
        if(texture != NULL)
        {
            position.h = texture->h;
            position.w = texture->w;
            position.x = renderVar->window->w / 2 - position.w / 2;
            SDL_RenderCopy(renderVar, texture, NULL, &position);
            SDL_DestroyTexture(texture);
        }

        position.y += MDL_INTERLIGNE;
        texture = TTF_Write(renderVar, police, trad[3], couleur);
        if(texture != NULL)
        {
            position.h = texture->h;
            position.w = texture->w;
            position.x = renderVar->window->w / 2 - position.w / 2;
            SDL_RenderCopy(renderVar, texture, NULL, &position);
            SDL_DestroyTexture(texture);
        }

        TTF_SetFontStyle(police, TTF_STYLE_UNDERLINE|TTF_STYLE_BOLD);
        position.y = 20+3*MDL_INTERLIGNE + (renderVar->window->h - (20+3*MDL_INTERLIGNE)) / 2;

        texture = TTF_Write(renderVar, police, trad[4], couleur);
        if(texture != NULL)
        {
            position.h = texture->h;
            position.w = texture->w;
            position.y -= position.h / 2;
            position.x = renderVar->window->w / 4 - position.w / 2;
            SDL_RenderCopy(renderVar, texture, NULL, &position);
            SDL_DestroyTexture(texture);
        }

        texture = TTF_Write(renderVar, police, trad[5], couleur);
        if(texture != NULL)
        {
            position.h = texture->h;
            position.w = texture->w;
            position.x = renderVar->window->w / 2 + renderVar->window->w / 4 - position.w / 2;
            SDL_RenderCopy(renderVar, texture, NULL, &position);
            SDL_DestroyTexture(texture);
        }
        TTF_CloseFont(police);
    }
    SDL_RenderPresent(renderVar);
    MUTEX_UNLOCK(mutexDispIcons);
}

int MDLPWaitEvent(SDL_Renderer * renderVar)
{
    SDL_Event event;
    while(1)
    {
        SDL_WaitEvent(&event);
        if(haveInputFocus(&event, renderVar->window))
        {
            switch(event.type)
            {
                case SDL_WINDOWEVENT:
                {
                    if(event.window.event == SDL_WINDOWEVENT_CLOSE)
                        return 1; //Nop
                    else
                        SDL_RenderPresent(renderVar);
                    break;
                }
                case SDL_KEYDOWN:
                {
                    if(event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_ESCAPE)
                        return 1;   //Nop
                    else if(event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_KP_ENTER)
                        return 2;   //Eyup
                    break;
                }
                case SDL_MOUSEBUTTONUP:
                {
                    if(event.button.y >= 20+3*MDL_INTERLIGNE)
                    {
                        if(event.button.x > renderVar->window->w / 2)
                            return 1;   //Nop
                        else
                            return 2;   //Eyup
                    }
                    break;
                }
            }
        }
    }
    return 1; //Shouldn't happen
}

void MDLPEraseDispChecking()
{
    applyBackground(rendererDL, 0, HAUTEUR_POURCENTAGE-1, rendererDL->window->w, rendererDL->window->h - HAUTEUR_POURCENTAGE);
    SDL_RenderPresent(rendererDL);
}
