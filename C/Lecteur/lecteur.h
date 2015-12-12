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

//Macro pour libérer plus facilement la mémoire
#define FREE_CONTEXT() cleanMemory(dataReader, page, pageTexture, pageTooBigToLoad, prevPage, nextPage, UI_PageAccesDirect, infoTexture, controlBar, fontNormal, fontTiny)
#define REFRESH_SCREEN() refreshScreen(pageTexture, pageTooBigToLoad, positionSlide, positionPage, positionControlBar, controlBar, infoTexture, positionInfos, pageAccesDirect, UI_PageAccesDirect)

#ifdef __OBJC__

typedef struct
{
	uint nbPage;	//Contexte
    uint pageCourante;
    uint *pageCouranteDuChapitre;

    uint *pathNumber;	//Correspondance entre nomPage et path
    char **nomPages;
	char **path;		//Path de l'image, utilisé pour les volumes où les fichiers sont dans plusieurs répertoires
	uint * nameID;		//Utilisé par les PDF pour padder

	uint IDDisplayed;
    uint *chapitreTomeCPT; //Pour la crypto
	
	__unsafe_unretained NSMutableDictionary * PDFArrayForNames;	//Objective C PDF Cache, manually retained/freeed
	
} DATA_LECTURE;

bool configFileLoader(PROJECT_DATA project, bool isTome, uint IDRequested, DATA_LECTURE* dataReader);
void releaseDataReader(DATA_LECTURE *data);

#endif

/** lecteur_check_newElems.c **/

uint checkNewElementInRepo(PROJECT_DATA *project, bool isTome, uint CT);

/** lecteur_loading.c **/

bool reader_getNextReadableElement(PROJECT_DATA project, bool isTome, uint *currentPosIntoStructure);

bool changeChapter(PROJECT_DATA* project, bool isTome, uint *ptrToSelectedID, uint *posIntoStruc, bool goToNextChap);
bool changeChapterAllowed(PROJECT_DATA* project, bool isTome, uint posIntoStruc);

/**	lecteur_tool.c	**/
uint reader_getPosIntoContentIndex(PROJECT_DATA project, uint currentSelection, bool isTome);
bool reader_isLastElem(PROJECT_DATA project, bool isTome, uint currentSelection);


/*Mouvements*/
#define PAGE_MOVE 50

/*Limites buffers*/
#define LONGUEUR_NOM_PAGE 1024

/*Status*/
enum
{
	READER_ETAT_DEFAULT,
	READER_ETAT_PREVPAGE,
	READER_ETAT_NEXTPAGE,
	READER_ETAT_JUMP
};


#define READER_CHANGEPAGE_SUCCESS		0
#define READER_CHANGEPAGE_NEXTCHAP		1
#define READER_CHANGEPAGE_UPDATE_TOPBAR	2

/*Calibration*/
#define LECTEUR_DISTANCE_OPTIMALE_INFOS_ET_PAGEACCESDIRE 100
#define LECTEUR_DISTANCE_MINIMALE_INFOS_ET_PAGEACCESDIRE 5
#define DELAY_KEY_PRESSED_TO_START_PAGE_SLIDE 350
