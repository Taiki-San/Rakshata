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

//#define INSTALLING_CONSIDERED_AS_INSTALLED

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
#define MDL_CODE_UNPAID				-5
#define MDL_CODE_INTERNAL_ERROR		-4
#define MDL_CODE_ERROR_INSTALL		-3
#define MDL_CODE_ERROR_DL			-2
#define MDL_CODE_ABORTED			-1
#define MDL_CODE_DEFAULT			0
#define MDL_CODE_WAITING_PAY		1
#define MDL_CODE_WAITING_LOGIN		2
#define MDL_CODE_DL					3
#define MDL_CODE_DL_OVER			4
#define MDL_CODE_INSTALL			5
#define MDL_CODE_INSTALL_OVER		6

#define MDL_CODE_UNUSED				-5
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

enum isInstalledRC {
	ERROR_CHECK				= -1,
	NOT_INSTALLED			= 0,
	ALTERNATIVE_INSTALLED	= 1,
#ifdef INSTALLING_CONSIDERED_AS_INSTALLED
	INSTALLING				= 2,
#endif
	ALREADY_INSTALLED		= 3
};

enum requestIDCodes {
	RID_DEFAULT					= 0,
	RID_UPDATE_STATUS			= 1
};

enum downloadStatusCodes {
	DLSTATUS_SUSPENDED	= 0x0f,
	DLSTATUS_ABORT		= 0xf0
};

typedef struct data_loaded_from_download_list
{
	void * rowViewResponsible;
	CURL * curlHandler;
	
    MANGAS_DATA* datas;
	int *listChapitreOfTome;
	unsigned char *tomeName;
	
	int chapitre;
    int partOfTome; //Si VALEUR_FIN_STRUCTURE, alors chapitre indé, sinon, tome dont c'est l'ID

	bool subFolder;
	uint8_t downloadSuspended;	//Divised in two parts
} DATA_LOADED;

typedef struct intermediary_from_data_loaded_to_DL_thread
{
	void ** rowViewResponsible;
	CURL ** curlHandler;
	uint8_t * downloadSuspended;	//Divised in two parts
	
    MANGAS_DATA* datas;
	int *listChapitreOfTome;
	unsigned char *tomeName;
	
	int chapitre;
    int partOfTome; //Si VALEUR_FIN_STRUCTURE, alors chapitre indé, sinon, tome dont c'est l'ID
	
	bool subFolder;
	
}PROXY_DATA_LOADED;

typedef struct data_sent_to_pay_thread
{
    int8_t ** statusLocal;
    int prix;
    int sizeStatusLocal;
    unsigned int factureID;
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

typedef struct {
	uint bytesDownloaded;
	uint totalExpectedSize;
	
	TMP_DL * outputContainer;
	CURL ** curlHandler;
	uint8_t *aborted;
	
	int errorCode;
} DL_DATA;

typedef struct data_pour_installation
{
    MANGAS_DATA *mangaDB;
    void *downloadedData;
    size_t length;

    int chapitre;
    int tome;
    bool subFolder;
    bool isLastElemOfTome;
} DATA_INSTALL;

typedef struct main_data_module_DL
{
    PROXY_DATA_LOADED* todoList;
    void* buf;
    size_t length;
} DATA_MOD_DL;

typedef struct argument_to_main_worker
{
	DATA_LOADED **** todoList;
	int8_t *** status;
	uint * nbElemTotal;
	
	bool * quit;
	void * mainTab;		//RakMDLController
	
} MDL_MWORKER_ARG;

typedef struct argument_to_MDL_handler
{
	int selfCode;
    int8_t *currentState;
    char ***historiqueTeam;
    
	DATA_LOADED* todoList;
    bool isTomeAndLastElem;
	
	int8_t *** fullStatus;
	uint statusLength;
	
} MDL_HANDLER_ARG;

extern int WINDOW_SIZE_H_DL;
extern int WINDOW_SIZE_W_DL;

/**Download.c**/
int downloadChapter(TMP_DL *output, uint8_t *abortTransmiter, void ** rowViewResponsible, CURL ** curlHandler);

/**ModuleDL2.c**/
bool startMDL(char *state, MANGAS_DATA * cache, THREAD_TYPE * coreWorker, DATA_LOADED **** todoList, int8_t *** status, int8_t *** statusCache, uint * nbElem, bool * quit, void * mainTab);
bool startWorker(MANGAS_DATA * cache, THREAD_TYPE * coreWorker, DATA_LOADED **** todoList, int8_t *** status, int8_t *** statusCache, uint * nbElemTotal, bool * quit, void * mainTab);
void MDLCleanup(int nbElemTotal, int8_t ** status, int8_t ** statusCache, DATA_LOADED *** todoList, MANGAS_DATA * cache);
char* MDLParseFile(DATA_LOADED **todoList, int8_t **status, int nombreTotal);

/**ModuleDL2_tool.c**/
char* MDL_craftDownloadURL(PROXY_DATA_LOADED data);
char* internalCraftBaseURL(TEAMS_DATA teamData, int* length);
#define MDL_loadDataFromImport(a, b) MDL_updateDownloadList(a, b, NULL)
DATA_LOADED ** MDL_updateDownloadList(MANGAS_DATA* mangaDB, uint* nombreMangaTotal, DATA_LOADED ** oldDownloadList);
DATA_LOADED ** MDLInjectElementIntoMainList(DATA_LOADED ** mainList, uint *mainListSize, int * currentPosition, DATA_LOADED ** newChunk, int chunckSize);
DATA_LOADED ** MDLCreateElement(MANGAS_DATA * data, bool isTome, int element, int * lengthCreated);
DATA_LOADED ** MDLGetRidOfDuplicates(DATA_LOADED ** currentList, int beginingNewData, uint *nombreMangaTotal);
char MDL_isAlreadyInstalled(MANGAS_DATA projectData, bool isSubpartOfTome, int IDChap, uint *posIndexTome);
void MDL_createSharedFile(MANGAS_DATA data, int chapitreID, uint tomeID);
bool MDLCheckDuplicate(DATA_LOADED *struc1, DATA_LOADED *struc2);
DATA_LOADED** getTomeDetails(DATA_LOADED tomeDatas, int *outLength);
int sortMangasToDownload(const void *a, const void *b);

bool checkIfWebsiteAlreadyOpened(TEAMS_DATA teamToCheck, char ***historiqueTeam);
void grabInfoPNG(MANGAS_DATA mangaToCheck);
void MDLDownloadOver(uint selfCode);
void MDLQuit();
void MDLUpdateIcons(uint selfCode, void * UIInstance);
void updatePercentage(void * rowViewResponsible, float percentage);
bool MDLisThereCollision(MANGAS_DATA projectToTest, bool isTome, int element, DATA_LOADED ** list, int8_t * status, uint nbElem);

/**ModuleDLMainWorker.m**/
void mainDLProcessing(MDL_MWORKER_ARG * arg);
void MDLSetThreadID(THREAD_TYPE *thread);
void MDLStartHandler(uint posElement, uint nbElemTotal, DATA_LOADED ** todoList, int8_t *** status, char ***historiqueTeam);

/**ModuleDLWorker.c**/
void MDLHandleProcess(MDL_HANDLER_ARG* inputVolatile);
bool MDLTelechargement(DATA_MOD_DL* input);
void MDLUpdateKillState(bool newState);
bool MDLInstallation(void *buf, size_t sizeBuf, MANGAS_DATA *mangaDB, int chapitre, int tome, bool subFolder, bool haveToPutTomeAsReadable);

/**Module2_event.h**/
bool MDLisClicOnAValidX(int x, bool twoColumns);
int MDLisClicOnAValidY(int y, int nombreElement);
void MDLDealWithClicsOnIcons(DATA_LOADED ***todoList, int ligne, bool isFirstNonDL, bool isLastNonDL);

/**Module2_paid.h**/
void MDLPHandle(DATA_LOADED ** data, int8_t *** status, int length);
char *MDLPCraftPOSTRequest(DATA_LOADED ** data, int *index);
void MDLPHandlePayProcedure(DATA_PAY * arg);
bool waitForGetPaid(unsigned int factureID);
void MDLPDestroyCache(unsigned int factureID);

bool MDLPCheckAnythingPayable(DATA_LOADED ** data, int8_t ** status, int length);
int * MDLPGeneratePaidIndex(DATA_LOADED ** data, int8_t ** status, int length);
bool MDLPCheckIfPaid(unsigned int factureID);

void MDLPDispAskToPay(int prix);
int MDLPWaitEvent();

/**Native.c**/
THREAD_TYPE createNewThreadRetValue(void *function, void *arg);
