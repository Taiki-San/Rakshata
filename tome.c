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
    for(pos = 0; pos < size && buffer[pos]; pos++)
    {
        if(buffer[pos] == '\n')
            nombreMaxElems++;
    }
    if(buffer[pos-1] != '\n' || buffer[pos] != '\r')
        nombreMaxElems++;
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
        if((ligne[i] == ' ' || !ligne[i]) && i > 0 && i < 10) //Données saines
        {
            sscanfs(ligne, "%d %s %s %s", &lines[ligneCourante].ID, lines[ligneCourante].name, MAX_TOME_NAME_LENGTH, lines[ligneCourante].description1, TOME_DESCRIPTION_LENGTH, lines[ligneCourante].description2, TOME_DESCRIPTION_LENGTH);
            escapeTomeLineElement(&lines[ligneCourante++]);
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

void escapeTomeLineElement(META_TOME *ligne)
{
    ///Si l'élement ne contient que _, je l'ignore
    if(ligne->name[0] == '_' && ligne->name[1] == 0)
        ligne->name[0] = 0;
    else
        changeTo((char*) ligne->name, '_', ' ');

    if(ligne->description1[0] == '_' && ligne->description1[1] == 0)
        ligne->description1[0] = 0;
    else
        changeTo((char*) ligne->description1, '_', ' ');

    if(ligne->description2[0] == '_' && ligne->description2[1] == 0)
        ligne->description2[0] = 0;
    else
        changeTo((char*) ligne->description2, '_', ' ');
}

void refreshTomeList(MANGAS_DATA *mangaDB)
{
    if(mangaDB->tomes != NULL)
        free(mangaDB->tomes);

    /*On commence par énumérer les chapitres spéciaux*/
    char temp[TAILLE_BUFFER];
    snprintf(temp, TAILLE_BUFFER, "manga/%s/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName, TOME_INDEX);
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
        {
            tomeDBParser(mangaDB, buffer, size);
            free(buffer);
        }
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
        if(!checkTomeReadable(*mangaDB, mangaDB->tomes[nbElem].ID))
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
    char temp[TAILLE_BUFFER], texteTrad[SIZE_TRAD_ID_19][TRAD_LENGTH];
    loadTrad(texteTrad, 19);

    if((i = autoSelectionChapitreTome(mangaDB, 1, contexte)) != VALEUR_FIN_STRUCTURE_CHAPITRE)
        return i;

    snprintf(temp, TAILLE_BUFFER, "manga/%s/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName, CONFIGFILE);
    if(checkFileExist(temp) || contexte == CONTEXTE_DL)
    {
        /*Initialize internal chapter list*/
        refreshTomeList(mangaDB);
        if(contexte == CONTEXTE_DL)
        {
            if(checkFileExist(temp))
                dernierLu = VALEUR_FIN_STRUCTURE_CHAPITRE; //Si un manga est déjà installé, on le met dans le sens décroissant
        }
        else
        {
            checkTomeValable(mangaDB, &dernierLu);
            if(mangaDB->nombreTomes == PALIER_MENU)
                return PALIER_MENU;
        }

        //Generate chapter list
        DATA_ENGINE *tomeDB = generateTomeList(*mangaDB, (dernierLu == VALEUR_FIN_STRUCTURE_CHAPITRE), contexte, texteTrad[14], texteTrad[1]);

        //Si liste vide
        if(tomeDB == NULL) //Erreur de mémoire ou liste vide
            return errorEmptyCTList(contexte, 1, texteTrad);

        displayTemplateTome(mangaDB, tomeDB[0], contexte, texteTrad);
        do
        {
            tomeChoisis = engineCore(tomeDB, CONTEXTE_TOME, tomeDB[0].nombreElementTotal > ENGINE_ELEMENT_PAR_PAGE ? BORDURE_SUP_SELEC_TOME_FULL : BORDURE_SUP_SELEC_TOME_PARTIAL, NULL);
        }while(tomeChoisis == ENGINE_RETVALUE_SWITCH);
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

void displayTemplateTome(MANGAS_DATA* mangaDB, DATA_ENGINE data, int contexte, char texteTrad[SIZE_TRAD_ID_19][TRAD_LENGTH])
{
    displayTemplateChapitreTome(mangaDB, contexte, 1, data, texteTrad);
}

DATA_ENGINE *generateTomeList(MANGAS_DATA mangaDB, bool ordreCroissant, int contexte, char* stringAll, char* stringGeneric)
{
    bool outCheck;
    int i = 0;
    char temp[500], stringGenericUsable[TRAD_LENGTH];

    //On capitalise la première lettre
    if(strlen(stringGeneric) >= TRAD_LENGTH)
        stringGeneric[TRAD_LENGTH-1] = 0;
    strcpy(stringGenericUsable, stringGeneric);
    if(stringGenericUsable[0] >= 'a' && stringGenericUsable[0] <= 'z')
        stringGenericUsable[0] += 'A'-'a';

    /*On prépare maintenant la structure*/
    DATA_ENGINE *tomeDB = calloc(mangaDB.nombreTomes+2, sizeof(DATA_ENGINE));

    /************************************************************
    ** Génére le noms des chapitre en vérifiant leur existance **
    **              Si on télécharge, on met tout              **
    ************************************************************/
    int tomeCourant = 0;

    if(contexte != CONTEXTE_LECTURE)
    {
        tomeDB[tomeCourant].ID = VALEUR_FIN_STRUCTURE_CHAPITRE;
        usstrcpy(tomeDB[tomeCourant].stringToDisplay, MAX_LENGTH_TO_DISPLAY, stringAll);
        tomeCourant++;
    }

    if(ordreCroissant)
        i = 0;
    else
        i = mangaDB.nombreTomes-1;
    while((i < mangaDB.nombreTomes && ordreCroissant) || (i >= 0 && !ordreCroissant))
    {
        snprintf(temp, 500, "manga/%s/%s/Tome_%d/%s", mangaDB.team->teamLong, mangaDB.mangaName, mangaDB.tomes[i].ID, CONFIGFILETOME);

        outCheck = checkFileExist(temp);
        if((outCheck && contexte != CONTEXTE_DL) || (!outCheck && contexte == CONTEXTE_DL))
        {
            if(mangaDB.tomes[i].name[0] != 0) //Si on a un nom custom
                usstrcpy(tomeDB[tomeCourant].stringToDisplay, MAX_LENGTH_TO_DISPLAY, mangaDB.tomes[i].name);
            else
                snprintf(tomeDB[tomeCourant].stringToDisplay, MAX_LENGTH_TO_DISPLAY, "%s %d", stringGenericUsable, mangaDB.tomes[i].ID);

            tomeDB[tomeCourant].ID = mangaDB.tomes[i].ID;
            tomeDB[tomeCourant].description1 = (char *) mangaDB.tomes[i].description1;
            tomeDB[tomeCourant].description2 = (char *) mangaDB.tomes[i].description2;
            tomeCourant++;
        }
        if(ordreCroissant)
            i++;
        else
            i--;
    }
    tomeDB[0].nombreElementTotal = tomeCourant;
    tomeDB[0].website = mangaDB.team->site;
    tomeDB[0].currentTomeInfoDisplayed = VALEUR_FIN_STRUCTURE_CHAPITRE;

    if((tomeCourant == 1 && contexte != CONTEXTE_LECTURE) || (tomeCourant == 0 && contexte == CONTEXTE_LECTURE)) //Si il n'y a pas de chapitre
    {
        free(tomeDB);
        tomeDB = NULL;
    }
    return tomeDB;
}

void printTomeDatas(MANGAS_DATA mangaDB, char *bufferDL, int tome)
{
    size_t length = strlen(mangaDB.team->teamLong) + strlen(mangaDB.mangaName) + 100;
    char *bufferPath = malloc(length);
    FILE* out = NULL;
    if(bufferPath != NULL)
    {
        //I create the path to the file
        snprintf(bufferPath, length, "manga/%s/%s/Tome_%d/%s.tmp", mangaDB.team->teamLong, mangaDB.mangaName, tome, CONFIGFILETOME);
        out = fopen(bufferPath, "w+");
        if(out == NULL)
        {
            createPath(bufferPath); //If I can't create the file, I try to create its path, then retry
            out = fopen(bufferPath, "w+");
            if(out == NULL)
                return;
        }
        if(fwrite(bufferDL, strlen(bufferDL), 1, out) != 1) //Write data then check if everything went fine
        {
            logR("Failed at write tome infos");
#ifdef DEV_VERSION
            logR(bufferDL);
#endif
        }
        fclose(out);
        free(bufferPath);
    }
}

int extractNumFromConfigTome(char *input, int ID)
{
    int output = VALEUR_FIN_STRUCTURE_CHAPITRE, posDebut = 0;
    char basePath[100];

    snprintf(basePath, 100, "Tome_%d/Chapitre_", ID);

    if(!strncmp(input, "Chapitre_", 9))
        posDebut = 9;
    else if(!strncmp(input, basePath, strlen(basePath)))
        posDebut = strlen(basePath);

    if(posDebut)
    {
        int i = sscanfs(&input[posDebut], "%d", &output);
        output *= 10;

        if(input[posDebut+i] == '.' && isNbr(input[posDebut+i+1]))
            output += (int) input[posDebut+i+1] - '0';
    }
    return output;
}

