/*********************************************************************************************
**	__________         __           .__            __                 ________   _______   	**
**	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
**	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
**	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
**	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
**	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
**                                                                                          **
**    Licence propriétaire, code source confidentiel, distribution formellement interdite   **
**                                                                                          **
*********************************************************************************************/

#ifdef _WIN32
    #define mutexAskUIThreadWIP mutexUI
#else
    extern pthread_mutex_t mutexAskUIThreadWIP;
#endif

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
#define MDL_CODE_UNPAID				-4
#define MDL_CODE_INTERNAL_ERROR		-3
#define MDL_CODE_ERROR_INSTALL		-2
#define MDL_CODE_ERROR_DL			-1
#define MDL_CODE_DEFAULT			0
#define MDL_CODE_WAITING_PAY		1
#define MDL_CODE_WAITING_LOGIN		2
#define MDL_CODE_DL					3
#define MDL_CODE_DL_OVER			4
#define MDL_CODE_INSTALL			5
#define MDL_CODE_INSTALL_OVER		6
#define MDL_CODE_FIRST_ERROR MDL_CODE_ERROR_DL

#define MDLP_CODE_ERROR 0  //En cas de données insuffisante, calloc met directement error aux manquants
#define MDLP_CODE_PAID 1
#define MDLP_CODE_TO_PAY 2
#define MDLP_HIGHEST_CODE 2

/*Icones*/
#define MDL_ICON_ERROR_DEFAULT "data/icon/e.png"
#define MDL_ICON_ERROR_GENERAL "data/icon/eg.png"
#define MDL_ICON_ERROR_DOWNLOAD "data/icon/ed.png"
#define MDL_ICON_ERROR_INSTALL "data/icon/ei.png"
#define MDL_ICON_DL "data/icon/dl.png"
#define MDL_ICON_INSTALL "data/icon/i.png"
#define MDL_ICON_WAIT "data/icon/w.png"
#define MDL_ICON_OVER "data/icon/o.png"
#define MDL_ICON_TO_PAY "data/icon/p.png"

/*Divers*/
#define HAUTEUR_POURCENTAGE WINDOW_SIZE_H_DL - 50

typedef struct data_loaded_from_download_list
{
	int chapitre;
	int *listChapitreOfTome;
	unsigned char *tomeName;
	bool subFolder;
    int partOfTome; //Si VALEUR_FIN_STRUCTURE, alors chapitre indé, sinon, tome dont c'est l'ID
    MANGAS_DATA* datas;
} DATA_LOADED;

typedef struct data_sent_to_pay_thread
{
    int prix;
    int sizeStatusLocal;
    unsigned int factureID;
    int ** statusLocal;
    bool somethingToPay;
} DATA_PAY;

typedef struct download_data_obfuscated
{
	char *data;
	char *mask;
} DATA_DL_OBFS;

typedef struct download_data_struct
{
    char *URL;
    void *buf;
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

/**Download.c**/
int download_UI(TMP_DL *output);

/**ModuleDL2.c**/
void mainMDL();
void MDLLauncher();
void mainDLProcessing(DATA_LOADED *** todoList);
void MDLStartHandler(int posElement, DATA_LOADED ** todoList, char ***historiqueTeam);
void MDLHandleProcess(MDL_HANDLER_ARG* inputVolatile);
bool MDLTelechargement(DATA_MOD_DL* input);
bool MDLInstallation(void *buf, size_t sizeBuf, MANGAS_DATA *mangaDB, int chapitre, int tome, bool subFolder, bool haveToPutTomeAsReadable);
int MDLDrawUI(DATA_LOADED** todoList, char trad[SIZE_TRAD_ID_22][TRAD_LENGTH]);
void MDLUpdateIcons(bool ignoreCache);
#define MDLDispDownloadHeader(a) MDLDispHeader(0, a)
#define MDLDispInstallHeader(a) MDLDispHeader(1, a)
void MDLDispHeader(bool isInstall, DATA_LOADED *todoList);
bool MDLDispError(char trad[SIZE_TRAD_ID_22][TRAD_LENGTH]);
void MDLParseFile(DATA_LOADED **todoList, int **status, int nombreTotal, bool errorPrinted);

/**ModuleDL2_tool.c**/
char* MDL_craftDownloadURL(DATA_LOADED data);
char* internalCraftBaseURL(TEAMS_DATA teamData, int* length);
#define MDL_loadDataFromImport(a, b) MDL_updateDownloadList(a, b, NULL)
DATA_LOADED ** MDL_updateDownloadList(MANGAS_DATA* mangaDB, int* nombreMangaTotal, DATA_LOADED ** oldDownloadList);
DATA_LOADED ** MDLGetRidOfDuplicates(DATA_LOADED ** currentList, int beginingNewData, int *nombreMangaTotal);
bool MDLCheckDuplicate(DATA_LOADED *struc1, DATA_LOADED *struc2);
DATA_LOADED** getTomeDetails(DATA_LOADED tomeDatas, int *outLength);
int sortMangasToDownload(const void *a, const void *b);

bool checkIfWebsiteAlreadyOpened(TEAMS_DATA teamToCheck, char ***historiqueTeam);
bool checkChapterAlreadyInstalled(DATA_LOADED dataToCheck);
void grabInfoPNG(MANGAS_DATA mangaToCheck);
void getIconPath(int status, char *path, uint length);

/**Module2_event.h**/
bool MDLEventsHandling(DATA_LOADED ***todoList, int nbElemDrawn);
bool MDLisClicOnAValidX(int x, bool twoColumns);
int MDLisClicOnAValidY(int y, int nombreElement);
void MDLDealWithClicsOnIcons(DATA_LOADED ***todoList, int ligne, bool isFirstNonDL, bool isLastNonDL);

/**Module2_paid.h**/
void MDLPHandle(DATA_LOADED ** data, int length);
char *MDLPCraftPOSTRequest(DATA_LOADED ** data, int *index);
void MDLPHandlePayProcedure(DATA_PAY * arg);
bool waitForGetPaid(unsigned int factureID);
void MDLPDestroyCache(unsigned int factureID);

bool MDLPCheckAnythingPayable(DATA_LOADED ** data, int length);
int * MDLPGeneratePaidIndex(DATA_LOADED ** data, int length);
bool MDLPCheckIfPaid(unsigned int factureID);

void MDLPDispCheckingIfPaid();
void MDLPDispAskToPay(int prix);
int MDLPWaitEvent();
void MDLPEraseDispChecking();

/**Module2_UI.c**/
void startMDLUIThread();
void MDLTUIQuit();

/**Native.c**/
THREAD_TYPE createNewThreadRetValue(void *function, void *arg);
