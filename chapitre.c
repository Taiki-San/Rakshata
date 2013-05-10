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

void refreshChaptersList(MANGAS_DATA *mangaDB)
{
    if(mangaDB->chapitres != NULL)
    {
        free(mangaDB->chapitres);
    }
    /*On commence par énumérer les chapitres spéciaux*/
    int nbElem = 0, i;
    char temp[TAILLE_BUFFER];
    snprintf(temp, TAILLE_BUFFER, "manga/%s/%s/chapDB", mangaDB->team->teamLong, mangaDB->mangaName);
    FILE* chapSpeciaux = fopenR(temp, "r");
    if(chapSpeciaux != NULL)
    {
        fscanfs(chapSpeciaux, "%d", &nbElem);
    }
    nbElem += mangaDB->lastChapter - mangaDB->firstChapter + 1;
    mangaDB->chapitres = ralloc((nbElem+5)*sizeof(int));
    for(i = 0; i < nbElem+5; mangaDB->chapitres[i++] = VALEUR_FIN_STRUCTURE_CHAPITRE);

    for(i = 0; i <= mangaDB->lastChapter-mangaDB->firstChapter && i < nbElem; i++)
        mangaDB->chapitres[i] = (mangaDB->firstChapter+i)*10;

    if(chapSpeciaux != NULL)
    {
        if(nbElem)
        {
            for(; i < nbElem && fgetc(chapSpeciaux) != EOF; i++)
            {
                fseek(chapSpeciaux, -1, SEEK_CUR);
                fscanfs(chapSpeciaux, "%d", &(mangaDB->chapitres[i]));
            }
        }
        fclose(chapSpeciaux);
    }
    qsort(mangaDB->chapitres, i-1, sizeof(int), sortNumbers);
    mangaDB->nombreChapitre = i;
}

void checkChapitreValable(MANGAS_DATA *mangaDB, int *dernierLu)
{
    int first = -1, end = -1, fBack, eBack, nbElem = 0;
    char temp[TAILLE_BUFFER*5];

    snprintf(temp, TAILLE_BUFFER*5, "manga/%s/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName, CONFIGFILE);
    FILE* file = fopenR(temp, "r");
    if(temp == NULL)
    {
        mangaDB->chapitres[0] = VALEUR_FIN_STRUCTURE_CHAPITRE;
        mangaDB->nombreChapitre = 0;
        return;
    }
    fscanfs(file, "%d %d", &fBack, &eBack);
    if(fgetc(file) != EOF)
    {
        fseek(file, -1, SEEK_CUR);
        fscanfs(file, "%d", dernierLu);
    }
    fclose(file);

    for(nbElem = 0; mangaDB->chapitres[nbElem] != VALEUR_FIN_STRUCTURE_CHAPITRE && nbElem < mangaDB->nombreChapitre; nbElem++)
    {
        if(!checkChapterReadable(*mangaDB, &mangaDB->chapitres[nbElem]))
            mangaDB->chapitres[nbElem] = VALEUR_FIN_STRUCTURE_CHAPITRE;
    }

    qsort(mangaDB->chapitres, nbElem, sizeof(int), sortNumbers);
    for(; nbElem > 0 && mangaDB->chapitres[nbElem-1] == VALEUR_FIN_STRUCTURE_CHAPITRE; nbElem--);

    if(nbElem == 0)
    {
        snprintf(temp, TAILLE_BUFFER, "manga/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName);
        removeFolder(temp);
        mangaDB->chapitres[0] = VALEUR_FIN_STRUCTURE_CHAPITRE;
        mangaDB->nombreChapitre = 0;
        return;
    }

    first = mangaDB->chapitres[0];
    end = mangaDB->chapitres[nbElem-1];

    if(first > *dernierLu && *dernierLu != VALEUR_FIN_STRUCTURE_CHAPITRE)
        *dernierLu = mangaDB->chapitres[0];

    else if(end < *dernierLu && *dernierLu != VALEUR_FIN_STRUCTURE_CHAPITRE)
        *dernierLu = mangaDB->chapitres[nbElem-1];

    if((first != fBack || end != eBack) && first <= end)
    {
        snprintf(temp, TAILLE_BUFFER, "manga/%s/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName, CONFIGFILE);
        file = fopenR(temp, "w+");
        if(temp != NULL)
        {
            fprintf(file, "%d %d", first, end);
            if(*dernierLu != VALEUR_FIN_STRUCTURE_CHAPITRE)
                fprintf(file, " %d", *dernierLu);
            fclose(file);
        }
    }
    else if(first > end)
    {
        snprintf(temp, TAILLE_BUFFER, "manga/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName);
        removeFolder(temp);
        mangaDB->chapitres[0] = VALEUR_FIN_STRUCTURE_CHAPITRE;
        mangaDB->nombreChapitre = 0;
        return;
    }
    mangaDB->nombreChapitre = nbElem;
}

void getUpdatedChapterList(MANGAS_DATA *mangaDB)
{
    int i = VALEUR_FIN_STRUCTURE_CHAPITRE;
    refreshChaptersList(mangaDB);
    checkChapitreValable(mangaDB, &i);
}

int askForChapter(MANGAS_DATA *mangaDB, int contexte)
{
    /*Initialisations*/
    int i = 0, chapitreChoisis = 0, dernierLu = VALEUR_FIN_STRUCTURE_CHAPITRE, chapitreMax;
    char temp[TAILLE_BUFFER], texteTrad[SIZE_TRAD_ID_19][LONGUEURTEXTE];
    loadTrad(texteTrad, 19);

    if((i = autoSelectionChapitre(mangaDB, contexte)) != VALEUR_FIN_STRUCTURE_CHAPITRE)
        return i;

    snprintf(temp, TAILLE_BUFFER, "manga/%s/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName, CONFIGFILE);
    if(checkFileExist(temp) || contexte == CONTEXTE_DL)
    {
        /*Initialize internal chapter list*/
        if(contexte == CONTEXTE_DL)
        {
            refreshChaptersList(mangaDB);
            if(checkFileExist(temp))
                dernierLu = VALEUR_FIN_STRUCTURE_CHAPITRE; //Si un manga est déjà installé, on le met dans le sens décroissant
        }
        else
        {
            refreshChaptersList(mangaDB);
            checkChapitreValable(mangaDB, &dernierLu);
            if(mangaDB->nombreChapitre == 0)
                return PALIER_MENU;
        }

        //Generate chapter list
        MANGAS_DATA *chapitreDB = generateChapterList(*mangaDB, (dernierLu == VALEUR_FIN_STRUCTURE_CHAPITRE), contexte, texteTrad[14], texteTrad[0]);
        if(chapitreDB == NULL) //Erreur de mémoire ou liste vide
            return errorEmptyCTList(contexte, 0, texteTrad);

        chapitreMax = dernierLu != VALEUR_FIN_STRUCTURE_CHAPITRE ? chapitreDB[(contexte != CONTEXTE_LECTURE)].pageInfos : chapitreDB[chapitreDB[0].nombreChapitre-1].pageInfos;

        displayTemplateChapitre(mangaDB, chapitreDB[0].nombreChapitre, contexte, texteTrad, dernierLu);
        chapitreChoisis = PALIER_QUIT-1;
        while(chapitreChoisis == PALIER_QUIT-1)
        {
            do
            {
                chapitreChoisis = displayMangas(chapitreDB, CONTEXTE_CHAPITRE, chapitreMax, chapitreDB[0].nombreChapitre>MANGAPARPAGE_TRI?BORDURE_SUP_SELEC_CHAPITRE_FULL:BORDURE_SUP_SELEC_CHAPITRE_PARTIAL);
                if(chapitreChoisis == CODE_CLIC_LIEN_CHAPITRE) //Site team
                    ouvrirSite(mangaDB->team);
            }while((chapitreChoisis == CODE_CLIC_LIEN_CHAPITRE) //On reste dans la boucle si on clic sur le site de la team
                    || (chapitreChoisis > CODE_CLIC_LIEN_CHAPITRE && contexte == CONTEXTE_DL) //On reste dans la boucle si dans le module de DL on clic sur les trucs en bas (inactifs)
                    || (dernierLu == VALEUR_FIN_STRUCTURE_CHAPITRE && chapitreChoisis == CODE_BOUTON_2_CHAPITRE)); //Si on clic sur premiere lecture en bas

            if(chapitreChoisis > 0 && chapitreChoisis < CODE_CLIC_LIEN_CHAPITRE)
                chapitreChoisis = chapitreDB[chapitreChoisis-1].pageInfos; //Contient le n° du chapitre

            else if(chapitreChoisis == CODE_BOUTON_CHAPITRE_DL)
                chapitreChoisis = mangaDB->chapitres[0];

            else if (chapitreChoisis > CODE_CLIC_LIEN_CHAPITRE && contexte != CONTEXTE_DL)
            {
                if(chapitreChoisis == CODE_BOUTON_1_CHAPITRE) //Premier chapitre
                    chapitreChoisis = mangaDB->chapitres[0];

                else if(chapitreChoisis == CODE_BOUTON_2_CHAPITRE)
                    chapitreChoisis = dernierLu; //Dernier lu

                else if(chapitreChoisis == CODE_BOUTON_3_CHAPITRE)
                    chapitreChoisis = mangaDB->chapitres[mangaDB->nombreChapitre-1]; //Dernier chapitre

                else
                    chapitreChoisis = PALIER_QUIT-1;
            }

            else if(chapitreChoisis < CODE_CHAPITRE_FREE) //Numéro entré manuellement
                chapitreChoisis = (chapitreChoisis  - CODE_CHAPITRE_FREE) * -1;
        }
        free(chapitreDB);
    }
    else
        chapitreChoisis = PALIER_CHAPTER;

    return chapitreChoisis;
}

void displayTemplateChapitre(MANGAS_DATA* mangaDB, int nombreElements, int contexte, char texteTrad[SIZE_TRAD_ID_19][TRAD_LENGTH], int dernierLu)
{
    displayTemplateChapitreTome(mangaDB, contexte, 0, nombreElements, texteTrad, dernierLu);
}

int autoSelectionChapitre(MANGAS_DATA *mangaDB, int contexte)
{
    return autoSelectionChapitreTome(mangaDB, mangaDB->firstChapter, mangaDB->lastChapter, contexte);
}

MANGAS_DATA *generateChapterList(MANGAS_DATA mangaDB, bool ordreCroissant, int contexte, char* stringAll, char* stringGeneric)
{
    int i = 0;
    char temp[500], stringGenericUsable[TRAD_LENGTH];
    register FILE* file = NULL; //Make that stuff faster

    if(strlen(stringGeneric) >= TRAD_LENGTH)
        stringGeneric[TRAD_LENGTH-1] = 0;
    strcpy(stringGenericUsable, stringGeneric);
    if(stringGenericUsable[0] >= 'a' && stringGenericUsable[0] <= 'z')
        stringGenericUsable[0] += 'A'-'a';

    /*On prépare maintenant la structure*/
    MANGAS_DATA *chapitreDB = calloc(mangaDB.nombreChapitre+2, sizeof(MANGAS_DATA));

    /************************************************************
    ** Génére le noms des chapitre en vérifiant leur existance **
    **              Si on télécharge, on met tout              **
    ************************************************************/
    int chapitreCourant = 0;

    if(contexte != CONTEXTE_LECTURE)
    {
        chapitreDB[chapitreCourant].pageInfos = VALEUR_FIN_STRUCTURE_CHAPITRE;
        usstrcpy(chapitreDB[chapitreCourant].mangaName, LONGUEUR_NOM_MANGA_MAX, stringAll);
        chapitreCourant++;
    }

    if(ordreCroissant)
        i = 0;
    else
        i = mangaDB.nombreChapitre-1;
    while((i < mangaDB.nombreChapitre && ordreCroissant) || (i >= 0 && !ordreCroissant))
    {
        if(mangaDB.chapitres[i] % 10)
            snprintf(temp, 500, "manga/%s/%s/Chapitre_%d.%d/%s", mangaDB.team->teamLong, mangaDB.mangaName, mangaDB.chapitres[i]/10, mangaDB.chapitres[i]%10, CONFIGFILE);
        else
            snprintf(temp, 500, "manga/%s/%s/Chapitre_%d/%s", mangaDB.team->teamLong, mangaDB.mangaName, mangaDB.chapitres[i]/10, CONFIGFILE);

        file = fopen(temp, "r"); //On utilise pas checkFileExist() car file est dans les registres et plus rapide
        if((file != NULL && contexte != CONTEXTE_DL) || (file == NULL && contexte == CONTEXTE_DL))
        {
            if(contexte != CONTEXTE_DL)
                fclose(file);
            chapitreDB[chapitreCourant].pageInfos = mangaDB.chapitres[i];
            if(mangaDB.chapitres[i]%10)
                snprintf(chapitreDB[chapitreCourant++].mangaName, LONGUEUR_NOM_MANGA_MAX, "%s %d.%d", stringGenericUsable, mangaDB.chapitres[i]/10, mangaDB.chapitres[i]%10);
            else
                snprintf(chapitreDB[chapitreCourant++].mangaName, LONGUEUR_NOM_MANGA_MAX, "%s %d", stringGenericUsable, mangaDB.chapitres[i]/10);
        }
        else if(contexte == CONTEXTE_DL)
            fclose(file);
        if(ordreCroissant)
            i++;
        else
            i--;
    }
    chapitreDB[chapitreCourant].mangaName[0] = 0;
    chapitreDB[chapitreCourant].pageInfos = VALEUR_FIN_STRUCTURE_CHAPITRE;
    chapitreDB[0].nombreChapitre = chapitreCourant;

    if((chapitreCourant == 1 && contexte != CONTEXTE_LECTURE) || (chapitreCourant == 0 && contexte == CONTEXTE_LECTURE)) //Si il n'y a pas de chapitre
    {
        free(chapitreDB);
        chapitreDB = NULL;
    }
    return chapitreDB;
}

