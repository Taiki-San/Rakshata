/*********************************************************************************************
**  __________         __           .__            __                  ____     ________    **
**  \______   \_____  |  | __  _____|  |__ _____ _/  |______    ___  _/_   |    \_____  \   **
**   |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \   \  \/ /|   |     /  ____/   **
**   |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   / |   |    /       \   **
**   |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /   \_/  |___| /\ \_______ \  **
**          \/      \/     \/     \/     \/     \/          \/               \/         \/  **
**                                                                                          **
**    Licence propriétaire, code source confidentiel, distribution formellement interdite   **
**                                                                                          **
*********************************************************************************************/

#define HAUTEUR_FENETRE_DL 470

/*Specs Font*/
#define MDL_SIZE_FONT_USED POLICE_MOYEN
#define MDL_LARGEUR_FONT 36             //On doit l'utiliser dans une fonction n'écrivant pas de texte, il faut donc utiliser une constante pour des raisons de cohérence

/*Micro-engine*/
#define MDL_NOMBRE_ELEMENT_COLONNE 6
#define MDL_NOMBRE_COLONNE 2
#define MDL_HAUTEUR_DEBUT_CATALOGUE 110
#define MDL_BORDURE_CATALOGUE 30
#define MDL_ESPACE_INTERCOLONNE (LARGEUR/2 - MDL_BORDURE_CATALOGUE)
#define MDL_INTERLIGNE 50
#define MDL_ICON_SIZE 40
#define MDL_ICON_POS 350

/*Header*/
#define HAUTEUR_TEXTE_TELECHARGEMENT 15
#define HAUTEUR_TEXTE_INSTALLATION 55

/*Codes*/
#define MDL_CODE_INTERNAL_ERROR -3
#define MDL_CODE_ERROR_INSTALL -2
#define MDL_CODE_ERROR_DL -1
#define MDL_CODE_DEFAULT 0
#define MDL_CODE_DL 1
#define MDL_CODE_DL_OVER 2
#define MDL_CODE_INSTALL 3
#define MDL_CODE_INSTALL_OVER 4

/*Icones*/
#define MDL_ICON_ERROR "data/icon/e.png"
#define MDL_ICON_DL "data/icon/dl.png"
#define MDL_ICON_INSTALL "data/icon/i.png"
#define MDL_ICON_WAIT "data/icon/w.png"
#define MDL_ICON_OVER "data/icon/o.png"

/*Divers*/
#define HAUTEUR_POURCENTAGE WINDOW_SIZE_H_DL - 50

typedef struct data_loaded_from_download_list
{
	int chapitre;
	bool subFolder;
    int partOfTome; //Si VALEUR_FIN_STRUCTURE, alors chapitre indé, sinon, tome dont c'est l'ID
    MANGAS_DATA* datas;
} DATA_LOADED;

typedef struct download_data_struct
{
    char *URL;
    char *buf;
    size_t length;
    size_t current_pos;
} TMP_DL;

typedef struct data_pour_installation
{
    void *downloadedData;
    bool subFolder;
    int chapitre;
    int tome;
    bool isLastElemOfTome;
    size_t length;
    MANGAS_DATA *mangaDB;
} DATA_INSTALL;

typedef struct main_data_module_DL
{
    int* currentState;
    DATA_LOADED* todoList;
    void* buf;
    size_t length;
} DATA_MOD_DL;

typedef struct argument_to_MDL_handler
{
    bool isTomeAndLastElem;
    int *currentState;
    char ***historiqueTeam;
    DATA_LOADED* todoList;
} MDL_HANDLER_ARG;

extern int WINDOW_SIZE_H_DL;
extern int WINDOW_SIZE_W_DL;
extern int INSTALL_DONE;
extern int CURRENT_TOKEN;

/**Download.c**/
int download_UI(TMP_DL *output);

/**ModuleDL2.c**/
void mainMDL();
void MDLLauncher();
void mainDLProcessing(DATA_LOADED *** todoList);
void MDLStartHandler(int posElement, DATA_LOADED ** todoList, char ***historiqueTeam);
void MDLHandleProcess(MDL_HANDLER_ARG* inputVolatile);
void MDLTelechargement(DATA_MOD_DL* input);
void MDLInstallation(MDL_HANDLER_ARG input, DATA_MOD_DL data);
int MDLDrawUI(DATA_LOADED** todoList, char trad[SIZE_TRAD_ID_22][TRAD_LENGTH]);
void MDLUpdateIcons(bool ignoreCache);
#define MDLDispDownloadHeader(a) MDLDispHeader(0, a)
#define MDLDispInstallHeader(a) MDLDispHeader(1, a)
void MDLDispHeader(bool isInstall, DATA_LOADED *todoList);
bool MDLDispError(char trad[SIZE_TRAD_ID_22][TRAD_LENGTH]);
bool MDLEventsHandling(DATA_LOADED **todoList, int nombreElementDrawn);
void MDLParseFile(DATA_LOADED **todoList, int **status, int nombreTotal, bool errorPrinted);

/**ModuleDL2_tool.c**/
char* MDL_craftDownloadURL(DATA_LOADED data);
char* internalCraftBaseURL(TEAMS_DATA teamData, int* length);
#define MDL_loadDataFromImport(a, b) MDL_updateDownloadList(a, b, NULL, NULL)
DATA_LOADED ** MDL_updateDownloadList(MANGAS_DATA* mangaDB, int* nombreMangaTotal, int **status, DATA_LOADED ** oldDownloadList);
DATA_LOADED** getTomeDetails(DATA_LOADED tomeDatas, int *outLength);
int sortMangasToDownload(const void *a, const void *b);
bool checkIfWebsiteAlreadyOpened(TEAMS_DATA teamToCheck, char ***historiqueTeam);
bool checkChapterAlreadyInstalled(DATA_LOADED dataToCheck);
void grabInfoPNG(MANGAS_DATA mangaToCheck);
SDL_Texture *getIconTexture(SDL_Renderer *rendererVar, int status);
void updateWindowSizeDL(int w, int h);

/**Module2_event.h**/
bool MDLEventsHandling(DATA_LOADED **todoList, int nbElemDrawn);
bool MDLisClicOnAValidX(int x, bool twoColumns);
bool MDLisClicOnAValidY(int y, int nombreElement);
void MDLDealWithClicsOnIcons(DATA_LOADED *todoList, int ligne);

/**Native.c**/
THREAD_TYPE createNewThreadRetValue(void *function, void *arg);
