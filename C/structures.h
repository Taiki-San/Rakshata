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
    char teamLong[LONGUEUR_NOM_MANGA_MAX];
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
	int *chapitresFull;
	int *chapitresInstalled;
	META_TOME *tomesFull;
	META_TOME *tomesInstalled;

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
	size_t nombreChapitreInstalled;
	size_t nombreTomes;
	size_t nombreTomesInstalled;
	
	//Padding, 50 + 10 o, 4 o libres
	char mangaName[LONGUEUR_NOM_MANGA_MAX];
    char mangaNameShort[LONGUEUR_COURT];
	
	//32b récupérés par le cache
	uint32_t cacheDBID;

} MANGAS_DATA;

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

