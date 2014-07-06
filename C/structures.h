/*********************************************************************************************
**	__________         __           .__            __                 ________   _______   	**
**	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
**	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
**	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
**	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
**	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
**                                                                                          **
**		Source code and assets are property of Taiki, distribution is stricly forbidden		**
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
    char teamLong[LENGTH_PROJECT_NAME];
    char teamCourt[LONGUEUR_COURT];
    char type[LONGUEUR_TYPE_TEAM];
    char URLRepo[LONGUEUR_URL];
    char site[LONGUEUR_SITE];
    bool openSite;
} TEAMS_DATA;

typedef struct details_tome_data{
	int ID;
	bool isNative;			//Chapitre indé
}CONTENT_TOME;

typedef struct tome_metadata
{
	CONTENT_TOME * details;
    int ID;
	int readingID;
	wchar_t readingName[MAX_TOME_NAME_LENGTH];
    wchar_t description[TOME_DESCRIPTION_LENGTH];
}META_TOME;

/**********************************************************************************************
**
**	Structure optimisée, explication du contenu
**
**		Infos générales
**			-	projectID				ID unique dans la team pour le projet
**			-	projectName				Nom complet du projet
**			-	description				Description du projet
**			-	team					Pointeur vers la structure de la team
**			-	status					État du projet (en cours, fini, suspendu, etc...)
**			-	type					Type du projet (comics/BD/manga/webcomics)
**			-	category				Catégorie du projet
**			-	japaneseOrder			Sens de lecture
**
**		Page d'info
**			- URLLarge					URL vers une version de l'image de projet pour le tab CT
**			- hashLarge					CRC32 de la version courante
**			- URLSmall					URL vers une version de l'image de projet pour le D&D
**			- hashSmall					CRC32 de la version courante
**
**		Chapitres
**			-	nombreChapitre			nombre de chapitres total
**			-	chapitresFull			tableau contenant tous les chapitres classés
**			-	nombreChapitreInstalled	nombre de chapitres installés
**			-	chapitresFullInstalled	tableau contenant tous les chapitres installés classés
**
**		Tomes
**			-	nombreTomes				nombre de chapitres total
**			-	tomesFull				tableau contenant tous les chapitres classés
**			-	nombreTomesInstalled	nombre de chapitres installés
**			-	tomesFullInstalled		tableau contenant tous les chapitres installés classés
**
**		Module DL
**			-	favoris					est-ce que c'est un favoris
**
**		Cache
**			-	cacheDBID				ID utilisé pour accéder très vite à l'élement
**
**********************************************************************************************/

typedef struct dataProject
{
	//Pointeurs, un bloc chacun (64b)
	TEAMS_DATA *team;
	int *chapitresFull;
	int *chapitresInstalled;
	META_TOME *tomesFull;
	META_TOME *tomesInstalled;
	
	//Un bloc de 64b complet chacun
	size_t nombreChapitre;
	size_t nombreChapitreInstalled;
	size_t nombreTomes;
	size_t nombreTomesInstalled;
	
	wchar_t description[LENGTH_DESCRIPTION];
	
	//2 x ((51 + 1) x 32b) = 52 x 64b
	wchar_t projectName[LENGTH_PROJECT_NAME];
	bool favoris;
	wchar_t authorName[LENGTH_AUTHORS];
	bool japaneseOrder;
	
	//Un bloc de 64b complet
	uint8_t status;
	uint8_t type;
	uint16_t category;
	
	//Un bloc de 64b
	uint32_t projectID;
	uint32_t cacheDBID;
	
} PROJECT_DATA;

typedef struct dataProjectWithExtra
{
	//Pointeurs, un bloc chacun (64b)
	TEAMS_DATA *team;
	int *chapitresFull;
	int *chapitresInstalled;
	META_TOME *tomesFull;
	META_TOME *tomesInstalled;
	
	//Un bloc de 64b complet chacun
	size_t nombreChapitre;
	size_t nombreChapitreInstalled;
	size_t nombreTomes;
	size_t nombreTomesInstalled;
	
	wchar_t description[LENGTH_DESCRIPTION];
	
	//2 x ((51 + 1) x 32b) = 52 x 64b
	wchar_t projectName[LENGTH_PROJECT_NAME];
	bool contentDownloadable;
	wchar_t authorName[LENGTH_AUTHORS];
	bool favoris;
	
	//Un bloc de 64b complet
	uint8_t status;
	uint8_t type;
	uint16_t category;
	bool japaneseOrder;
	
	//Un bloc de 64b
	uint32_t projectID;
	uint32_t cacheDBID;
	
	//2 x 64b
	char hashLarge[LENGTH_HASH];
	char hashSmall[LENGTH_HASH];
	
	//2 x 256o
	char URLLarge[LENGTH_URL];
	char URLSmall[LENGTH_URL];
	
} PROJECT_DATA_EXTRA;

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

