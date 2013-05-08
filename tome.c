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

int askForTome(MANGAS_DATA *mangaDB, int contexte)
{
    int buffer = 0, i = 0, tomeChoisis = 0, dernierLu = VALEUR_FIN_STRUCTURE_CHAPITRE;
    char temp[TAILLE_BUFFER], texteTrad[SIZE_TRAD_ID_19][LONGUEURTEXTE];
    loadTrad(texteTrad, 19);

    if((i = autoSelectionTome(mangaDB, contexte)) != VALEUR_FIN_STRUCTURE_CHAPITRE)
        return i;

    snprintf(temp, TAILLE_BUFFER, "manga/%s/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName, CONFIGFILE);
    if(checkFileExist(temp) || contexte == CONTEXTE_DL)
    {
        /*Initialize internal chapter list*/
        if(contexte == CONTEXTE_DL)
        {
            refreshChaptersList(mangaDB);
            snprintf(temp, TAILLE_BUFFER, "manga/%s/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName, CONFIGFILE);
            if(checkFileExist(temp))
                dernierLu = VALEUR_FIN_STRUCTURE_CHAPITRE; //Si un manga est déjà installé, on le met dans le sens décroissant
        }
        else
        {
            refreshChaptersList(mangaDB);
            checkChapitreValable(mangaDB, &dernierLu);
            if(mangaDB->nombreTomes == PALIER_MENU)
                return PALIER_MENU;
        }

        //Generate chapter list
        MANGAS_DATA *chapitreDB = generateChapterList(*mangaDB, (dernierLu == VALEUR_FIN_STRUCTURE_CHAPITRE), contexte, texteTrad[14], texteTrad[0]);

        //Si liste vide
        i = errorEmptyChapterList(contexte, texteTrad);
        if(i < PALIER_DEFAULT)
            return i;

        displayTemplateTome(mangaDB, contexte, texteTrad, dernierLu);
        tomeChoisis = PALIER_QUIT-1;
        while(tomeChoisis == PALIER_QUIT-1)
        {
            do
            {
                tomeChoisis = displayMangas(chapitreDB, SECTION_CHAPITRE_ONLY, 0, mangaDB->nombreTomes>MANGAPARPAGE_TRI?BORDURE_SUP_SELEC_CHAPITRE_FULL:BORDURE_SUP_SELEC_CHAPITRE_PARTIAL);
                if(tomeChoisis == CODE_CLIC_LIEN_CHAPITRE) //Site team
                    ouvrirSite(mangaDB->team);
            }while((tomeChoisis == CODE_CLIC_LIEN_CHAPITRE) //On reste dans la boucle si on clic sur le site de la team
                    || (tomeChoisis > CODE_CLIC_LIEN_CHAPITRE && contexte == CONTEXTE_DL) //On reste dans la boucle si dans le module de DL on clic sur les trucs en bas (inactifs)
                    || (dernierLu == VALEUR_FIN_STRUCTURE_CHAPITRE && tomeChoisis == CODE_BOUTON_2_CHAPITRE)); //Si on clic sur premiere lecture en bas

            if(tomeChoisis > 0 && tomeChoisis < CODE_CLIC_LIEN_CHAPITRE)
                tomeChoisis = chapitreDB[tomeChoisis-1].pageInfos; //Contient le n° du chapitre

            else if(tomeChoisis == CODE_BOUTON_CHAPITRE_DL)
                tomeChoisis = mangaDB->chapitres[0];

            else if (tomeChoisis > CODE_CLIC_LIEN_CHAPITRE && contexte != CONTEXTE_DL)
            {
                if(tomeChoisis == CODE_BOUTON_1_CHAPITRE) //Premier chapitre
                    tomeChoisis = mangaDB->chapitres[0];

                else if(tomeChoisis == CODE_BOUTON_2_CHAPITRE)
                    tomeChoisis = dernierLu; //Dernier lu

                else
                    tomeChoisis = mangaDB->chapitres[mangaDB->nombreTomes-1]; //Dernier chapitre
            }

            else if(tomeChoisis < PALIER_QUIT) //Numéro entré manuellement
                tomeChoisis = tomeChoisis * -1 + PALIER_QUIT;
        }
        free(chapitreDB);
    }
    else
    {
        buffer = showError();
        if(buffer > PALIER_MENU) // Si pas de demande spéciale
            buffer = PALIER_MENU;
        return buffer;
    }
    return tomeChoisis;
}

void displayTemplateTome(MANGAS_DATA* mangaDB, int contexte, char texteTrad[SIZE_TRAD_ID_19][TRAD_LENGTH], int dernierLu)
{
    displayTemplateChapitreTome(mangaDB, contexte, 1, texteTrad, dernierLu);
}

int autoSelectionTome(MANGAS_DATA *mangaDB, int contexte)
{
    return autoSelectionChapitreTome(mangaDB, mangaDB->firstTome, mangaDB->lastTome, contexte);
}

MANGAS_DATA *generateTomeList(MANGAS_DATA mangaDB, bool ordreCroissant, int contexte, char* stringAll, char* stringGeneric)
{
    int i = 0;
    char temp[500], stringGenericUsable[TRAD_LENGTH];
    register FILE* file = NULL; //Make that stuff faster

    //On capitalise la première lettre
    if(strlen(stringGeneric) >= TRAD_LENGTH)
        stringGeneric[TRAD_LENGTH-1] = 0;
    strcpy(stringGenericUsable, stringGeneric);
    if(stringGenericUsable[0] >= 'a' && stringGenericUsable[0] <= 'z')
        stringGenericUsable[0] += 'A'-'a';

    /*On prépare maintenant la structure*/
    MANGAS_DATA *tomeDB = calloc(mangaDB.nombreTomes+2, sizeof(MANGAS_DATA));

    /************************************************************
    ** Génére le noms des chapitre en vérifiant leur existance **
    **              Si on télécharge, on met tout              **
    ************************************************************/
    int tomeCourant = 0;

    if(contexte != CONTEXTE_LECTURE)
    {
        tomeDB[tomeCourant].pageInfos = VALEUR_FIN_STRUCTURE_CHAPITRE;
        usstrcpy(tomeDB[tomeCourant].mangaName, LONGUEUR_NOM_MANGA_MAX, stringAll);
        tomeCourant++;
    }

    if(ordreCroissant)
        i = 0;
    else
        i = mangaDB.nombreTomes-1;
    while((i < mangaDB.nombreTomes && ordreCroissant) || (i >= 0 && !ordreCroissant))
    {
        snprintf(temp, 500, "manga/%s/%s/Tome_%d/%s", mangaDB.team->teamLong, mangaDB.mangaName, mangaDB.tomes[i].ID, CONFIGFILE);

        file = fopen(temp, "r"); //On utilise pas checkFileExist() car file est dans les registres et plus rapide
        if((file != NULL && contexte != CONTEXTE_DL) || (file == NULL && contexte == CONTEXTE_DL))
        {
            if(contexte != CONTEXTE_DL)
                fclose(file);
            tomeDB[tomeCourant].pageInfos = mangaDB.chapitres[i];
            if(mangaDB.tomes[i].name[0] != 0) //Si on a un nom custom
                usstrcpy(tomeDB[tomeCourant++].mangaName, LONGUEUR_NOM_MANGA_MAX, mangaDB.tomes[i].name);
            else
                snprintf(tomeDB[tomeCourant++].mangaName, LONGUEUR_NOM_MANGA_MAX, "%s %d", stringGenericUsable, mangaDB.tomes[i].ID);
        }
        else if(contexte == CONTEXTE_DL)
            fclose(file);
        if(ordreCroissant)
            i++;
        else
            i--;
    }
    tomeDB[tomeCourant].mangaName[0] = tomeDB[tomeCourant].pageInfos = VALEUR_FIN_STRUCTURE_CHAPITRE;

    if((tomeCourant == 1 && contexte != CONTEXTE_LECTURE) || (tomeCourant == 0 && contexte == CONTEXTE_LECTURE)) //Si il n'y a pas de chapitre
    {
        free(tomeDB);
        tomeDB = NULL;
    }
    return tomeDB;
}

