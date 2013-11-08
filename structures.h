/*********************************************************************************************
**	__________         __           .__            __                 ________   _______   	**
**	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
**	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
**	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
**	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
**	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
**                                                                                          **
**    Licence propriÈtaire, code source confidentiel, distribution formellement interdite   **
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
    //Infos gÈnÈrales
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

    //Module DL
    bool contentDownloadable;

    int favoris;
} MANGAS_DATA;

typedef struct data_provided_to_engine
{
    char stringToDisplay[MAX_LENGTH_TO_DISPLAY];
    int nombreElementTotal; //Nombre total, seulement dans le premier Èlement de la structure
    int ID;

    /*Variable pour CONTEXTE_LECTURE*/
    MANGAS_DATA *data;

    /*Variable pour CONTEXTE_DL*/
    int nombreChapitreDejaSelect;
    bool anythingToDownload;            //Si rien de nouveau ‡ tÈlÈcharger
    bool isFullySelected;               //Is element in cache

    /*Variable commune ‡ CONTEXTE_CHAPITRE et CONTEXTE_TOME*/
    bool switchAvailable;
    int IDDernierElemLu;
    char* website;

    /*Variable pour CONTEXTE_CHAPITRE*/
	int chapitrePlusAncien;
	int chapitrePlusRecent;

    /*Variable pour CONTEXTE_TOME*/
    int currentTomeInfoDisplayed; //PrÈcise si (et quel) tome est affichÈ
    char *description1; //Ligne de description 1
    char *description2; //Ligne de description 2
} DATA_ENGINE;

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
