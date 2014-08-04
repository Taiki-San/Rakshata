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

//#define INSTALLING_CONSIDERED_AS_INSTALLED

/*Codes*/
#define MDL_CODE_UNPAID				(-5)
#define MDL_CODE_INTERNAL_ERROR		(-4)
#define MDL_CODE_ERROR_INSTALL		(-3)
#define MDL_CODE_ERROR_DL			(-2)
#define MDL_CODE_ABORTED			(-1)
#define MDL_CODE_DEFAULT			0
#define MDL_CODE_WAITING_PAY		1
#define MDL_CODE_WAITING_LOGIN		2
#define MDL_CODE_DL					3
#define MDL_CODE_DL_OVER			4
#define MDL_CODE_INSTALL			5
#define MDL_CODE_INSTALL_OVER		6

#define MDL_CODE_UNUSED				(-6)
#define MDL_CODE_FIRST_ERROR MDL_CODE_ERROR_DL

#define MDLP_CODE_ERROR '#'  //En cas de données insuffisante, calloc met directement error aux manquants
#define MDLP_CODE_PAID '0'
#define MDLP_CODE_TO_PAY '2'
#define MDLP_HIGHEST_CODE '2'

/*Divers*/

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
	RID_UPDATE_STATUS			= 1,
	RID_UPDATE_STATUS_REANIMATE	= 2,
	RID_UPDATE_INSTALL			= 3
};

enum downloadStatusCodes {
	DLSTATUS_SUSPENDED	= 0x0f,
	DLSTATUS_ABORT		= 0xf0
};

typedef struct
{
	int element;
	bool subFolder;
} DATA_LOADED_TOME_DETAILS;

typedef struct data_loaded_from_download_list
{
	void * rowViewResponsible;
	CURL * curlHandler;
	
    PROJECT_DATA* datas;
	wchar_t *tomeName;
	
	DATA_LOADED_TOME_DETAILS *listChapitreOfTome;	//Should be used to differentiate chapters from volumes
	
	uint nbElemList;
	int identifier;
	
	uint8_t downloadSuspended;	//Divised in two parts
} DATA_LOADED;

typedef struct intermediary_from_data_loaded_to_DL_thread
{
	void ** rowViewResponsible;
	CURL ** curlHandler;
	uint8_t * downloadSuspended;	//Divised in two parts
	
    PROJECT_DATA* datas;
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
	size_t bytesDownloaded;
	size_t totalExpectedSize;
	
	TMP_DL * outputContainer;
	CURL ** curlHandler;
	uint8_t *aborted;
	
	int errorCode;
	short retryAttempt;
} DL_DATA;

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
	uint ** IDToPosition;
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
	
	int8_t *** fullStatus;
	uint statusLength;
	
} MDL_HANDLER_ARG;

/**Download.c**/
int downloadChapter(TMP_DL *output, uint8_t *abortTransmiter, void ** rowViewResponsible, uint currentPos, uint nbElem, CURL ** curlHandler);

/**ModuleDL2.c**/
bool startMDL(char * state, PROJECT_DATA * cache, THREAD_TYPE * coreWorker, DATA_LOADED **** todoList, int8_t *** status, uint ** IDToPosition, uint * nbElemTotal, bool * quit, void * mainTab);
bool startWorker(THREAD_TYPE * coreWorker, DATA_LOADED **** todoList, int8_t *** status, uint ** IDToPosition, uint * nbElemTotal, bool * quit, void * mainTab);
void MDLCleanup(int nbElemTotal, int8_t ** status, DATA_LOADED *** todoList, PROJECT_DATA * cache);
char* MDLParseFile(DATA_LOADED **todoList, int8_t **status, uint* IDToPosition, uint nombreTotal);

/**ModuleDL2_tool.c**/
char* MDL_craftDownloadURL(PROXY_DATA_LOADED data);
char* internalCraftBaseURL(TEAMS_DATA teamData, uint* length);
DATA_LOADED ** MDLLoadDataFromState(PROJECT_DATA* mangaDB, uint* nombreMangaTotal, char * state);
DATA_LOADED ** MDLInjectElementIntoMainList(DATA_LOADED ** mainList, uint *mainListSize, int * currentPosition, DATA_LOADED ** newChunk);
DATA_LOADED * MDLCreateElement(PROJECT_DATA * data, bool isTome, int element);
char MDL_isAlreadyInstalled(PROJECT_DATA projectData, bool isSubpartOfTome, int IDChap, uint *posIndexTome);
void MDL_createSharedFile(PROJECT_DATA data, int chapitreID, uint tomeID);
bool MDLCheckDuplicate(DATA_LOADED *struc1, DATA_LOADED *struc2);
bool getTomeDetails(DATA_LOADED *tomeDatas);
int sortMangasToDownload(const void *a, const void *b);

bool checkIfWebsiteAlreadyOpened(TEAMS_DATA teamToCheck, char ***historiqueTeam);
bool MDLDownloadOver(bool reanimateOnly);
bool MDLStartNextInstallation();
void MDLQuit();
void MDLUpdateIcons(uint selfCode, void * UIInstance);
void updatePercentage(void * rowViewResponsible, float percentage, size_t speed);
bool MDLisThereCollision(PROJECT_DATA projectToTest, bool isTome, int element, DATA_LOADED ** list, int8_t ** status, uint nbElem);
void dataRequireLogin(DATA_LOADED ** data, int8_t ** status, uint * IDToPosition, uint length, void* mainTab);
void watcherForLoginRequest(MDL_MWORKER_ARG * arg);

/**ModuleDLMainWorker.m**/
void mainDLProcessing(MDL_MWORKER_ARG * arg);
void MDLSetThreadID(THREAD_TYPE *thread);
void MDLStartHandler(uint posElement, uint nbElemTotal, DATA_LOADED ** todoList, int8_t *** status, char ***historiqueTeam);

/**ModuleDLWorker.c**/
void MDLHandleProcess(MDL_HANDLER_ARG* inputVolatile);
bool MDLTelechargement(DATA_MOD_DL* input, uint currentPos, uint nbElem);
void MDLUpdateKillState(bool newState);
bool MDLInstallation(void *buf, size_t sizeBuf, PROJECT_DATA *mangaDB, int chapitre, int tome, bool subFolder, bool haveToPutTomeAsReadable);

/**Module2_paid.h**/
bool MDLPHandle(DATA_LOADED ** data, int8_t *** status, uint * IDToPosition, uint length);
char *MDLPCraftPOSTRequest(DATA_LOADED ** data, uint *index);
void MDLPHandlePayProcedure(DATA_PAY * arg);
bool waitToGetPaid(unsigned int factureID);
void MDLPDestroyCache(unsigned int factureID);

bool MDLPCheckAnythingPayable(DATA_LOADED ** data, int8_t ** status, uint * IDToPosition, uint length);
uint * MDLPGeneratePaidIndex(DATA_LOADED ** data, int8_t ** status, uint * IDToPosition, uint length);
bool MDLPCheckIfPaid(unsigned int factureID);