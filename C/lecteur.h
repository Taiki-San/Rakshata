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

typedef struct data_lecture_tome
{
    uint nombrePageTotale;
    uint pageCourante;
    uint *pageCouranteDuChapitre;

    int *pathNumber; //Correspondance entre nomPage et path
    char **nomPages;
    char **path;

    int IDDisplayed;
    int *chapitreTomeCPT; //Pour la crypto
} DATA_LECTURE;

/** lecteur_check_newElems.c **/

uint checkNewElementInRepo(PROJECT_DATA *projectDB, bool isTome, int CT);

/** lecteur_loading.c **/

bool reader_getNextReadableElement(PROJECT_DATA projectDB, bool isTome, uint *currentPosIntoStructure);
bool configFileLoader(PROJECT_DATA projectDB, bool isTome, int chapitre_tome, DATA_LECTURE* dataReader);
char ** loadChapterConfigDat(char* input, int *nombrePage);
void releaseDataReader(DATA_LECTURE *data);

bool changeChapter(PROJECT_DATA* projectDB, bool isTome, int *ptrToSelectedID, uint *posIntoStruc, bool goToNextChap);
bool changeChapterAllowed(PROJECT_DATA* projectDB, bool isTome, int posIntoStruc);

/**	lecteur_tool.c	**/
int reader_getPosIntoContentIndex(PROJECT_DATA projectDB, int currentSelection, bool isTome);
bool reader_isLastElem(PROJECT_DATA projectDB, bool isTome, int currentSelection);


/*Mouvements*/
#define PAGE_MOVE 50
#define DEPLACEMENT_BIG positionSlide.h - BORDURE_CONTROLE_LECTEUR
#define DEPLACEMENT_SOURIS 7
#define DEPLACEMENT_LATERAL_PAGE 5
#define DEPLACEMENT_HORIZONTAL_PAGE 5
#define TOLERANCE_CLIC_PAGE 10

/*Limites buffers*/
#define LONGUEUR_NOM_PAGE LENGTH_PROJECT_NAME*2+300

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
