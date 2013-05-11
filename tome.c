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
    char ligne[15+MAX_TOME_NAME_LENGTH+2*TOME_DESCRIPTION_LENGTH];
    pos = 0;
    while(ligneCourante < nombreMaxElems && pos < size)
    {
        for(i = 0; i < 14+MAX_TOME_NAME_LENGTH+2*TOME_DESCRIPTION_LENGTH && buffer[pos] != '\n' && buffer[pos] != '\r' && pos < size; ligne[i++] = buffer[pos++]); //Charge la première ligne
        ligne[i] = 0; //i <= longueur-1

        if(buffer[pos] != '\n') //Finis la ligne
            while(pos < size && buffer[++pos] != '\n');
        for(pos++; pos < size && (buffer[pos] == '\n' || buffer[pos] == '\r') ; pos++);

        for(i = 0; i < 10 && isNbr(ligne[i]); i++);
        if(ligne[i] == ' ' && i > 0 && i < 10) //Données saines
        {
            sscanfs(ligne, "%d %s %s %s", &lines[ligneCourante].ID, lines[ligneCourante].name, MAX_TOME_NAME_LENGTH, lines[ligneCourante].description1, TOME_DESCRIPTION_LENGTH, lines[ligneCourante].description2, TOME_DESCRIPTION_LENGTH);
            changeTo((char*) lines[ligneCourante].name, '_', ' ');
            changeTo((char*) lines[ligneCourante].description1, '_', ' ');
            changeTo((char*) lines[ligneCourante].description2, '_', ' ');
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
        free(mangaDB->tomes);

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
    int nbElem = 0;
    char temp[LONGUEUR_NOM_MANGA_MAX*2+50];
    FILE* config = NULL;

    snprintf(temp, LONGUEUR_NOM_MANGA_MAX*2+50, "manga/%s/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName, CONFIGFILETOME);
    if((config = fopen(temp, "r")) != NULL)
    {
        *dernierLu = VALEUR_FIN_STRUCTURE_CHAPITRE;
        fscanfs(config, "%d", dernierLu);
        fclose(config);
    }

    for(nbElem = 0; mangaDB->tomes[nbElem].ID != VALEUR_FIN_STRUCTURE_CHAPITRE && nbElem < mangaDB->nombreTomes; nbElem++)
    {
        if(!checkTomeReadable(*mangaDB, &mangaDB->tomes[nbElem]))
        {
            mangaDB->tomes[nbElem].ID = VALEUR_FIN_STRUCTURE_CHAPITRE;
            mangaDB->tomes[nbElem].name[0] = 0;
        }
    }

    qsort(mangaDB->tomes, nbElem, sizeof(META_TOME), sortTomes);
    for(; nbElem > 0 && mangaDB->tomes[nbElem-1].ID == VALEUR_FIN_STRUCTURE_CHAPITRE; nbElem--);
    mangaDB->nombreTomes = nbElem;
}

void getUpdatedTomeList(MANGAS_DATA *mangaDB)
{
    int i = VALEUR_FIN_STRUCTURE_CHAPITRE;
    refreshTomeList(mangaDB);
    checkTomeValable(mangaDB, &i);
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
    if(mangaDB->tomes == NULL)
        return VALEUR_FIN_STRUCTURE_CHAPITRE;
    return autoSelectionChapitreTome(mangaDB, 1, mangaDB->tomes[0].ID, mangaDB->tomes[mangaDB->nombreTomes-1].ID, contexte);
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
        snprintf(temp, 500, "manga/%s/%s/Tome_%d/%s", mangaDB.team->teamLong, mangaDB.mangaName, mangaDB.tomes[i].ID, CONFIGFILETOME);

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

