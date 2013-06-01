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

typedef struct infos_Team
{
    char teamLong[LONGUEUR_NOM_MANGA_MAX];
    char teamCourt[LONGUEUR_COURT];
    char type[LONGUEUR_TYPE_TEAM];
    char URL_depot[LONGUEUR_URL];
    char site[LONGUEUR_SITE];
    int openSite;
} TEAMS_DATA;

typedef struct tome_metadata
{
    int ID;
    unsigned char name[MAX_TOME_NAME_LENGTH];
    unsigned char description1[TOME_DESCRIPTION_LENGTH];
    unsigned char description2[TOME_DESCRIPTION_LENGTH];
}META_TOME;

typedef struct dataMangas
{
    //Infos générales
    char mangaName[LONGUEUR_NOM_MANGA_MAX];
    char mangaNameShort[LONGUEUR_COURT];
    TEAMS_DATA *team;

    int status;
    int genre;
    int pageInfos;

    //Chapitres
    int firstChapter;
    int lastChapter;
    int nombreChapitreSpeciaux;
    int nombreChapitre;
    int *chapitres;

    //Tomes
    int firstTome;
    int nombreTomes;
    META_TOME *tomes;

    int favoris;
} MANGAS_DATA;

typedef struct data_lecture_tome
{
    int nombrePageTotale;
    int pageCourante;
    int *pageCouranteDuChapitre;

    int *pathNumber; //Correspondance entre nomPage et path
    char **nomPages;
    char **path;

    int IDDisplayed;
    int *chapitreTomeCPT; //Pour la crypto
} DATA_LECTURE;

typedef struct data_loaded_from_download_list
{
	int chapitre;
	bool subFolder;
    int partOfTome; //Si VALEUR_FIN_STRUCTURE, alors chapitre indé, sinon, tome dont c'est l'ID
    MANGAS_DATA* datas;
} DATA_LOADED;

typedef struct download_data_struct
{
    char *URL;
    char *buf;
    size_t length;
    size_t current_pos;
} TMP_DL;

typedef struct data_pour_installation
{
    void *downloadedData;
    bool subFolder;
    int chapitre;
    int tome;
    bool isLastElemOfTome;
    size_t length;
    MANGAS_DATA *mangaDB;
} DATA_INSTALL;

typedef struct data_provided_to_engine
{
    char stringToDisplay[MAX_LENGTH_TO_DISPLAY];
    int nombreElementTotal; //Nombre total, seulement dans le premier élement de la structure
    int ID;

    /*Variable pour CONTEXTE_LECTURE*/
    MANGAS_DATA *data;

    /*Variable pour CONTEXTE_DL*/
    int nombreChapitreDejaSelect;

    /*Variable commune à CONTEXTE_CHAPITRE et CONTEXTE_TOME*/
    bool switchAvailable;
    int IDDernierElemLu;
    char* website;

    /*Variable pour CONTEXTE_CHAPITRE*/
	int chapitrePlusAncien;
	int chapitrePlusRecent;

    /*Variable pour CONTEXTE_TOME*/
    int currentTomeInfoDisplayed; //Précise si (et quel) tome est affiché
    char *description1; //Ligne de description 1
    char *description2; //Ligne de description 2
} DATA_ENGINE;

typedef struct data_thread_check_new_CT
{
    MANGAS_DATA mangaDB;
    bool isTome;
    int CT;
} DATA_CK_LECTEUR;

typedef struct
{
    int r;
    int g;
    int b;
} PALETTE_RGB;

typedef struct
{
    PALETTE_RGB fond;               //Background
    PALETTE_RGB police;             //Police normale
    PALETTE_RGB police_new;         //Police nouveau manga (rouge)
    PALETTE_RGB police_unread;      //Police non-lu (vert)
    PALETTE_RGB police_actif;       //Police pour menus actifs (rouge)
    PALETTE_RGB police_indispo;     //Police pour menus indisponibles (gris)
} PALETTE_GLOBALE;

typedef unsigned char uint8_t;

#ifndef _WIN32
    struct thread_info {        /* Used as argument to thread_start() */
        pthread_t thread_id;        /* ID returned by pthread_create() */
        int       thread_num;       /* Application-defined thread # */
        char     *argv_string;      /* From command-line argument */
    };

#endif
