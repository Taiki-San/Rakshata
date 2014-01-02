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
        unsigned int randomID;
        char * POSTRequest = MDLPCraftPOSTRequest(data, index, &randomID);

        if(POSTRequest != NULL)
        {
            char URL[200], *bufferOut, *bufferOutBak;
            for(sizeIndex = 0; index[sizeIndex] != VALEUR_FIN_STRUCTURE_CHAPITRE; sizeIndex++);

            bufferOut = calloc(sizeIndex*2+10, sizeof(char)); //sizeIndex * 2 pour les espaces suivants les 0/1
            if(bufferOut != NULL)
            {
                /*Interrogration du serveur*/
                bufferOutBak = bufferOut;
                snprintf(URL, 200, "https://%s/checkPaid.php", SERVEUR_URL);
                if(download_mem(URL, POSTRequest, bufferOut, sizeIndex*2+10, SSL_ON) == CODE_RETOUR_OK && isNbr(bufferOut[0]))
                {
                    int prix = -1, pos = 0;
                    sscanfs(bufferOut, "%d", &prix);
                    if(prix != -1)
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
							
							for(; pos < sizeIndex; *status[index[pos++]] = MDL_CODE_INTERNAL_ERROR);	//Manque
							
                            if(needLogin)
                            {
                                DATA_PAY * arg = malloc(sizeof(DATA_PAY));
                                if(arg != NULL)
                                {
                                    arg->prix = prix;
                                    arg->somethingToPay = somethingToPay;
                                    arg->sizeStatusLocal = posStatusLocal;
                                    arg->statusLocal = statusLocal;
                                    arg->factureID = randomID;
                                    createNewThread(MDLPHandlePayProcedure, arg);
                                }
                                else
                                    free(statusLocal);
                            }
                            else
                                free(statusLocal);
                        }
                    }
					else
					{
						int pos;
						for(pos = 0; pos < sizeIndex; *status[index[pos++]] = MDL_CODE_INTERNAL_ERROR);
					}

                }
                else
				{
					int pos;
					for(pos = 0; pos < sizeIndex; *status[index[pos++]] = MDL_CODE_INTERNAL_ERROR);
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

char *MDLPCraftPOSTRequest(DATA_LOADED ** data, int *index, unsigned int *factureID)
{
    int pos, length = strlen(COMPTE_PRINCIPAL_MAIL) + 50, compteur;
    char *output = NULL, buffer[500];
    void *buf;

    do
    {
        *factureID = ~rand() | rand();
    }while(*factureID == 0);

    output = malloc(length * sizeof(char));
    if(output != NULL)
    {
		char bufferURLDepot[3*LONGUEUR_URL], bufferMangaName[3*LONGUEUR_NOM_MANGA_MAX], bufferEmail[3*sizeof(COMPTE_PRINCIPAL_MAIL)];
		
		checkIfCharToEscapeFromPOST(COMPTE_PRINCIPAL_MAIL, sizeof(COMPTE_PRINCIPAL_MAIL), bufferEmail);
        snprintf(output, length-1, "ver=%d&mail=%s&id=%d", CURRENTVERSION, COMPTE_PRINCIPAL_MAIL, *factureID);

        for(pos = compteur = 0; index[pos] != VALEUR_FIN_STRUCTURE_CHAPITRE; compteur++)
        {
			checkIfCharToEscapeFromPOST(data[index[pos]]->datas->team->URL_depot, LONGUEUR_URL, bufferURLDepot);
			checkIfCharToEscapeFromPOST(data[index[pos]]->datas->mangaName, LONGUEUR_NOM_MANGA_MAX, bufferMangaName);
			
            snprintf(buffer, 500, "&data[%d][editor]=%s&data[%d][proj]=%s&data[%d][isTome]=%d&data[%d][ID]=%d", compteur, data[index[pos]]->datas->team->URL_depot, compteur, data[index[pos]]->datas->mangaName,
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
    unsigned int factureID = arg->factureID;
    free(arg);

    SDL_Window * windowAuth = NULL;
    SDL_Renderer *rendererAuth = NULL;

    MUTEX_LOCK(mutexRS);

    windowAuth = SDL_CreateWindow(PROJECT_NAME, RESOLUTION[0] / 2 - LARGEUR / 2, 25, LARGEUR, SIZE_WINDOWS_AUTHENTIFICATION, CREATE_WINDOW_FLAG|SDL_WINDOW_SHOWN|SDL_WINDOW_INPUT_FOCUS);
    
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
                snprintf(URLStore, 300, "http://store.rakshata.com/?mail=%s&id=%d", COMPTE_PRINCIPAL_MAIL, factureID);
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
        if(waitForGetPaid(factureID) == true)
        {
            int i;
            for(i = 0; i < sizeStatusLocal; i++)
            {
                if(*statusLocal[i] == MDL_CODE_WAITING_PAY)
                    *statusLocal[i] = MDL_CODE_DEFAULT;
            }
        }
    }

    free(statusLocal);

    if(cancel)
        MDLPDestroyCache(factureID);

    quit_thread(0);
}

bool waitForGetPaid(unsigned int factureID)
{
    do
    {
        SDL_Delay(500);
    } while(!MDLPCheckIfPaid(factureID) && quit == false);

    if(quit == false)
        return true;
    return false;
}

void MDLPDestroyCache(unsigned int factureID)
{
    char output[100], URL[0x100], POST[120];

    snprintf(URL, 0x100, "https://%s/cancelOrder.php", SERVEUR_URL);
    snprintf(POST, 120, "mail=%s&id=%d", COMPTE_PRINCIPAL_MAIL, factureID);
    download_mem(URL, POST, output, 100, SSL_ON);
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

bool MDLPCheckIfPaid(unsigned int factureID)
{
    char URL[300], output[50];
    snprintf(URL, 300, "https://%s/order/%d", SERVEUR_URL, factureID);
    if(download_mem(URL, NULL, output, 50, SSL_ON) == CODE_RETOUR_OK)
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

    police = OpenFont(FONTUSED, MDL_SIZE_FONT_USED);

    if(police != NULL)
    {

        texture = MDLTUITTFWrite(police, trad[0], couleur);

#ifdef WIN_OPENGL_BUGGED
    MDLTUIRefresh();
#endif

        if(texture != NULL)
        {
            position.h = texture->h;
            position.w = texture->w;
            position.x = rendererDL->window->w / 2 - position.w / 2;
            position.y = HAUTEUR_POURCENTAGE * getRetinaZoom();
            MDLTUICopy(texture, NULL, &position);
            MDLTUIDestroyTexture(texture);
        }
        TTF_CloseFont(police);
        MDLTUIRefresh();
    }

}

void MDLPDispAskToPay(SDL_Renderer * renderVar, int prix)
{
    char trad[SIZE_TRAD_ID_31][TRAD_LENGTH];
    SDL_Texture *texture = NULL;
    SDL_Rect position;
    SDL_Color couleur = {palette.police.r, palette.police.g, palette.police.b};
    TTF_Font *police = NULL;

    loadTrad(trad, 31);

    police = OpenFont(FONTUSED, MDL_SIZE_FONT_USED);

    if(police != NULL)
    {
        SDL_RenderClear(renderVar);
        char buffer[TRAD_LENGTH+10];
        position.y = 20 * getRetinaZoom();

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

        position.y += MDL_INTERLIGNE * getRetinaZoom();
        texture = TTF_Write(renderVar, police, trad[2], couleur);
        if(texture != NULL)
        {
            position.h = texture->h;
            position.w = texture->w;
            position.x = renderVar->window->w / 2 - position.w / 2;
            SDL_RenderCopy(renderVar, texture, NULL, &position);
            SDL_DestroyTexture(texture);
        }

        position.y += MDL_INTERLIGNE * getRetinaZoom();
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
        position.y = (20+3*MDL_INTERLIGNE + (renderVar->window->h - (20+3*MDL_INTERLIGNE)) / 2) * getRetinaZoom();

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
        SDL_RenderPresent(renderVar);
    }
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
#ifdef WIN_OPENGL_BUGGED
    MDLTUIRefresh();
#endif
    MDLTUIBackground(0, (HAUTEUR_POURCENTAGE-1) * getRetinaZoom(), getW(rendererDL), getH(rendererDL) - HAUTEUR_POURCENTAGE * getRetinaZoom());
    MDLTUIRefresh();
}

