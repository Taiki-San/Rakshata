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

#include "db.h"

//Macro pour libérer plus facilement la mémoire
#define FREE_CONTEXT() cleanMemory(dataReader, page, pageTexture, pageTooBigToLoad, prevPage, nextPage, UI_PageAccesDirect, infoTexture, controlBar, fontNormal, fontTiny)
#define REFRESH_SCREEN() refreshScreen(pageTexture, pageTooBigToLoad, positionSlide, positionPage, positionControlBar, controlBar, infoTexture, positionInfos, pageAccesDirect, UI_PageAccesDirect)

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

/** lecteur_event.c **/

void applyFullscreen(bool *var_fullscreen, bool *redrawScreen, bool *changementEtat);

/** lecteur_check_newElems.c **/

uint checkNewElementInRepo(MANGAS_DATA *mangaDB, bool isTome, int CT);
void addtoDownloadListFromReader(MANGAS_DATA mangaDB, int firstElem, bool isTome);

/** lecteur_loading.c **/

bool reader_getNextReadableElement(MANGAS_DATA mangaDB, bool isTome, uint *currentPosIntoStructure);
bool configFileLoader(MANGAS_DATA mangaDB, bool isTome, int chapitre_tome, DATA_LECTURE* dataReader);
char ** loadChapterConfigDat(char* input, int *nombrePage);
void releaseDataReader(DATA_LECTURE *data);

int changementDePage(MANGAS_DATA *mangaDB, DATA_LECTURE* dataReader, bool goToNextPage, int *changementPage, bool isTome, int *chapitreChoisis, int currentPosIntoStructure);
bool changeChapter(MANGAS_DATA* mangaDB, bool isTome, int *ptrToSelectedID, uint *posIntoStruc, bool goToNextChap);
bool changeChapterAllowed(MANGAS_DATA* mangaDB, bool isTome, int posIntoStruc);

/**	lecteur_tool.c	**/
int reader_getPosIntoContentIndex(MANGAS_DATA mangaDB, int currentSelection, bool isTome);
bool reader_isLastElem(MANGAS_DATA mangaDB, bool isTome, int currentSelection);


/*Mouvements*/
#define PAGE_MOVE 50
#define DEPLACEMENT_BIG positionSlide.h - BORDURE_CONTROLE_LECTEUR
#define DEPLACEMENT_SOURIS 7
#define DEPLACEMENT_LATERAL_PAGE 5
#define DEPLACEMENT_HORIZONTAL_PAGE 5
#define TOLERANCE_CLIC_PAGE 10

/*Limites buffers*/
#define LONGUEUR_NOM_PAGE LONGUEUR_NOM_MANGA_MAX*2+300

/*Tailles*/
#define BORDURE_LAT_LECTURE 20
#define BORDURE_HOR_LECTURE 40
#define BORDURE_INFERIEURE 25
#define BORDURE_BUTTON_H 5
#define BORDURE_BUTTON_W 10
#define BORDURE_CONTROLE_LECTEUR 100
#define LARGEUR_CONTROLE_LECTEUR 800
#define MINIICONE_H 42
#define MINIICONE_W 42
#define BIGICONE_H 90
#define BIGICONE_W 90

/*Positions*/
#define LARGE_BUTTONS_LECTEUR_PC 30
#define LARGE_BUTTONS_LECTEUR_PP 185
#define LARGE_BUTTONS_LECTEUR_NP 510
#define LARGE_BUTTONS_LECTEUR_NC 665

/*Return values*/
#define CLIC_SUR_BANDEAU_NONE 0
#define CLIC_SUR_BANDEAU_PREV_CHAPTER 1
#define CLIC_SUR_BANDEAU_PREV_PAGE 2
#define CLIC_SUR_BANDEAU_NEXT_PAGE 3
#define CLIC_SUR_BANDEAU_NEXT_CHAPTER 4
#define CLIC_SUR_BANDEAU_FAVORITE 5
#define CLIC_SUR_BANDEAU_FULLSCREEN 6
#define CLIC_SUR_BANDEAU_DELETE 7
#define CLIC_SUR_BANDEAU_MAINMENU 8

/*Status*/
#define READER_ETAT_DEFAULT 0
#define READER_ETAT_PREVPAGE 1
#define READER_ETAT_NEXTPAGE 2


#define READER_CHANGEPAGE_SUCCESS		0
#define READER_CHANGEPAGE_NEXTCHAP		1
#define READER_CHANGEPAGE_UPDATE_TOPBAR	2

/*Calibration*/
#define LARGEUR_MAX_LECTEUR (RESOLUTION[0] * 80 / 100)
#define LECTEUR_DISTANCE_OPTIMALE_INFOS_ET_PAGEACCESDIRE 100
#define LECTEUR_DISTANCE_MINIMALE_INFOS_ET_PAGEACCESDIRE 5
#define DELAY_KEY_PRESSED_TO_START_PAGE_SLIDE 350
