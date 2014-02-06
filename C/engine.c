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


/*Checks*/

int letterLimitationEnforced(int letter, char firstLetterOfTheManga)
{
    if(letter == 0 || firstLetterOfTheManga == letter)
        return 1;
    return 0;
}

int buttonLimitationEnforced(int button_selected[8], MANGAS_DATA mangaDB)
{
    if(mangaDB.genre == GENRE_HENTAI && !getUnlock()) //Hentai alors que verrouillé
        return 0;

    if(!checkButtonPressed(button_selected)) //Si aucun bouton n'est pressé
        return 1;

    if(button_selected[POS_BUTTON_STATUS_FAVORIS] == 1 && mangaDB.favoris && (mangaDB.genre != GENRE_HENTAI || getUnlock()))
        return 1; //Hentai favoris bloqués

    if((!checkFirstLineButtonPressed(button_selected) || button_selected[mangaDB.status - 1] == 1) //En cours/Suspendus/Terminé
	   && (!checkSecondLineButtonPressed(button_selected) || button_selected[mangaDB.genre + 3] == 1)) //Shonen/Shojo/Seinen
        return 1;

    return 0;
}

bool engineCheckIfToDisplay(int contexte, DATA_ENGINE input, int limitationLettre, int button_selected[8])
{
	if(contexte == CONTEXTE_CHAPITRE || contexte == CONTEXTE_TOME)
		return true;

	if(contexte == CONTEXTE_SUPPRESSION)
		return true;

	if(!letterLimitationEnforced(limitationLettre, input.data->mangaName[0]))
		return false;

	if(!buttonLimitationEnforced(button_selected, *input.data))
		return false;

	return true;
}

/* Stuffs */

int engineDefineElementClicked(int x, int y, int tailleTexte[ENGINE_NOMBRE_COLONNE][ENGINE_NOMBRE_LIGNE][2], int hauteurDebut, int nombreMaxElem)
{
    int ligne, colonne = 0, retinaDevider = 1;
    for(ligne = 0; ligne < ENGINE_NOMBRE_LIGNE && (y < hauteurDebut + ligne * (LARGEUR_MOYENNE_MANGA_PETIT + INTERLIGNE) || y > hauteurDebut + ligne * (LARGEUR_MOYENNE_MANGA_PETIT + INTERLIGNE) + LARGEUR_MOYENNE_MANGA_PETIT); ligne++);

    if(ligne < ENGINE_NOMBRE_LIGNE && y >= hauteurDebut) //Si on choisis un chapitre
    {
        /*Nombre Colonne*/
        for(colonne = 0; colonne < ENGINE_NOMBRE_COLONNE && ligne*ENGINE_NOMBRE_COLONNE + colonne < nombreMaxElem && (x < tailleTexte[colonne][ligne][0] / retinaDevider || x > tailleTexte[colonne][ligne][0] / retinaDevider + tailleTexte[colonne][ligne][1] / retinaDevider); colonne++);
        if(colonne < ENGINE_NOMBRE_COLONNE && nombreMaxElem > ligne * ENGINE_NOMBRE_COLONNE + colonne)
        {
            return colonne * 100 + ligne;
        }
    }
    return VALEUR_FIN_STRUCTURE_CHAPITRE;
}

bool unlocked = 0;
bool getUnlock()
{
	return unlocked;
}

void setUnlock(bool var)
{
	unlocked = var;
}


/*Multi Page*/
int curPage;

void loadMultiPage(int nombreManga, int *pageSelection, int *pageTotale)
{
    if (nombreManga > ENGINE_ELEMENT_PAR_PAGE)
    {
        *pageTotale = nombreManga / ENGINE_ELEMENT_PAR_PAGE;
        if(nombreManga % ENGINE_ELEMENT_PAR_PAGE > 0)
            *pageTotale += 1;
        if(*pageTotale < curPage)
            *pageSelection = curPage = *pageTotale;
    }
    else
    {
        *pageSelection = *pageTotale = curPage = 1;
    }
}

void engineLoadCurrentPage(int nombreElement, int pageCourante, int out[ENGINE_NOMBRE_COLONNE])
{
    if(pageCourante * ENGINE_ELEMENT_PAR_PAGE <= nombreElement) //Page complète
    {
        out[0] = 10;
        out[1] = 20;
        out[2] = 30;
    }
    else //Si page incomplète
    {
        int i;
        int elementParColonne = nombreElement % ENGINE_ELEMENT_PAR_PAGE / ENGINE_NOMBRE_COLONNE;
        int excedent = (nombreElement % ENGINE_ELEMENT_PAR_PAGE) % ENGINE_NOMBRE_COLONNE;

        for(i = 0; i < ENGINE_NOMBRE_COLONNE; i++)
            out[i] = elementParColonne * (i + 1);

        if(excedent != 0)
        {
            for(i = 0; i < excedent && i < ENGINE_NOMBRE_COLONNE; i++) //On impacte l'excedent
                out[i] += (i + 1);

            if(!out[1] && !out[2])
                out[1] = out[2] = out[0];
            else
            {
                for(; i < ENGINE_NOMBRE_COLONNE; i++)
                    out[i] = out[i] + i;
            }
        }
    }
}


/*CONTEXTE_LECTURE ONLY*/

void button_available(PREFS_ENGINE prefs, DATA_ENGINE* input, int button[8])
{
    int i, casTeste, nombreTour = prefs.nombreElementTotal;
    for(casTeste = 0; casTeste < 8; casTeste++)
    {
        if(casTeste == 3)
        {
            for(i = 0; i < nombreTour && input[i].data->mangaName[0] && input[i].data->favoris == 0; i++);

            if(i == nombreTour || !input[i].data->mangaName[0])
                button[casTeste] = -1;
            else
                button[casTeste] = 0;
        }
        else if(casTeste == 7 && !getUnlock())
        {
            button[casTeste] = -1;
        }
        else
        {
            for(i = 0; i < nombreTour && ((casTeste < 3 && input[i].data->status != casTeste+1) || (casTeste > 3 && input[i].data->genre != casTeste-3)); i++);

            if(i == nombreTour)
                button[casTeste] = -1;

            else
                button[casTeste] = 0;
        }
    }
}


