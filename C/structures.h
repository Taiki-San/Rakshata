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

typedef wchar_t charType;

//Copied from sqlite3.h
typedef struct sqlite3_stmt sqlite3_stmt;

typedef struct details_tome_data
{
	int ID;
	bool isNative;			//Chapitre indé

} CONTENT_TOME;

typedef struct tome_metadata
{
	CONTENT_TOME * details;
	
	int ID;
	int readingID;
	
	uint price;
	uint lengthDetails;
	
	charType readingName[MAX_TOME_NAME_LENGTH];
	charType description[TOME_DESCRIPTION_LENGTH];

} META_TOME;

typedef struct
{
	void* data;
	size_t length;
	
} IMG_DATA;

//Gestion du tag

typedef struct tag_verbose_structure
{
	charType * name;
	uint ID;
	
} TAG_VERBOSE;

typedef struct tag_structure
{
	uint ID;
	
} TAG;

typedef struct category_structure
{
	uint ID;
	uint rootID;
	
	charType * name;
	
	TAG tags[TAG_PAR_CAT];
	byte mainTag;
	
} CATEGORY;


/**********************************************************************************************
 **
 **		REPOSITORY STRUCTURES
 **
 **	Ce système utilise un total de trois structures:
 **		- ROOT_REPO_DATA: contient le parsage complet d'un fichier rakshata-repo-3
 **		- REPO_DATA_EXTRA: contient l'intégralité des données relatives à une sous-repo
 **		- REPO_DATA: contient les données relatives à une sous-repo requise au fonctionnement
 **
 **	Structure optimisée, explication du contenu
 **
 **		Infos générales
 **			-	repoID					ID identifiant la repo parmit les sous-repo d'un root
 **			-	parentRepoID			ID identifiant le parent parmis les root
 **			-	name					Nom du gestionnaire de la repo
 **			-	type					Type de repo (Payant, Dropbox, ...)
 **			-	URL						Adresse utilisée pour contacter le repo
 **			-	language				Langue principale de la repo
 **			-	isMature				Contenu pornographique présent dans la repo
 **
 **		Extras
 **			- URLImage					URL pour DL la version courante de l'image (150x150, 72ppi)
 **			- URLImageRetina			URL pour DL la version courante de l'image retina (300x300, 144ppi)
 **			- hashImage					MD5 de la version courante
 **
 **		Root additions
 **			-	descriptions			Tableau de descriptions localisés
 **			-	langueDescriptions		Tableau synchronisé donnant la localisation des descriptions
 **			-	nombreDescriptions		Taille des tableaux sus-nommés
 **			-	subRepo					Tableau contenant les données des sous-repos
 **			-	nombreSubrepo			Taille du tableau des sous-repos
 **			-	subRepoAreExtra			Défini la structure de subRepo (REPO_DATA/REPO_DATA_EXTRA)
 **			-	trusted					Indique si la repo est de confiance
 **
 **********************************************************************************************/


typedef struct repository_data
{
	//Bloc sans padding
	
	char website[REPO_WEBSITE_LENGTH];
	charType name[REPO_NAME_LENGTH];
	char URL[REPO_URL_LENGTH];
	
	//64b
	char language[REPO_LANGUAGE_LENGTH];
	
	byte type;
	bool isMature;
	bool active;
	
	//32b
	uint repoID;
	uint parentRepoID;
	
} REPO_DATA;

typedef struct repository_data_extra
{
	REPO_DATA * data;
	
	char URLImage[REPO_URL_LENGTH];
	char URLImageRetina[REPO_URL_LENGTH];
	
	char hashImage[LENGTH_HASH];
	char hashImageRetina[LENGTH_HASH];
	
	bool haveRetina;
	
} REPO_DATA_EXTRA;

typedef struct root_repository_data
{
	charType name[REPO_NAME_LENGTH];
	
	byte type;
	char URL[REPO_URL_LENGTH];
	
	charType ** descriptions;
	char ** langueDescriptions;
	uint nombreDescriptions;
	
	REPO_DATA * subRepo;
	uint nombreSubrepo;
	
	uint repoID;
	bool trusted;
	bool subRepoAreExtra;
	
} ROOT_REPO_DATA;

/**********************************************************************************************
**
**		PROJECT STRUCTURES
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
**			-	isPaid					Contenus payants disponibles
**			-	haveDRM					Les DRM sont-elles activées
**
**		Page d'info
**			- hashLarge					CRC32 de la version courante
**			- hashSmall					CRC32 de la version courante
**
**		Chapitres
**			-	nombreChapitre			nombre de chapitres total
**			-	chapitresFull			tableau contenant tous les chapitres classés
**			-	chapitresPrix			tableau contenant le prix des chapitres
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

/////////////////////////////////////////
/////////////////////////////////////////
////////						/////////
////////		WARNING			/////////
////////						/////////
////////	 The two structs	/////////
////////	have to be synced	/////////
////////						/////////
/////////////////////////////////////////
/////////////////////////////////////////

typedef struct dataProject
{
	//Pointeurs, un bloc chacun (64b)
	REPO_DATA *repo;
	int *chapitresFull;
	uint *chapitresPrix;
	int *chapitresInstalled;
	META_TOME *tomesFull;
	META_TOME *tomesInstalled;
	
	//Un bloc de 64b complet chacun
	size_t nombreChapitre;
	size_t nombreChapitreInstalled;
	size_t nombreTomes;
	size_t nombreTomesInstalled;
	
	charType description[LENGTH_DESCRIPTION];
	
	//2 x ((51 + 1) x 32b) = 52 x 64b
	charType projectName[LENGTH_PROJECT_NAME];
	bool favoris;
	charType authorName[LENGTH_AUTHORS];
	bool japaneseOrder;
	
	//Un bloc de 64b complet
	uint32_t type;
	uint32_t tag;
	
	//Un bloc de 64b
	uint32_t projectID;
	uint32_t cacheDBID;
	
	uint8_t status;
	bool isPaid;
	bool haveDRM;
	bool isInitialized;
	
} PROJECT_DATA;

//Type here how many time you forgot to update the second structure: 2

typedef struct dataProjectWithExtra
{
	//Pointeurs, un bloc chacun (64b)
	REPO_DATA *repo;
	int *chapitresFull;
	uint *chapitresPrix;
	int *chapitresInstalled;
	META_TOME *tomesFull;
	META_TOME *tomesInstalled;
	
	//Un bloc de 64b complet chacun
	size_t nombreChapitre;
	size_t nombreChapitreInstalled;
	size_t nombreTomes;
	size_t nombreTomesInstalled;
	
	charType description[LENGTH_DESCRIPTION];
	
	//2 x ((51 + 1) x 32b) = 52 x 64b
	charType projectName[LENGTH_PROJECT_NAME];
	bool favoris;
	charType authorName[LENGTH_AUTHORS];
	bool japaneseOrder;
	
	//Un bloc de 64b complet
	uint32_t type;
	uint32_t tag;
	
	//Un bloc de 64b
	uint32_t projectID;
	uint32_t cacheDBID;
	
	//64b
	uint8_t status;
	bool isPaid;
	bool haveDRM;
	bool isInitialized;

	//2 x 64b
	bool haveImages[NB_IMAGES];
	char hashesImages[NB_IMAGES][LENGTH_HASH];
	char * URLImages[NB_IMAGES];
	
} PROJECT_DATA_EXTRA;

