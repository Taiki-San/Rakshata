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

void tomeDBParser(MANGAS_DATA* mangaDB, unsigned char* buffer, size_t size)
{
    int pos, nombreMaxElems = 0;
    META_TOME *lines = NULL;
    for(pos = 0; pos < size; pos++)
    {
        if(buffer[pos] == '\n')
            nombreMaxElems++;
    }
    lines = calloc(nombreMaxElems+1, sizeof(META_TOME));

    int ligneCourante = 0, i;
    char ligne[100];
    pos = 0;
    while(ligneCourante < nombreMaxElems && pos < size)
    {
        for(i = 0; i < 99 && buffer[pos] != '\n' && pos < size; ligne[i++] = buffer[pos++]); //Charge la première ligne
        ligne[i] = 0; //i < 100

        if(buffer[pos] != '\n') //Finis la ligne
            while(pos < size && buffer[pos++] != '\n');
        while(pos < size && buffer[++pos] == '\n');

        for(i = 0; i < 10 && isNbr(ligne[i]); i++);
        if(ligne[i] == ' ' && i > 0 && i < 10) //Données saines
        {
            sscanfs(ligne, "%d", &lines[ligneCourante].ID);
            while(ligne[++i] == ' ');
            memccpy(lines[ligneCourante].name, &ligne[i], 0, MAX_TOME_NAME_LENGTH-1);
            ligneCourante++;
        }
    }
    if(ligneCourante == nombreMaxElems) //Aucun element invalide
        mangaDB->tomes = lines;
    else
    {
        mangaDB->tomes = malloc((ligneCourante+1)*sizeof(META_TOME));
        memcpy(mangaDB->tomes, lines, ligneCourante*sizeof(META_TOME));
        free(lines);
    }
    qsort(mangaDB->tomes, ligneCourante, sizeof(META_TOME), sortTomes);
    mangaDB->nombreTomes = ligneCourante;

    mangaDB->tomes[ligneCourante].name[0] = 0;
    mangaDB->tomes[ligneCourante].ID = VALEUR_FIN_STRUCTURE_CHAPITRE; //Un flag final, au cas où
}

void refreshTomeList(MANGAS_DATA *mangaDB)
{
    if(mangaDB->tomes != NULL)
    {
        free(mangaDB->tomes);
    }
    /*On commence par énumérer les chapitres spéciaux*/
    char temp[TAILLE_BUFFER];
    snprintf(temp, TAILLE_BUFFER, "manga/%s/%s/tomeDB", mangaDB->team->teamLong, mangaDB->mangaName);
    FILE* tomeDB = fopenR(temp, "r");
    if(tomeDB != NULL)
    {
        unsigned char *buffer = NULL;
        size_t size = 0;
        fseek(tomeDB, 0, SEEK_END);
        size = ftell(tomeDB);
        if(size != 0)
        {
            buffer = calloc(1, size+1);
            if(buffer == NULL)
                memoryError(size+1);
            else
            {
                rewind(tomeDB);
                fread(buffer, size, 1, tomeDB);
            }
        }
        fclose(tomeDB);
        if(buffer != NULL)
            tomeDBParser(mangaDB, buffer, size);
    }
}

void checkTomeValable(MANGAS_DATA *mangaDB, int *dernierLu)
{
    if(mangaDB != NULL)
        return;

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
        if(!checkChapterReadable(*mangaDB, mangaDB->chapitres[nbElem]))
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

void getUpdatedTomeList(MANGAS_DATA *mangaDB)
{
    //int i = VALEUR_FIN_STRUCTURE_CHAPITRE;
    refreshTomeList(mangaDB);
    //checkTomeValable(mangaDB, &i);
}

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
            refreshTomeList(mangaDB);
            if(checkFileExist(temp))
                dernierLu = VALEUR_FIN_STRUCTURE_CHAPITRE; //Si un manga est déjà installé, on le met dans le sens décroissant
        }
        else
        {
            refreshTomeList(mangaDB);
            checkTomeValable(mangaDB, &dernierLu);
            if(mangaDB->nombreTomes == PALIER_MENU)
                return PALIER_MENU;
        }

        //Generate chapter list
        MANGAS_DATA *tomeDB = generateTomeList(*mangaDB, (dernierLu == VALEUR_FIN_STRUCTURE_CHAPITRE), contexte, texteTrad[14], texteTrad[1]);

        //Si liste vide
        if(tomeDB == NULL) //Erreur de mémoire ou liste vide
            return errorEmptyCTList(contexte, 1, texteTrad);

        displayTemplateTome(mangaDB, tomeDB[0].nombreTomes, contexte, texteTrad, dernierLu);
        tomeChoisis = PALIER_QUIT-1;
        while(tomeChoisis == PALIER_QUIT-1)
        {
            tomeChoisis = displayMangas(tomeDB, CONTEXTE_TOME, 0, (tomeDB[0].nombreTomes+(contexte != CONTEXTE_LECTURE))>MANGAPARPAGE_TRI?BORDURE_SUP_SELEC_CHAPITRE_FULL:BORDURE_SUP_SELEC_CHAPITRE_PARTIAL);

            if(tomeChoisis == CODE_CLIC_LIEN_CHAPITRE) //Site team
            {
                ouvrirSite(mangaDB->team);
                tomeChoisis = PALIER_QUIT-1;
            }
            else if(tomeChoisis > VALEUR_FIN_STRUCTURE_CHAPITRE && tomeChoisis < CODE_CLIC_LIEN_CHAPITRE)
                tomeChoisis = tomeDB[tomeChoisis-1].pageInfos; //Contient le n° du chapitre
            else if (tomeChoisis < PALIER_QUIT || tomeChoisis >= CODE_CLIC_LIEN_CHAPITRE)
                tomeChoisis = PALIER_QUIT-1;
        }
        free(tomeDB);
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

void displayTemplateTome(MANGAS_DATA* mangaDB, int nombreElements, int contexte, char texteTrad[SIZE_TRAD_ID_19][TRAD_LENGTH], int dernierLu)
{
    displayTemplateChapitreTome(mangaDB, contexte, 1, nombreElements, texteTrad, dernierLu);
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
            tomeDB[tomeCourant].pageInfos = mangaDB.tomes[i].ID;
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
    tomeDB[tomeCourant].mangaName[0] = 0;
    tomeDB[tomeCourant].pageInfos = VALEUR_FIN_STRUCTURE_CHAPITRE;
    tomeDB[0].nombreTomes = tomeCourant;

    if((tomeCourant == 1 && contexte != CONTEXTE_LECTURE) || (tomeCourant == 0 && contexte == CONTEXTE_LECTURE)) //Si il n'y a pas de chapitre
    {
        free(tomeDB);
        tomeDB = NULL;
    }
    return tomeDB;
}

