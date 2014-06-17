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
void refreshChaptersList(MANGAS_DATA *mangaDB);
bool checkChapterReadable(MANGAS_DATA mangaDB, int chapitre);
void checkChapitreValable(MANGAS_DATA *mangaDB, int *dernierLu);
void getUpdatedChapterList(MANGAS_DATA *mangaDB, bool getInstalled);
void internalDeleteChapitre(MANGAS_DATA mangaDB, int chapitreDelete, bool careAboutLinkedChapters);
bool isChapterShared(char *path, MANGAS_DATA data, int ID);

/**check.c**/
int checkEvnt();
void fillCheckEvntList(char list[NOMBRE_DE_FICHIER_A_CHECKER][LONGUEUR_NOMS_DATA]);
int checkFilesExistance(char list[NOMBRE_DE_FICHIER_A_CHECKER][LONGUEUR_NOMS_DATA], int results[NOMBRE_DE_FICHIER_A_CHECKER], bool* cantWrite);

void networkAndVersionTest();
int checkNetworkState(int state);
void checkHostNonModifie();
int checkInfopngUpdate(char teamLong[100], char nomProjet[100], int valeurAChecker);		///// NON UTILISEE
int checkNewManga(MANGAS_DATA mangasDB);													///// NON UTILISEE
int checkChapitreUnread(MANGAS_DATA mangasDB);												///// NON UTILISEE
int checkFirstLineButtonPressed(int button_selected[8]);
int checkSecondLineButtonPressed(int button_selected[8]);
int checkButtonPressed(int button_selected[8]);
int checkNameFileZip(char fileToTest[256]);
bool checkPathEscape(char *string, int length);

/**CTCommon.c**/
void getUpdatedCTList(MANGAS_DATA *mangaDB, bool isTome);
bool checkReadable(MANGAS_DATA mangaDB, bool isTome, void *data);
bool isAnythingToDownload(MANGAS_DATA mangaDB);
void internalDeleteCT(MANGAS_DATA mangaDB, bool isTome, int selection);
void releaseCTData(MANGAS_DATA data);

/**Donwload.c**/
void initializeDNSCache();
void releaseDNSCache();
int download_mem(char* adresse, char *POST, char *buffer_out, size_t buffer_length, int SSL_enabled);
int download_disk(char* adresse, char * POST, char *file_name, int SSL_enabled);

/**Error.c**/
void logR(char *error);
void connexionNeededToAllowANewComputer();
int libcurlErrorCode(CURLcode code);
int erreurReseau();
int showError();
int emptyLibrary();
void affichageRepoIconnue();
int UI_Alert(char* titre, char* contenu);
int errorEmptyCTList(int contexte, char trad[SIZE_TRAD_ID_19][TRAD_LENGTH]);
void memoryError(size_t size);

/**Favoris.c**/
bool checkIfFaved(MANGAS_DATA* mangaDB, char **favs);
void updateFavorites();
void getNewFavs();

/**Interface.c**/
void updateSectionMessage(char messageVersion[5]);
void checkSectionMessageUpdate();

/**Keys.c**/
int getMasterKey(unsigned char *input);
void generateRandomKey(unsigned char output[SHA256_DIGEST_LENGTH]);
int earlyInit(int argc, char *argv[]);
int get_compte_infos();
int logon();
int check_login(char adresseEmail[100]);
int getPassword(int curThread, char password[100]);
void passToLoginData(char passwordIn[100], char passwordSalted[SHA256_DIGEST_LENGTH*2+1]);
int checkPass(char adresseEmail[100], char password[100], int login);
int createSecurePasswordDB(unsigned char *key_sent);
int createNewMK(char password[50], unsigned char key[SHA256_DIGEST_LENGTH]);
void recoverPassFromServ(unsigned char key[SHA256_DIGEST_LENGTH]);

/**Lecteur.c**/
char ** loadChapterConfigDat(char* input, int *nombrePage);

/**Native.c**/
int mkdirR(char *path);
void getToWD(int *argc, char** argv);
void strend(char *recepter, size_t length, const char *sender);
char* mergeS(char* input1, char* input2);
void *ralloc(size_t length);
int charToInt(char *input);
int fscanfs(FILE* stream, const char *format, ...);
int sscanfs(char *char_input, const char *format, ...);
size_t ustrlen(const void *input);
void usstrcpy(void* output, size_t length, const void* input);
void ustrcpy(void* output, const void* input);

void removeFolder(char *path);
void ouvrirSite(char *URL);
int lancementExternalBinary(char cheminDAcces[100]);
int checkPID(int PID);
#define checkFileExist(filename) (access(filename, F_OK) != -1)
int checkDirExist(char *dirname);

/**Repo.c**/
int checkAjoutRepoParFichier(char *argv);
void addRepoByFile();
int ajoutRepo(bool ajoutParFichier);
int deleteRepo();
int defineTypeRepo(char *URL);
int confirmationRepo(char team[LONGUEUR_NOM_MANGA_MAX]);

/**Securite.c**/
int _AESEncrypt(void *_password, void *_path_input, void *_path_output, int cryptIntoMemory, int ECB); //Cachés dans crypto/rijndael.c
int _AESDecrypt(void *_password, void *_path_input, void *_path_output, int cryptIntoMemory, int ECB);
int AESEncrypt(void *_password, void *_path_input, void *_path_output, int cryptIntoMemory);
int AESDecrypt(void *_password, void *_path_input, void *_path_output, int cryptIntoMemory);
void decryptPage(void *password, rawData *buffer_int, rawData *buffer_out, size_t length);
void generateFingerPrint(unsigned char output[SHA256_DIGEST_LENGTH+1]);
void get_file_date(const char *filename, char *date);
void KSTriggered(TEAMS_DATA team);
void screenshotSpoted(char team[LONGUEUR_NOM_MANGA_MAX], char manga[LONGUEUR_NOM_MANGA_MAX], int chapitreChoisis);
IMG_DATA *loadSecurePage(char *pathRoot, char *pathPage, int numeroChapitre, int page);
void getPasswordArchive(char *fileName, char password[300]);
void loadKS(char killswitch_string[NUMBER_MAX_TEAM_KILLSWITCHE][2*SHA256_DIGEST_LENGTH+1]);
bool checkKS(TEAMS_DATA dataCheck, char dataKS[NUMBER_MAX_TEAM_KILLSWITCHE][2*SHA256_DIGEST_LENGTH+1]);
uint getRandom();

/**Settings.c**/
int affichageMenuGestion(); //Remplacer gestion par setting
char *loadPrefFile();
void addToPref(char flag, char *stringToAdd);
void removeFromPref(char flag);
void updatePrefs(char flag, char *stringToAdd);
int loadLangueProfile();
int loadEmailProfile();
char* loadLargePrefs(char flag);
void setFavorite(MANGAS_DATA* mangaDB);

/**Thread.c**/
void createNewThread(void *function, void *arg);
THREAD_TYPE createNewThreadRetValue(void *function, void *arg);
bool isThreadStillRunning(THREAD_TYPE hThread);
int getThreadCount();

/**Tome.c**/
void tomeDBParser(MANGAS_DATA* mangaDB, unsigned char* buffer, size_t size);
void escapeTomeLineElement(META_TOME *ligne);
void refreshTomeList(MANGAS_DATA *mangaDB);
void setTomeReadable(MANGAS_DATA mangaDB, int ID);
bool checkTomeReadable(MANGAS_DATA mangaDB, int ID);
bool parseTomeDetails(MANGAS_DATA mangaDB, int ID, CONTENT_TOME ** output);
void checkTomeValable(MANGAS_DATA *mangaDB, int *dernierLu);
void getUpdatedTomeList(MANGAS_DATA *mangaDB, bool getInstalled);
void copyTomeList(META_TOME * input, uint nombreTomes, META_TOME * output);
void freeTomeList(META_TOME * data, bool includeDetails);
void printTomeDatas(MANGAS_DATA mangaDB, char *bufferDL, int tome);
int extractNumFromConfigTome(char *input, int ID);
void internalDeleteTome(MANGAS_DATA mangaDB, int tomeDelete, bool careAboutLinkedChapters);

/**Translation.c**/
void loadTrad(char trad[][TRAD_LENGTH], int IDTrad);
void setNewLangue(int newLangue);
int tradAvailable();

/**Unzip.c**/
int miniunzip (void *inputData, char *outputZip, char *passwordZip, size_t size, size_t type);

/**Update.c**/
void checkUpdate();
void checkJustUpdated();

/**Utilitaires.c**/
#define crashTemp(string, length) memset(string, 0, length)
void changeTo(char *string, char toFind, char toPut);
int sortNumbers(const void *a, const void *b);
int sortMangas(const void *a, const void *b);
int sortTomes(const void *a, const void *b);
void versionRak(char *output);
uint getPosOfChar(char *input, char toFind, bool isEOFAcceptable);
int positionnementApresChar(char* input, char *stringToFind);
void checkIfCharToEscapeFromPOST(char * input, uint length, char * output);
void createPath(char *output);
IMG_DATA* readFile(char * path);
#define isHexa(caract) ((caract >= '0' && caract <= '9') || (caract >= 'a' && caract <= 'f') || (caract >= 'A' && caract <= 'F'))?1:0
#define isNbr(caract) isdigit(caract)
#define swapValues(a, b) { uint c; c = b; b = a; a = c; }
#define MIN(a, b) (a < b ? a : b)
void hexToDec(const char *input, unsigned char *output);
void hexToCGFloat(const char *input, uint32_t length, double *output);
void decToHex(const unsigned char *input, size_t length, char *output);
void MajToMin(char* input);
void minToMaj(char* input);
void unescapeLineReturn(char *input);
uint jumpLine(char * data);
void openOnlineHelp();
bool isDownloadValid(char *input);
int isJPEG(void *input);
int isPNG(void *input);
void addToRegistry(bool firstStart);
uint countSpaces(char * data);
uint32_t getFileSize(const char *filename);
uint64_t getFileSize64(const char * filename);
void mergeSort(int * tab, size_t length);
int removeDuplicate(int * array, int length);

