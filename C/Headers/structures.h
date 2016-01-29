/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at https://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

typedef byte rawData;

typedef wchar_t charType;

//Copied from sqlite3.h
typedef struct sqlite3_stmt sqlite3_stmt;

typedef struct
{
	uint ID;
	bool isPrivate;			//Chapitre indé

} CONTENT_TOME;

typedef struct
{
	CONTENT_TOME * details;
	
	uint ID;
	int readingID;
	
	uint price;
	uint lengthDetails;
	
	charType readingName[MAX_TOME_NAME_LENGTH + 1];
	charType description[TOME_DESCRIPTION_LENGTH];

} META_TOME;

typedef struct
{
	void* data;
	size_t length;
	
} IMG_DATA;

typedef struct
{
	char string[1024];
} ICON_PATH;

//Gestion du tag

typedef struct
{
	charType * name;
	uint ID;
	
} TAG_VERBOSE;

typedef struct
{
	uint ID;
	
} TAG;

typedef struct
{
	uint ID;
	uint rootID;
	
	charType * name;

} CATEGORY_VERBOSE;

typedef struct
{
	uint ID;

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
 **			-	nbDescriptions		Taille des tableaux sus-nommés
 **			-	subRepo					Tableau contenant les données des sous-repos
 **			-	nbSubrepo			Taille du tableau des sous-repos
 **			-	subRepoAreExtra			Défini la structure de subRepo (REPO_DATA/REPO_DATA_EXTRA)
 **			-	trusted					Indique si la repo est de confiance
 **
 **********************************************************************************************/


typedef struct
{
	//Bloc sans padding
	
	char website[REPO_WEBSITE_LENGTH];
	charType name[REPO_NAME_LENGTH];
	char URL[REPO_URL_LENGTH];
	
	//64b
	char language[REPO_LANGUAGE_LENGTH];
	
	//32b
	uint repoID;
	uint parentRepoID;

	byte type;
	bool isMature;
	bool active;

} REPO_DATA;

typedef struct
{
	REPO_DATA * data;
	
	char URLImage[REPO_URL_LENGTH];
	char URLImageRetina[REPO_URL_LENGTH];
	
	char hashImage[LENGTH_CRC];
	char hashImageRetina[LENGTH_CRC];
	
	bool haveRetina;
	
} REPO_DATA_EXTRA;

typedef struct
{
	charType name[REPO_NAME_LENGTH];
	
	byte type;
	char URL[REPO_URL_LENGTH];
	
	charType ** descriptions;
	char ** langueDescriptions;
	uint nbDescriptions;
	
	REPO_DATA * subRepo;
	uint nbSubrepo;
	
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
**			-	rightToLeft				Sens de lecture
**			-	isPaid					Contenus payants disponibles
**			-	haveDRM					Les DRM sont-elles activées
**			-	locale					Le projet vient-il d'une source externe et a été importé par un .rak
**
**		Page d'info
**			- hashLarge					CRC32 de la version courante
**			- hashSmall					CRC32 de la version courante
**
**		Chapitres
**			-	nbChapter				nombre de chapitre total
**			-	chaptersFull			tableau contenant tous les chapitres classés
**			-	chaptersPrix			tableau contenant le prix des chapitres
**			-	nbChapterInstalled		nombre de chapitres installés
**			-	chaptersFullInstalled	tableau contenant tous les chapitres installés classés
**
**		Tomes
**			-	nbVolumes				nombre de tome total
**			-	volumesFull				tableau contenant tous les tomez classés
**			-	nbVolumesInstalled		nombre de tome installés
**			-	volumesFullInstalled	tableau contenant tous les tome installés classés
**
**		Module DL
**			-	favoris					est-ce que c'est un favoris
**
**		Cache
**			-	cacheDBID				ID utilisé pour accéder très vite à l'élement
**
**********************************************************************************************/

typedef struct
{
	//Pointeurs, un bloc chacun (64b)
	REPO_DATA *repo;
	uint *chaptersFull;
	uint *chaptersPrix;
	uint *chaptersInstalled;
	META_TOME *volumesFull;
	META_TOME *volumesInstalled;
	
	//Un bloc de 64b complet chacun
	size_t nbChapter;
	size_t nbChapterInstalled;
	size_t nbVolumes;
	size_t nbVolumesInstalled;
	
	charType description[LENGTH_DESCRIPTION];
	
	//2 x ((51 + 1) x 32b) = 52 x 64b
	charType projectName[LENGTH_PROJECT_NAME];
	bool favoris;
	charType authorName[LENGTH_AUTHORS];
	bool rightToLeft;
	
	//Un bloc de 64b
	uint32_t projectID;
	uint32_t cacheDBID;

	//Deux blocs de 64b complets
	uint32_t category;
	uint32_t mainTag;
	TAG *tags;

	//64 bit
	uint32_t nbTags;
	uint8_t status;
	bool isPaid;
	bool locale;
	bool haveDRM;

	bool isInitialized;
	
} PROJECT_DATA;

typedef struct
{
	PROJECT_DATA project;

	uint *chaptersRemote;
	uint *chaptersLocal;

	uint nbChapterRemote;
	uint nbChapterLocal;

	META_TOME * tomeRemote;
	META_TOME * tomeLocal;

	uint nbVolumesRemote;
	uint nbVolumesLocal;

} PROJECT_DATA_PARSED;

typedef struct
{
	PROJECT_DATA_PARSED data;

	//2 x 64b
	bool haveImages[NB_IMAGES];
	char hashesImages[NB_IMAGES][LENGTH_CRC];
	char * URLImages[NB_IMAGES];
	
} PROJECT_DATA_EXTRA;

