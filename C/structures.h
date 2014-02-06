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

#ifndef byte
	typedef unsigned char byte;
#endif
typedef byte rawData;

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
    size_t nombreChapitre;
    int *chapitres;

    //Tomes
    int firstTome;
    size_t nombreTomes;
    META_TOME *tomes;

    //Module DL
    bool contentDownloadable;

    int favoris;
} MANGAS_DATA;

typedef struct data_provided_to_engine
{
    char stringToDisplay[MAX_LENGTH_TO_DISPLAY];
    int ID;

    /*Variable pour CONTEXTE_LECTURE*/
    MANGAS_DATA *data;

    /*Variable pour CONTEXTE_DL*/
    bool anythingToDownload;            //Si rien de nouveau à télécharger
    bool isFullySelected;               //Is element in cache

    /*Variable pour CONTEXTE_TOME*/
    char *description1; //Ligne de description 1
    char *description2; //Ligne de description 2
	
} DATA_ENGINE;

typedef struct preferences_to_engine
{
	int nombreElementTotal;
	
	int nombreChapitreDejaSelect;	//CONTEXTE_DL
	
    /*Variable commune à CONTEXTE_CHAPITRE et CONTEXTE_TOME*/
    int IDDernierElemLu;
    char* website;
	bool switchAvailable;
	
	/*Variable pour CONTEXTE_CHAPITRE*/
	int chapitrePlusAncien;
	int chapitrePlusRecent;
	
	int currentTomeInfoDisplayed; //Précise si (et quel) tome est affiché
	
} PREFS_ENGINE;

typedef struct
{
    int r;
    int g;
    int b;
} PALETTE_RGB;

typedef PALETTE_RGB Rak_Color;

typedef struct
{
    PALETTE_RGB fond;               //Background
    PALETTE_RGB police;             //Police normale
    PALETTE_RGB police_new;         //Police nouveau manga (rouge)
    PALETTE_RGB police_unread;      //Police non-lu (vert)
    PALETTE_RGB police_actif;       //Police pour menus actifs (rouge)
    PALETTE_RGB police_indispo;     //Police pour menus indisponibles (gris)
} PALETTE_GLOBALE;

typedef struct
{
	void* data;
	size_t length;
} IMG_DATA;

