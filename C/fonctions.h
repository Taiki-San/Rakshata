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

/**Chapitre.c**/
void refreshChaptersList(PROJECT_DATA *projectDB);
bool checkChapterReadable(PROJECT_DATA projectDB, int chapitre);
void checkChapitreValable(PROJECT_DATA *projectDB, int *dernierLu);
void getUpdatedChapterList(PROJECT_DATA *projectDB, bool getInstalled);
void internalDeleteChapitre(PROJECT_DATA projectDB, int chapitreDelete, bool careAboutLinkedChapters);
bool isChapterShared(char *path, PROJECT_DATA data, int ID);

/**check.c**/
void networkAndVersionTest();
bool checkNetworkState(int state);

/**CTCommon.c**/
void getUpdatedCTList(PROJECT_DATA *projectDB, bool isTome);
bool checkReadable(PROJECT_DATA projectDB, bool isTome, int data);
void internalDeleteCT(PROJECT_DATA projectDB, bool isTome, int selection);
void * buildInstalledList(void * fullData, uint nbFull, uint * installed, uint nbInstalled, bool isTome);
void releaseCTData(PROJECT_DATA data);

/**Download.c**/
void initializeDNSCache();
void releaseDNSCache();
int download_mem(char* adresse, char *POST, char **buffer_out, size_t * buffer_length, bool SSL_enabled);
int download_disk(char* adresse, char * POST, char *file_name, bool SSL_enabled);

/**Error.c**/
void logR(char *error);
int libcurlErrorCode(CURLcode code);
void memoryError(size_t size);

/**Favoris.c**/
bool checkIfFaved(PROJECT_DATA* projectDB, char **favs);
bool setFavorite(PROJECT_DATA* projectDB);
void updateFavorites();
bool checkFavoriteUpdate(PROJECT_DATA project, PROJECT_DATA * projectInPipeline, bool * isTomePipeline, int * elementInPipeline, bool checkOnly);
void getNewFavs();

/**JSONParser.m**/
PROJECT_DATA_EXTRA * parseRemoteData(REPO_DATA* repo, char * remoteDataRaw, uint * nbElem);
PROJECT_DATA * parseLocalData(REPO_DATA ** repo, uint nbRepo, unsigned char * remoteDataRaw, uint *nbElem);
char * reversedParseData(PROJECT_DATA * data, uint nbElem, REPO_DATA ** repo, uint nbRepo, size_t * sizeOutput);
void moveProjectExtraToStandard(const PROJECT_DATA_EXTRA input, PROJECT_DATA * output);

void convertTagMask(uint64_t input, uint32_t * category, uint64_t * tagMask, uint32_t * mainTag);

ROOT_REPO_DATA * parseRemoteRepo(char * parseDataRaw);
ROOT_REPO_DATA ** parseLocalRepo(char * parseDataRaw, uint * nbElem);
char * linearizeRepoData(ROOT_REPO_DATA ** root, uint rootLength, size_t * sizeOutput);

/**Keys.c**/
byte getMasterKey(unsigned char *input);
void generateRandomKey(unsigned char output[SHA256_DIGEST_LENGTH]);
void updateEmail(const char * email);
void deleteEmail();
void addPassToCache(const char * hashedPassword);
bool getPassFromCache(char pass[2 * SHA256_DIGEST_LENGTH + 1]);
bool validateEmail(const char* adresseEmail);
byte checkLogin(const char adresseEmail[100]);
int login(const char * adresseEmail, const char * password, bool createAccount);
void saltPassword(char passwordSalted[2*SHA256_DIGEST_LENGTH+1]);
byte createSecurePasswordDB(unsigned char *key_sent);
bool createNewMK(char password[50], unsigned char key[SHA256_DIGEST_LENGTH]);
bool recoverPassFromServ(unsigned char key[SHA256_DIGEST_LENGTH]);

/**Lecteur.c**/
char ** loadChapterConfigDat(char* input, uint *nombrePage);

/**Native.c**/
int mkdirR(char *path);
void strend(char *recepter, size_t length, const char *sender);
int fscanfs(FILE* stream, const char *format, ...);
int sscanfs(char *char_input, const char *format, ...);
size_t ustrlen(const void *input);
size_t wstrlen(const charType * input);
int wstrcmp(const charType * a, const charType * b);
charType * wstrdup(const charType * input);
void wstrncpy(charType * output, size_t length, const charType * input);
void usstrcpy(void* output, size_t length, const void* input);

void removeFolder(char *path);
void ouvrirSite(const char *URL);
#define checkFileExist(filename) (access(filename, F_OK) != -1)
bool checkDirExist(char *dirname);

/**Repo.c**/
bool getRepoData(byte type, char * repoURL, char ** output, size_t * sizeOutput);
char * getPathForRepo(REPO_DATA * repo);
char * getPathForRootRepo(ROOT_REPO_DATA * repo);
byte defineTypeRepo(char *URL);

/**Securite.c**/
void decryptPage(void *password, rawData *buffer_int, rawData *buffer_out, size_t length);
void generateFingerPrint(unsigned char output[SHA256_DIGEST_LENGTH+1]);
void getFileDate(const char *filename, char *date, void* internalData);
IMG_DATA *loadSecurePage(char *pathRoot, char *pathPage, int numeroChapitre, uint page);
void loadKS(char killswitch_string[NUMBER_MAX_REPO_KILLSWITCHE][2*SHA256_DIGEST_LENGTH+1]);
bool checkKS(ROOT_REPO_DATA dataCheck, char dataKS[NUMBER_MAX_REPO_KILLSWITCHE][2*SHA256_DIGEST_LENGTH+1]);
void KSTriggered(REPO_DATA team);
uint getRandom();
bool checkSignature(const char * input, const char * signature);

/**Settings.c**/
char *loadPrefFile();
void addToPref(char* flag, char *stringToAdd);
void removeFromPref(char* flag);
void updatePrefs(char* flag, char *stringToAdd);
bool loadEmailProfile();
char* loadLargePrefs(char* flag);

/**Thread.c**/
void createNewThread(void *function, void *arg);
THREAD_TYPE createNewThreadRetValue(void *function, void *arg);
bool isThreadStillRunning(THREAD_TYPE hThread);

/**Tome.c**/
uint getPosForID(PROJECT_DATA data, bool installed, int ID);
void refreshTomeList(PROJECT_DATA *projectDB);
void setTomeReadable(PROJECT_DATA projectDB, int ID);
bool checkTomeReadable(PROJECT_DATA projectDB, int ID);
void checkTomeValable(PROJECT_DATA *project, int *dernierLu);
void getUpdatedTomeList(PROJECT_DATA *projectDB, bool getInstalled);
void copyTomeList(META_TOME * input, uint nombreTomes, META_TOME * output);
void freeTomeList(META_TOME * data, uint length, bool includeDetails);
void internalDeleteTome(PROJECT_DATA projectDB, int tomeDelete, bool careAboutLinkedChapters);

/**Unzip.c**/
bool miniunzip(void *inputData, char *outputZip, PROJECT_DATA project, size_t size, int preferenceSetting);

/**Utilitaires.c**/
#define crashTemp(string, length) memset(string, 0, length)
int sortNumbers(const void *a, const void *b);
int sortProjects(const void *a, const void *b);
int sortRepo(const void *a, const void *b);
int sortRootRepo(const void *a, const void *b);
bool areProjectsIdentical(PROJECT_DATA a, PROJECT_DATA b);
uint positionnementApresChar(char* input, char *stringToFind);
void checkIfCharToEscapeFromPOST(char * input, uint length, char * output);
void createPath(char *output);
IMG_DATA* readFile(char * path);
#define isHexa(caract) (((caract >= '0' && caract <= '9') || (caract >= 'a' && caract <= 'f') || (caract >= 'A' && caract <= 'F')) ? 1 : 0)
#define isNbr(caract) isdigit(caract)
#define MIN(a, b) (a < b ? a : b)
bool isDownloadValid(char *input);
int isJPEG(void *input);
int isPNG(void *input);
void addToRegistry(bool firstStart);
uint32_t getFileSize(const char *filename);
uint64_t getFileSize64(const char * filename);