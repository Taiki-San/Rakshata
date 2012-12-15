/*********************************************
**	        	 Rakshata v1.1 		        **
**     Licence propriétaire, code source    **
**        confidentiel, distribution        **
**          formellement interdite          **
**********************************************/

#include "main.h"

int section()
{
    /*Initialisation*/
    int i = 0, longueur[NOMBRESECTION], sectionChoisis = 0, hauteurTexte = 0;
    SDL_Surface *texte = NULL;
    TTF_Font *police = NULL;
    SDL_Rect position;
    SDL_Event event;

	char texteTrad[SIZE_TRAD_ID_17][LONGUEURTEXTE];

    SDL_Color couleurTexte = {POLICE_R, POLICE_G, POLICE_B};

    if(ecran->h != HAUTEUR_FENETRE_SECTION || fond->h != HAUTEUR_FENETRE_SECTION)
    {
        SDL_FreeSurfaceS(ecran);
        SDL_FreeSurfaceS(fond);
        ecran = SDL_SetVideoMode(LARGEUR, HAUTEUR_FENETRE_SECTION, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
        fond = SDL_CreateRGBSurface(SDL_HWSURFACE, LARGEUR, HAUTEUR_FENETRE_SECTION, 32, 0, 0 , 0, 0); //on initialise le fond
    #ifdef __APPLE__
        SDL_FillRect(fond, NULL, SDL_Swap32(SDL_MapRGB(ecran->format, FOND_R, FOND_G, FOND_B))); //We change background color
    #else
        SDL_FillRect(fond, NULL, SDL_MapRGB(ecran->format, FOND_R, FOND_G, FOND_B)); //We change background color
    #endif

    }

    /*Affichage du texte*/
    applyBackground();

    loadTrad(texteTrad, 17);

    police = TTF_OpenFont(FONTUSED, POLICE_MOYEN);
    TTF_SetFontStyle(police, TTF_STYLE_ITALIC);

    texte = TTF_RenderText_Blended(police, texteTrad[1], couleurTexte);
    position.x = (ecran->w / 2) - (texte->w / 2);
    position.y = BORDURE_SUP_MENU + texte->h + INTERLIGNE_MENU;
    SDL_BlitSurface(texte, NULL, ecran, &position);
    SDL_FreeSurfaceS(texte);

    TTF_CloseFont(police);
    police = TTF_OpenFont(FONTUSED, POLICE_GROS);

    texte = TTF_RenderText_Blended(police, texteTrad[0], couleurTexte);
    position.x = (ecran->w / 2) - (texte->w / 2);
    position.y = BORDURE_SUP_MENU;
    SDL_BlitSurface(texte, NULL, ecran, &position);
    SDL_FreeSurfaceS(texte);

    TTF_SetFontStyle(police, TTF_STYLE_UNDERLINE);

    for(i = 1; i <= NOMBRESECTION; i++)
    {
        texte = TTF_RenderText_Blended(police, texteTrad[i + 2], couleurTexte);
        if(i % 2 == 1) //Colonne de gauche
            position.x = ecran->w / 4 - texte->w / 2;
        else
            position.x = ecran->w - ecran->w / 4 - texte->w / 2;
        position.y = BORDURE_SUP_SECTION + ((texte->h + INTERLIGNE) * ((i+1) / 2));
        SDL_BlitSurface(texte, NULL, ecran, &position);
        longueur[i - 1] = texte->w / 2;
        hauteurTexte = texte->h;
        SDL_FreeSurfaceS(texte);
    }
    refresh_rendering;
    TTF_CloseFont(police);

    /*Attente de l'evenement*/
    while(!sectionChoisis || sectionChoisis > NOMBRESECTION)
    {
        SDL_WaitEvent(&event);
        switch(event.type)
        {
            case SDL_QUIT:
                sectionChoisis = PALIER_QUIT;
                break;

            case SDL_KEYDOWN: //If a keyboard letter is pushed
            {
				if ((KMOD_LMETA & event.key.keysym.mod) && event.key.keysym.sym == SDLK_q)
                    sectionChoisis = PALIER_QUIT;
				else
				{
					sectionChoisis = nombreEntree(event);
					switch(event.key.keysym.sym)
					{
						case SDLK_DELETE:
						case SDLK_BACKSPACE:
							sectionChoisis = -2;
							break;

						case SDLK_ESCAPE:
							sectionChoisis = -3;
							break;

                        case SDLK_l:
                            sectionChoisis = 1;
                            break;

                        case SDLK_t:
                            sectionChoisis = 2;
                            break;

                        case SDLK_c:
                            sectionChoisis = 3;
                            break;

                        case SDLK_g:
                            sectionChoisis = 4;
                            break;

						default: //If another one
							break;
					}
				}
                break;
            }

            case SDL_MOUSEBUTTONUP:
            {
                if(!clicNotSlide(event))
                    break;

                //Définis la hauteur du clic par rapport à notre liste
                for(i = 1; ((((hauteurTexte + INTERLIGNE_SECTION) * i + BORDURE_SUP_SECTION) > event.button.y) || ((hauteurTexte + INTERLIGNE_SECTION) * i + BORDURE_SUP_SECTION + hauteurTexte) < event.button.y) && i < NOMBRESECTION/2 + 1; i++);

                if(i > NOMBRESECTION/2)
                    i = 0;

                else
                {
                    int positionBaseEcran = 0, numberTested = 0;
                    if(event.button.x < ecran->w / 2)
                    {
                        numberTested = i * 2 - 1;
                        positionBaseEcran = ecran->w / 4;
                    }
                    else
                    {
                        numberTested = i * 2;
                        positionBaseEcran = ecran->w - ecran->w / 4;
                    }
                    if(positionBaseEcran + longueur[numberTested - 1] >= event.button.x && positionBaseEcran - longueur[numberTested - 1] <= event.button.x)
                        sectionChoisis = numberTested;
                }
            }

            default:
                break;
        }
        if(sectionChoisis > NOMBRESECTION)
            sectionChoisis = 0;
    }
    return sectionChoisis;
}

int manga(int sectionChoisis, int sectionManga[NOMBRE_MANGA_MAX], char mangaDispo[NOMBRE_MANGA_MAX][LONGUEUR_NOM_MANGA_MAX], int nombreChapitre)
{
    /*Initilisation*/
    int mangaChoisis = 0, i = 0, nombreMangaElligible = 0, hauteurDonnes = 0;
	char texteTrad[SIZE_TRAD_ID_18][LONGUEURTEXTE];
	SDL_Color couleurTexte = {POLICE_R, POLICE_G, POLICE_B};
    SDL_Surface *texte = NULL;
    SDL_Rect position;
    TTF_Font *police = NULL;

    for(nombreMangaElligible = 0; nombreMangaElligible < NOMBRE_MANGA_MAX && mangaDispo[nombreMangaElligible][0]; nombreMangaElligible++); //Enumération

    if(nombreMangaElligible > 0)
    {
        if(sectionChoisis == SECTION_DL && nombreMangaElligible <= 3)
            i = BORDURE_SUP_SELEC_MANGA + LARGEUR_MOYENNE_MANGA_PETIT + MINIINTERLIGNE + LARGEUR_BANDEAU_CONTROLE_SELECTION_MANGA;

        else if(sectionChoisis != SECTION_DL && nombreMangaElligible <= 6) //Affichage unicolonne
            i = BORDURE_SUP_SELEC_MANGA + nombreMangaElligible * (LARGEUR_MOYENNE_MANGA_GROS + INTERLIGNE) + nombreMangaElligible*10;

        else if(nombreMangaElligible <= MANGAPARPAGE_TRI)
            i = BORDURE_SUP_SELEC_MANGA + (LARGEUR_MOYENNE_MANGA_PETIT + MINIINTERLIGNE) * ((nombreMangaElligible / NBRCOLONNES_TRI)+1) + LARGEUR_BANDEAU_CONTROLE_SELECTION_MANGA;

        else
            i = BORDURE_SUP_SELEC_MANGA + (LARGEUR_MOYENNE_MANGA_PETIT + MINIINTERLIGNE) * (MANGAPARPAGE_TRI / NBRCOLONNES_TRI) + LARGEUR_BANDEAU_CONTROLE_SELECTION_MANGA;

        if(ecran->h != i || fond->h != i) //Empêche de redimensionner si unicolonne
        {
            SDL_FreeSurfaceS(ecran);
            SDL_FreeSurfaceS(fond);
            ecran = SDL_SetVideoMode(LARGEUR, i, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
            fond = SDL_CreateRGBSurface(SDL_HWSURFACE, LARGEUR, i, 32, 0, 0 , 0, 0); //on initialise le fond
    #ifdef __APPLE__
            SDL_FillRect(fond, NULL, SDL_Swap32(SDL_MapRGB(ecran->format, FOND_R, FOND_G, FOND_B))); //We change background color
    #else
            SDL_FillRect(fond, NULL, SDL_MapRGB(ecran->format, FOND_R, FOND_G, FOND_B)); //We change background color
    #endif
        }

        loadTrad(texteTrad, 18);

        applyBackground();

        police = TTF_OpenFont(FONTUSED, POLICE_MOYEN);
        TTF_SetFontStyle(police, TTF_STYLE_ITALIC);
        texte = TTF_RenderText_Blended(police, texteTrad[1], couleurTexte);
        position.x = (ecran->w / 2) - (texte->w / 2);
        position.y = BORDURE_SUP_TITRE_MANGA + texte->h + INTERLIGNE_MENU;
        SDL_BlitSurface(texte, NULL, ecran, &position);
        SDL_FreeSurfaceS(texte);

        TTF_CloseFont(police);
        police = TTF_OpenFont(FONTUSED, POLICE_GROS);

        if(sectionChoisis == SECTION_DL)
        {
            char temp[TAILLE_BUFFER];
            if(langue == 4) //Petit détail dans la syntax de la traduction allemande
                sprintf(temp, "%s %s", texteTrad[2], texteTrad[0]);
            else
                sprintf(temp, "%s %s", texteTrad[0], texteTrad[2]);
            texte = TTF_RenderText_Blended(police, temp, couleurTexte);
        }
        else
            texte = TTF_RenderText_Blended(police, texteTrad[0], couleurTexte);
        position.x = (ecran->w / 2) - (texte->w / 2);
        position.y = BORDURE_SUP_TITRE_MANGA;
        SDL_BlitSurface(texte, NULL, ecran, &position);
        SDL_FreeSurfaceS(texte);

        refresh_rendering;

        if(sectionChoisis == SECTION_CHOISIS_LECTURE)
            hauteurDonnes = BORDURE_SUP_SELEC_MANGA_LECTURE;
        else
            hauteurDonnes = BORDURE_SUP_SELEC_MANGA;

        /*Définition de l'affichage*/
        conversionAvant(mangaDispo);

        if((BORDURE_SUP_SELEC_MANGA + (LARGEUR_MOYENNE_MANGA_GROS + MINIINTERLIGNE) * nombreMangaElligible) < HAUTEUR_MAX && nombreMangaElligible < 6 && sectionChoisis!= SECTION_DL)
        {
            int mangaElligibles[NOMBRE_MANGA_MAX];
            for(i = nombreMangaElligible=0; sectionManga[i] != 0; mangaElligibles[nombreMangaElligible++] = i++);
            mangaElligibles[nombreMangaElligible] = -1;
            mangaChoisis = mangaUnicolonne(police, mangaDispo, sectionChoisis, nombreMangaElligible, mangaElligibles);
        }
        else
            mangaChoisis = mangaTriColonne(mangaDispo, sectionChoisis, sectionManga, nombreChapitre, hauteurDonnes);

        conversionApres(mangaDispo);
    }
    else
    {
        mangaChoisis = rienALire();
        if(mangaChoisis > -2)
            mangaChoisis = -2;
    }

    TTF_CloseFont(police);
    return mangaChoisis;
}

int chapitre(char team[LONGUEUR_NOM_MANGA_MAX], char mangaSoumis[LONGUEUR_NOM_MANGA_MAX], int mode)
{
    /*Initialisations*/
    int extreme[2] = {0, 0}, buffer = 0, i = 0, j = 0, k = 0, chapitreChoisis = 0, dernierLu = 0, nombreChapitre = 0;
    int hauteur_chapitre = 0;
    char temp[TAILLE_BUFFER] = {0}, nomsChapitre[NOMBRE_CHAPITRE_MAX+1][LONGUEUR_NOM_MANGA_MAX], texteTrad[SIZE_TRAD_ID_19][LONGUEURTEXTE];
    register FILE* checkE = NULL; //Make that stuff faster
    TTF_Font *police = NULL;
    SDL_Surface *texte = NULL;
    SDL_Color couleurTexte = {POLICE_R, POLICE_G, POLICE_B};
    SDL_Rect position;

	chargement();

	loadTrad(texteTrad, 19);

    /*On convertit mangaATester*/
    for(i = 0; i < LONGUEUR_NOM_MANGA_MAX; i++)
    {
        if(mangaSoumis[i] == ' ')
            mangaSoumis[i] = '_';
    }
    for(i = 0; i < NOMBRE_CHAPITRE_MAX; i++)
        for(dernierLu = 0; dernierLu < 100; nomsChapitre[i][dernierLu++] = 0);

    dernierLu = 0;

    /*Recherche la ligne puis le dernier chapitre disponible du manga*/

    checkE = fopenR(MANGA_DATABASE, "r");
    if(positionnementApres(checkE, mangaSoumis))
        fscanfs(checkE, "%s %d %d", temp, TAILLE_BUFFER, &extreme[0], &extreme[1]);
    else
        return -1;
    fclose(checkE);

    if(extreme[0] == extreme[1]) //Si une seul chapitre, on le séléctionne automatiquement
        return extreme[0];

    crashTemp(temp, TAILLE_BUFFER);
    sprintf(temp, "manga/%s/%s/%s", team, mangaSoumis, CONFIGFILE);
    checkE = fopenR(temp, "r");

    if(checkE != NULL || mode == 2)
    {
        if(checkE == NULL && mode == 2)
        {
            i = extreme[0];
            dernierLu = -1;
        }

        else if (mode != 2)
        {
            fscanfs(checkE, "%d %d", &i, &extreme[1]);
            if(fgetc(checkE) != EOF)
            {
                fseek(checkE, -1, SEEK_CUR);
                fscanfs(checkE, "%d", &dernierLu);
            }
            else
                dernierLu = -1;
            fclose(checkE);

            if(i < extreme[0]) //Si il y a des chapitre a supprimer
            {
                for(; i < extreme[0]; i++)
                {
                    crashTemp(temp, TAILLE_BUFFER);
                    sprintf(temp, "manga\\%s\\%s\\Chapitre_%d", team, mangaSoumis, i);
                    removeFolder(temp);
                }
                for(; checkE == NULL; i++)
                {
                    crashTemp(temp, TAILLE_BUFFER);
                    sprintf(temp, "manga/%s/%s/Chapitre_%d/%s", team, mangaSoumis, i, CONFIGFILE);
                    checkE = fopenR(temp, "r");
                }
                fclose(checkE);
                crashTemp(temp, TAILLE_BUFFER);
                sprintf(temp, "manga/%s/%s/%s", team, mangaSoumis, CONFIGFILE);
                checkE = fopenR(temp, "w+");
                fprintf(checkE, "%d %d", i, extreme[1]);
                if(dernierLu != -1)
                    fprintf(checkE, " %d", dernierLu);
                fclose(checkE);
            }
            else
                extreme[0] = i;

            if(mode == 3)
                dernierLu = -1;
        }

        /************************************************************
        ** Génère le noms des chapitre en vérifiant leur existance **
        **    Sauf si on télécharge auquel cas, c'est l'inverse    **
        ************************************************************/
        nombreChapitre = 0;
        if(mode == 2 || mode == 3) //Bouton All
            sprintf(nomsChapitre[nombreChapitre++], "%s", texteTrad[11]);

        if(dernierLu == -1)
        {
            for(i = extreme[0]; i <= extreme[1]; i++)
            {
                crashTemp(temp, TAILLE_BUFFER);
                sprintf(temp, "manga/%s/%s/Chapitre_%d/%s", team, mangaSoumis, i, CONFIGFILE);
                checkE = fopenR(temp, "r");
                if(checkE != NULL && (mode == 1 || mode == 3))
                {
                    sprintf(nomsChapitre[nombreChapitre++], "%s %d", texteTrad[10], i);
                    fclose(checkE);
                }
                else if(checkE == NULL && mode == 2)
                    sprintf(nomsChapitre[nombreChapitre++], "%s %d", texteTrad[10], i);

                else if(checkE != NULL)
                    fclose(checkE);
            }
        }
        else
        {
            for(i = extreme[1]; i >= extreme[0]; i--)
            {
                crashTemp(temp, TAILLE_BUFFER);
                sprintf(temp, "manga/%s/%s/Chapitre_%d/%s", team, mangaSoumis, i, CONFIGFILE);
                checkE = fopenR(temp, "r");
                if(checkE != NULL && (mode == 1 || mode == 3))
                {
                    sprintf(nomsChapitre[nombreChapitre++], "%s %d", texteTrad[10], i);
                    fclose(checkE);
                }
                else if(checkE == NULL && mode == 2)
                {
                    sprintf(nomsChapitre[nombreChapitre++], "%s %d", texteTrad[10], i);
                }
                else if (checkE != NULL)
                    fclose(checkE);
            }
        }

        if(nombreChapitre <= MANGAPARPAGE_TRI)
		{
			if(nombreChapitre < NBRCOLONNES_TRI) //Si une seule ligne
				i = BORDURE_SUP_SELEC_MANGA + (LARGEUR_MOYENNE_MANGA_PETIT + MINIINTERLIGNE) * (nombreChapitre / NBRCOLONNES_TRI + 1) + 100;
			else
				i = BORDURE_SUP_SELEC_MANGA + (LARGEUR_MOYENNE_MANGA_PETIT + MINIINTERLIGNE) * (nombreChapitre / NBRCOLONNES_TRI + 1) + 50;
        }
		else
            i = BORDURE_SUP_SELEC_MANGA + (LARGEUR_MOYENNE_MANGA_PETIT + MINIINTERLIGNE) * (MANGAPARPAGE_TRI / NBRCOLONNES_TRI + 1) + 50;

        if(ecran->h != i || fond->h != i) //Empêche de redimensionner si unicolonne
        {
            SDL_FreeSurfaceS(ecran);
            SDL_FreeSurfaceS(fond);
            ecran = SDL_SetVideoMode(LARGEUR, i, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
            fond = SDL_CreateRGBSurface(SDL_HWSURFACE, LARGEUR, i, 32, 0, 0 , 0, 0); //on initialise le fond
#ifdef __APPLE__
            SDL_FillRect(fond, NULL, SDL_Swap32(SDL_MapRGB(ecran->format, FOND_R, FOND_G, FOND_B))); //We change background color
#else
            SDL_FillRect(fond, NULL, SDL_MapRGB(ecran->format, FOND_R, FOND_G, FOND_B)); //We change background color
#endif
        }
        applyBackground();

        police = TTF_OpenFont(FONTUSED, POLICE_GROS);
        crashTemp(temp, TAILLE_BUFFER);

        if(j == 1 && mode == 2) //Si aucun chapitre (uniquement au DL)
        {
            TTF_SetFontStyle(police, TTF_STYLE_UNDERLINE);
            texte = TTF_RenderText_Blended(police, texteTrad[12], couleurTexte);
            position.x = ecran->w / 2 - texte->w / 2;
            position.y = ecran->h / 2 - texte->h;
            SDL_BlitSurface(texte, NULL, ecran, &position);
            texte = TTF_RenderText_Blended(police, texteTrad[13], couleurTexte);
            position.x = ecran->w / 2 - texte->w / 2;
            position.y = ecran->h / 2 + texte->h;
            SDL_BlitSurface(texte, NULL, ecran, &position);
            refresh_rendering;
            chapitreChoisis = waitEnter();
            if(chapitreChoisis > PALIER_CHAPTER)
                chapitreChoisis = PALIER_CHAPTER;
            TTF_SetFontStyle(police, TTF_STYLE_NORMAL);
        }
        else
        {
			int section_inutile_mais_reclame[NOMBRE_MANGA_MAX]; //Je le mettais beaucoup plus bas mais notre cher VS ne supporte pas le C99 <3

            if(j == 1)
                k = j;
            //On affiche pas le même titre en fonction de la section
            sprintf(temp, "%s %s", texteTrad[0], texteTrad[mode]);
            texte = TTF_RenderText_Blended(police, temp, couleurTexte);
            position.x = (ecran->w / 2) - (texte->w / 2);
            position.y = BORDURE_SUP_TITRE_CHAPITRE;
            SDL_BlitSurface(texte, NULL, ecran, &position);
            SDL_FreeSurfaceS(texte);

            /*Affichage des infos sur la team*/
            crashTemp(temp, TAILLE_BUFFER);


            for(i = 0; i < LONGUEUR_NOM_MANGA_MAX; i++)
            {
                if(mangaSoumis[i] == '_')
                    mangaSoumis[i] = ' ';

                if(team[i] == '_')
                    team[i] = ' ';
            }

            sprintf(temp, "%s '%s' %s '%s'", texteTrad[6], mangaSoumis, texteTrad[7], team);

            for(i = 0; i < LONGUEUR_NOM_MANGA_MAX; i++)
            {
                if(mangaSoumis[i] == ' ')
                    mangaSoumis[i] = '_';

                if(team[i] == ' ')
                    team[i] = '_';
            }

            TTF_CloseFont(police);
            police = TTF_OpenFont(FONTUSED, POLICE_MOYEN);
            TTF_SetFontStyle(police, TTF_STYLE_UNDERLINE);

            texte = TTF_RenderText_Blended(police, temp, couleurTexte);
            position.x = (ecran->w / 2) - (texte->w / 2);
            position.y = BORDURE_SUP_INFOS_TEAM_CHAPITRE;
            SDL_BlitSurface(texte, NULL, ecran, &position);
            SDL_FreeSurfaceS(texte);


            /*Affichage des boutons du bas, central puis gauche, puis droit*/
            position.y = ecran->h - HAUTEUR_BOUTONS_CHAPITRE;
            if(mode == 1 || mode == 3)
            {
                crashTemp(temp, TAILLE_BUFFER);
                if(dernierLu == -1)
                   sprintf(temp, "%s", texteTrad[8]);
                else
                    sprintf(temp, "%s %d", texteTrad[9], dernierLu);

                texte = TTF_RenderText_Blended(police, temp, couleurTexte);
                position.x = ecran->w / 2 - texte->w / 2;
                SDL_BlitSurface(texte, NULL, ecran, &position);
                SDL_FreeSurfaceS(texte);
            }
            if(mode == 2)
                TTF_SetFontStyle(police, TTF_STYLE_NORMAL);
            crashTemp(temp, TAILLE_BUFFER);
            sprintf(temp, "%s %d", texteTrad[4], extreme[0]);
            texte = TTF_RenderText_Blended(police, temp, couleurTexte);
            position.x = BORDURE_BOUTON_LECTEUR;
            SDL_BlitSurface(texte, NULL, ecran, &position);
            SDL_FreeSurfaceS(texte);

            crashTemp(temp, TAILLE_BUFFER);
            sprintf(temp, "%s %d", texteTrad[5], extreme[1]);
            texte = TTF_RenderText_Blended(police, temp, couleurTexte);
            position.x = ecran->w - texte->w - BORDURE_BOUTON_LECTEUR;
            SDL_BlitSurface(texte, NULL, ecran, &position);
            SDL_FreeSurfaceS(texte);
            refresh_rendering;

            chapitreChoisis = -1;

            if(nombreChapitre >= MANGAPARPAGE_TRI)
                hauteur_chapitre = BORDURE_SUP_SELEC_MANGA;

            else
                hauteur_chapitre = BORDURE_SUP_SELEC_MANGA - 25;

            section_inutile_mais_reclame[nombreChapitre] = -1;
            while(chapitreChoisis == -1)
            {
                do
                {
                    section_inutile_mais_reclame[0] = 0;
                    chapitreChoisis = mangaTriColonne(nomsChapitre, SECTION_CHOISIS_CHAPITRE, section_inutile_mais_reclame, extreme[1], hauteur_chapitre);
                    if(chapitreChoisis == CODE_CLIC_LIEN_CHAPITRE) //Site team
                    {
                        /*Il nous faut le nom court du manga or, on a que le long*/
                        checkE = fopenR("data/repo", "r");
                        if(checkE == NULL)
                        {
                            logR("\"repo\" introuvable\n");
                            exit(0);
                        }
                        positionnementApres(checkE, team);
                        crashTemp(temp, TAILLE_BUFFER);
                        fscanfs(checkE, "%s ", temp, TAILLE_BUFFER);
                        ouvrirSite(temp);
                    }
                }while((chapitreChoisis == CODE_CLIC_LIEN_CHAPITRE) //On reste dans la boucle si on clic sur le site de la team
                        || (chapitreChoisis > CODE_CLIC_LIEN_CHAPITRE && mode == 2) //On reste dans la boucle si dans le module de DL on clic sur les trucs en bas (inactifs)
                        || (dernierLu == -1 && chapitreChoisis == CODE_BOUTON_2_CHAPITRE)); //Si on clic sur premiere lecture en bas

                if(mode != 1 && chapitreChoisis >= 0 && section_inutile_mais_reclame[0] != -1 //Entré manuellement
                   && chapitreChoisis < CODE_CLIC_LIEN_CHAPITRE)
                {
                    chapitreChoisis--;
                    if(chapitreChoisis == 0) //Si bouton all choisis
                        section_inutile_mais_reclame[0] = -1;
                }

                if(section_inutile_mais_reclame[0] != -1) //Si ce n'est pas all
                {
                    j = 0;
                    if(dernierLu == -1 && chapitreChoisis > 0 && chapitreChoisis < CODE_CLIC_LIEN_CHAPITRE)
                    {
                        if(mode != 2)
                        {
                            for(i = extreme[0]; i <= extreme[1] && chapitreChoisis != j; i++)
                            {
                                crashTemp(temp, TAILLE_BUFFER);
                                sprintf(temp, "manga/%s/%s/Chapitre_%d/%s", team, mangaSoumis, i, CONFIGFILE);
                                checkE = fopenR(temp, "r");
                                if(checkE != NULL)
                                {
                                    fclose(checkE);
                                    j++;
                                }
                            }
                        }

                        else
                        {
                            for(i = extreme[0]; i <= extreme[1] && chapitreChoisis != j; i++)
                            {
                                crashTemp(temp, TAILLE_BUFFER);
                                sprintf(temp, "manga/%s/%s/Chapitre_%d/%s", team, mangaSoumis, i, CONFIGFILE);
                                checkE = fopenR(temp, "r");
                                if(checkE == NULL)
                                    j++;
                                else
                                    fclose(checkE);
                            }
                        }
                        i--;
                        if(i <= extreme[1])
                            chapitreChoisis = i;
                        else
                            chapitreChoisis = -1;
                    }

                    else if (chapitreChoisis > 0 && chapitreChoisis < CODE_CLIC_LIEN_CHAPITRE)
                    {
                        if(mode != 2)
                        {
                            for(i = extreme[1]; i >= extreme[0] && chapitreChoisis != j; i--)
                            {
                                crashTemp(temp, TAILLE_BUFFER);
                                sprintf(temp, "manga/%s/%s/Chapitre_%d/%s", team, mangaSoumis, i, CONFIGFILE);
                                checkE = fopenR(temp, "r");
                                if(checkE != NULL)
                                {
                                    fclose(checkE);
                                    j++;
                                }
                            }
                        }

                        else
                        {
                            for(i = extreme[1]; i >= extreme[0] && chapitreChoisis != j; i--)
                            {
                                crashTemp(temp, TAILLE_BUFFER);
                                sprintf(temp, "manga/%s/%s/Chapitre_%d/%s", team, mangaSoumis, i, CONFIGFILE);
                                checkE = fopenR(temp, "r");
                                if(checkE == NULL)
                                    j++;
                                else
                                    fclose(checkE);
                            }
                        }
                        if(chapitreChoisis == j)
                            chapitreChoisis = i + 1;
                        else
                            chapitreChoisis = -1;
                    }

                    else if (chapitreChoisis > CODE_CLIC_LIEN_CHAPITRE && mode != 2)
                    {
                        if(chapitreChoisis == CODE_BOUTON_1_CHAPITRE) //Premier chapitre
                            chapitreChoisis = extreme[0];
                        else if(chapitreChoisis == CODE_BOUTON_2_CHAPITRE)
                        {
                            if(mode == 3)
                                chapitreChoisis = -1;
                            else
                                chapitreChoisis = dernierLu; //Dernier lu
                        }
                        else
                            chapitreChoisis = extreme[1]; //Dernier chapitre
                    }

                    else if(chapitreChoisis >= 0)
                        chapitreChoisis--;
                }

                if(k && chapitreChoisis > -1)
                    chapitreChoisis--;
            }
        }
    }
    else
    {
        buffer = showError();
        if(buffer > -3) // Si pas de demande spéciale
            buffer = -3;
        return buffer;
    }
    TTF_CloseFont(police);
    restartEcran();
    return chapitreChoisis;
}

