/*********************************************
**	        	 Rakshata v1.1 		        **
**     Licence propriétaire, code source    **
**        confidentiel, distribution        **
**          formellement interdite          **
**********************************************/

#include "main.h"

int ecranAccueil()
{
    /*En raison de la taille importante de la page d'acceuil (800kb),
    elle est enregistré une seule fois et on lui colle dessus la trad*/

    SDL_Surface *accueil = NULL;
	char temp[TAILLE_BUFFER];

    accueil = IMG_Load("data/acceuil.png");
    SDL_BlitSurface(accueil, NULL, ecran, NULL);
    SDL_FreeSurfaceS(accueil);

    sprintf(temp, "data/%s/acceuil.png", LANGUAGE_PATH[langue - 1]); //Traduction

    accueil = IMG_Load(temp);
    SDL_BlitSurface(accueil, NULL, ecran, NULL);
    SDL_FreeSurfaceS(accueil);

    refresh_rendering; //Refresh screen
    return waitEnter();
}

int showControls()
{
    SDL_Surface *controls = NULL;
    int retour = 0;
    char temp[TAILLE_BUFFER];
    SDL_Event event;

    crashTemp(temp, TAILLE_BUFFER);
    sprintf(temp, "data/%s/controls.png", LANGUAGE_PATH[langue - 1]);
    controls = IMG_Load(temp);

    if(ecran->h != controls->h)
    {
        SDL_FreeSurfaceS(ecran);
        SDL_FreeSurfaceS(fond);
        ecran = SDL_SetVideoMode(controls->w, controls->h, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
        fond = SDL_CreateRGBSurface(SDL_HWSURFACE, controls->w, controls->h, 32, 0, 0 , 0, 0); //on initialise le fond
    #ifdef __APPLE__
        SDL_FillRect(fond, NULL, SDL_Swap32(SDL_MapRGB(ecran->format, FOND_R, FOND_G, FOND_B))); //We change background color
    #else
        SDL_FillRect(fond, NULL, SDL_MapRGB(ecran->format, FOND_R, FOND_G, FOND_B)); //We change background color
    #endif

    }
    applyBackground();

    SDL_BlitSurface(controls, NULL, ecran, NULL);
    refresh_rendering;

    while(!retour)
    {
        event.type = -1;
        SDL_WaitEvent(&event);
        switch(event.type)
        {
            case SDL_QUIT:
                retour = PALIER_QUIT;
                break;

            case SDL_KEYDOWN: //If a keyboard letter is pushed
            {
                switch(event.key.keysym.sym)
                {
                    case SDLK_RETURN: //If return
                    case SDLK_KP_ENTER:
                        retour = 1; //quit the loop
                        break;

                    case SDLK_ESCAPE:
                        retour = -3;
                        break;

                    case SDLK_DELETE:
                    case SDLK_BACKSPACE:
                        retour = -2;
                        break;

                    default: //If other one
                        break;
                }
                break;
            }

            case SDL_MOUSEBUTTONUP:
            {
                if(!clicNotSlide(event))
                    break;

                if(event.button.x > ecran->w / 2 && event.button.y > ecran->h / 2)
                    #ifdef _WIN32
                    ShellExecute(NULL, "open", "http://www.rakshata.com/?page_id=31", NULL, NULL, SW_SHOWDEFAULT);
                    #else
                    {
                        char superTemp[200];
                        crashTemp(superTemp, 200);
                        #ifdef __APPLE__
                        sprintf(superTemp, "open http://www.rakshata.com/?page_id=31");
                        #else
                        sprintf(superTemp, "/etc/alternatives/x-www-browser %s", "http://www.rakshata.com/?page_id=31");
                        #endif
                        system(superTemp);
                    }
                    #endif
                else
                   retour = 1;
                break;
            }

            default:
                if ((KMOD_LMETA & event.key.keysym.mod) && event.key.keysym.sym == SDLK_q)
                    retour =
                    PALIER_QUIT;
                break;
        }
    }
    return retour;
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

                raffraichissmenent();
                break;

            case 2:
                /*Supprimer un dépot*/
                menu = deleteRepo();

                /*Raffraichissement*/
                raffraichissmenent();
                break;

            case 3:
                /*Supprimer des mangas*/
                menu = deleteManga();

                if(menu == -5)
                    menu = -2;
                break;

            case 4:
                /*RaffraÓchissement de la BDD*/
                raffraichissmenent();
                break;

            case 5:
                /*Changer langue*/
                menu = changementLangue();
                break;

            default:
                break;
        }
    }
    restartEcran();
    return menu;
}


