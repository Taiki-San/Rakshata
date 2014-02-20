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

//Copied from sqlite3.h
typedef struct sqlite3_stmt sqlite3_stmt;

typedef struct infos_Team
{
    char teamLong[LONGUEUR_NOM_MANGA_MAX];
    char teamCourt[LONGUEUR_COURT];
    char type[LONGUEUR_TYPE_TEAM];
    char URL_depot[LONGUEUR_URL];
    char site[LONGUEUR_SITE];
    bool openSite;
} TEAMS_DATA;

typedef struct tome_metadata
{
    int ID;
    unsigned char name[MAX_TOME_NAME_LENGTH];
    unsigned char description1[TOME_DESCRIPTION_LENGTH];
    unsigned char description2[TOME_DESCRIPTION_LENGTH];
}META_TOME;

/**********************************************************************************************
**
**	Structure optimisée, explication du contenu
**
**		Infos générales
**			-	mangaName				Nom complet du manga
**			-	mangaNameShort			Nom court du manga, un ID, pas présenté au user
**			-	team					Pointeur vers la structure de la team
**			-	status					État du projet (en cours, fini, suspendu, etc...)
**			-	genre					Type du projet (shonen & co)
**			-	pageInfos				Version de la page d'info
**			
**		Chapitres
**			-	firstChapter			# du premier chapitre
**			-	lastChapter				# du dernier chapitre
**			-	nombreChapitreSpeciaux	nombre de chapitres spéciaux
**			-	nombreChapitre			nombre de chapitres total
**			-	chapitres				tableau contenant tous les chapitres classés
**
**		Tomes
**			-	firstTome				# du premier chapitre
**			-	nombreTomes				nombre de tomes total
**			-	tomes					tableau contenant tous les tomes classés
**
**		Module DL
**			-	contentDownloadable		si il y a quelque chose qui n'est pas DL
**			-	favoris					est-ce que c'est un favoris
**
**		Cache
**
**			-	cacheDBID				ID utilisé pour accéder très vite à l'élement
**
**********************************************************************************************/

typedef struct dataMangas
{
	//Pointeurs, un bloc chacun (64b)
    TEAMS_DATA *team;
    int *chapitres;
    META_TOME *tomes;

	//Un bloc de 64b complet
	uint16_t status;
    uint16_t genre;
    uint16_t pageInfos;
    bool contentDownloadable;
    bool favoris;
	
	//Un bloc de 64b complet
	int firstChapter;
    int lastChapter;
    
	//Un bloc de 64b complet
    uint32_t nombreChapitreSpeciaux;
    int firstTome;
	
	//Un bloc de 64b complet chacun
	size_t nombreChapitre;
    size_t nombreTomes;
	
	//Padding, 50 + 10 o, 4 o libres
	char mangaName[LONGUEUR_NOM_MANGA_MAX];
    char mangaNameShort[LONGUEUR_COURT];
	
	//32b récupérés par le cache
	uint32_t cacheDBID;

} MANGAS_DATA;

typedef struct data_provided_to_engine
{
    /*Variable pour CONTEXTE_LECTURE*/
    MANGAS_DATA *data;
	
	/*Variable pour CONTEXTE_TOME*/
    char *description1; //Ligne de description 1
    char *description2; //Ligne de description 2

	//Position optimisée
	int ID;
	char stringToDisplay[MAX_LENGTH_TO_DISPLAY];
	
    /*Variable pour CONTEXTE_DL*/
    bool anythingToDownload;            //Si rien de nouveau à télécharger
    bool isFullySelected;               //Is element in cache

} DATA_ENGINE;

typedef struct preferences_to_engine
{
	uint nombreElementTotal;
	
	uint nombreChapitreDejaSelect;	//CONTEXTE_DL
	
    /*Variable pour CONTEXTE_CHAPITRE*/
	int chapitrePlusAncien;
	int chapitrePlusRecent;
	
	int currentTomeInfoDisplayed; //Précise si (et quel) tome est affiché
	
	/*Variable commune à CONTEXTE_CHAPITRE et CONTEXTE_TOME*/
    int IDDernierElemLu;
    char* website;
	bool switchAvailable;
	
} PREFS_ENGINE;

typedef struct
{
    char r;
    char g;
    char b;
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

