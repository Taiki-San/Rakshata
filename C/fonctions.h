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

/**Affichage.c**/
void welcome();
void raffraichissmenent(bool forced);
void loadPalette();
bool areSameColors(Rak_Color a, Rak_Color b);

/**Chapitre.c**/
void refreshChaptersList(MANGAS_DATA *mangaDB);
void checkChapitreValable(MANGAS_DATA *mangaDB, int *dernierLu);
void getUpdatedChapterList(MANGAS_DATA *mangaDB);
int askForChapter(MANGAS_DATA *mangaDB, int mode);
void displayTemplateChapitre(MANGAS_DATA* mangaDB, PREFS_ENGINE data, int contexte, char texteTrad[SIZE_TRAD_ID_19][TRAD_LENGTH]);
DATA_ENGINE *generateChapterList(MANGAS_DATA *mangaDB, bool ordreCroissant, int contexte, char* stringAll, char* stringGeneric, PREFS_ENGINE * prefs);

/**check.c**/
int checkEvnt();
void fillCheckEvntList(char list[NOMBRE_DE_FICHIER_A_CHECKER][LONGUEUR_NOMS_DATA]);
int checkFilesExistance(char list[NOMBRE_DE_FICHIER_A_CHECKER][LONGUEUR_NOMS_DATA], int results[NOMBRE_DE_FICHIER_A_CHECKER], bool* cantWrite);

int checkLancementUpdate();
void networkAndVersionTest();
int checkNetworkState(int state);
void checkHostNonModifie();
bool checkRestore();
int checkRestoreAvailable();
int checkInfopngUpdate(char teamLong[100], char nomProjet[100], int valeurAChecker);
int isItNew(MANGAS_DATA mangasDB); //Remplacer par checkNewManga
int checkChapitreUnread(MANGAS_DATA mangasDB);
int checkChapterEncrypted(MANGAS_DATA mangasDB, int chapitreChoisis);
int checkFirstLineButtonPressed(int button_selected[8]);
int checkSecondLineButtonPressed(int button_selected[8]);
int checkButtonPressed(int button_selected[8]);
int checkNameFileZip(char fileToTest[256]);
int checkFileValide(FILE* file);
bool checkPathEscape(char *string, int length);
bool checkChapterReadable(MANGAS_DATA mangaDB, int chapitre);
bool checkTomeReadable(MANGAS_DATA mangaDB, int ID);
bool checkReadable(MANGAS_DATA mangaDB, bool isTome, void *data);

/**CTCommon.c**/
int autoSelectionChapitreTome(MANGAS_DATA *mangaDB, bool isTome, int contexte);
void displayTemplateChapitreTome(MANGAS_DATA* mangaDB, int contexte, int isTome, PREFS_ENGINE data, char texteTrad[SIZE_TRAD_ID_19][TRAD_LENGTH]);
void displayIconeChapOrTome(bool isTome);
int askForCT(MANGAS_DATA* mangaDB, bool *isTome, int contexte);
void getUpdatedCTList(MANGAS_DATA *mangaDB, bool isTome);
bool isAnythingToDownload(MANGAS_DATA *mangaDB);

/**Database.c**/
MANGAS_DATA* miseEnCache(int mode);
MANGAS_DATA* allocateDatabase(size_t length);
void freeMangaData(MANGAS_DATA* mangaDB, size_t length);
void updateDataBase(bool forced);
void resetUpdateDBCache();
int get_update_repo(char *buffer_repo, TEAMS_DATA* teams);
bool checkValidationRepo(char *bufferDL, int isPaid);
void update_repo();
int get_update_mangas(char *buffer_manga, TEAMS_DATA* teams);
void update_mangas();
int deleteManga();
int internalDeleteCT(MANGAS_DATA mangaDB, bool isTome, int selection);
int internalDeleteTome(MANGAS_DATA mangaDB, int tomeDelete);
int internalDeleteChapitre(MANGAS_DATA mangaDB, int chapitreDelete);
void setLastChapitreLu(MANGAS_DATA* mangasDB, bool isTome, int dernierChapitre);
int databaseVersion(char* mangaDB);

/**Donwload.c**/
void initializeDNSCache();
void releaseDNSCache();
int download_mem(char* adresse, char *POST, char *buffer_out, size_t buffer_length, int SSL_enabled);
int download_disk(char* adresse, char * POST, char *file_name, int SSL_enabled);
int checkDLInProgress();

/**Engine.c**/
int letterLimitationEnforced(int letter, char firstLetterOfTheManga);
int buttonLimitationEnforced(int button_selected[8], MANGAS_DATA mangaDB);

bool getUnlock();
void setUnlock(bool var);

void loadMultiPage(int nombreManga, int *pageSelection, int *pageTotale);

Rak_Color getEngineColor(PREFS_ENGINE prefs, DATA_ENGINE input, int contexte, Rak_Color couleurUnread, Rak_Color couleurNew, Rak_Color couleurNothingToRead, Rak_Color couleurTexte);

void generateChoicePanel(char trad[SIZE_TRAD_ID_11][TRAD_LENGTH], int enable[8]);

void button_available(PREFS_ENGINE prefs, DATA_ENGINE* input, int button[8]);
void engineDisplayDownloadButtons(int nombreChapitreDejaSelect, char localization[SIZE_TRAD_ID_11][TRAD_LENGTH]);
void engineDisplayCurrentTypedChapter(int choix, int virgule, int hauteurNum);

void engineDisplayTomeInfos(DATA_ENGINE input);
void engineEraseDisplayedTomeInfos(int curThread);

/**Error.c**/
void logR(char *error);
void connexionNeededToAllowANewComputer();
int libcurlErrorCode(CURLcode code);
int erreurReseau();
int showError();
int rienALire();
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
int lecteur(MANGAS_DATA *mangaDB, int *chapitreChoisis, bool isTome, bool *fullscreen);
char ** loadChapterConfigDat(char* input, int *nombrePage);

/**Mainfunctions.c**/
void mainRakshata();
int mainLecture();
int mainChoixDL();

/**ModuleDL2_tool.c**/
int ecritureDansImport(MANGAS_DATA * mangaDB, bool isTome, int chapitreChoisis);
void lancementModuleDL();

/**Native.c**/
FILE* fopenR(void *_path, char *right);
void removeR(char *path);
void renameR(char *initialName, char *newName);
int mkdirR(char *path);
void chdirR();
void resetOriginalCHDir(int *argc, char** argv);
void strend(char *recepter, size_t length, const char *sender);
char* mergeS(char* input1, char* input2);
void *ralloc(size_t length);
int charToInt(char *input);
void fscanfs(FILE* stream, const char *format, ...);
int sscanfs(char *char_input, const char *format, ...);
size_t ustrlen(void *input);
void usstrcpy(void* output, size_t length, const void* input);
void ustrcpy(void* output, const void* input);

void removeFolder(char *path);
#define ouvrirSiteTeam(structTeam) ouvrirSite(structTeam->site)
void getDirectX();
bool isDirectXLoaded();
void ouvrirSite(char *URL);
int lancementExternalBinary(char cheminDAcces[100]);
int checkPID(int PID);
#define checkFileExist(filename) (access(filename, F_OK) != -1)
int checkDirExist(char *dirname);

/**PBKDF2.c**/
int internal_pbkdf2(uint32_t prf_hlen, const uint8_t *input, size_t inputLength, const uint8_t *salt, size_t saltLength, uint32_t iteneration, size_t lengthOutput, uint8_t *output);
void pbkdf2(uint8_t input[], uint8_t salt[], uint8_t output[]);

/**Repo.c**/
int checkAjoutRepoParFichier(char *argv);
int ajoutRepo(bool ajoutParFichier);
int deleteRepo();
int defineTypeRepo(char *URL);
int confirmationRepo(char team[LONGUEUR_NOM_MANGA_MAX]);

/**Securite.c**/
int _AESEncrypt(void *_password, void *_path_input, void *_path_output, int cryptIntoMemory, int ECB); //Cachés dans crypto/rijndael.c
int _AESDecrypt(void *_password, void *_path_input, void *_path_output, int cryptIntoMemory, int ECB);
int AESEncrypt(void *_password, void *_path_input, void *_path_output, int cryptIntoMemory);
int AESDecrypt(void *_password, void *_path_input, void *_path_output, int cryptIntoMemory);
void decryptPage(void *_password, rawData *buffer_int, rawData *buffer_out, size_t length);
void generateFingerPrint(unsigned char output[SHA256_DIGEST_LENGTH+1]);
void get_file_date(const char *filename, char *date);
void killswitchTriggered(char teamLong[LONGUEUR_NOM_MANGA_MAX]);
void screenshotSpoted(char team[LONGUEUR_NOM_MANGA_MAX], char manga[LONGUEUR_NOM_MANGA_MAX], int chapitreChoisis);
IMG_DATA *IMG_LoadS(char *pathRoot, char *pathPage, int numeroChapitre, int page);
void getPasswordArchive(char *fileName, char password[300]);
void Load_KillSwitch(char killswitch_string[NUMBER_MAX_TEAM_KILLSWITCHE][2*SHA256_DIGEST_LENGTH+1]);
int checkKillSwitch(char killswitch_string[NUMBER_MAX_TEAM_KILLSWITCHE][2*SHA256_DIGEST_LENGTH+1], TEAMS_DATA team_to_check);

/**Selection.c**/
int controleurManga(MANGAS_DATA* mangaDB, int contexte, int nombreChapitre, bool *selectMangaDLRightClick);
int controleurChapTome(MANGAS_DATA* mangaDB, bool *isTome, int contexte);

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

/**SHA256.c**/
int sha256(unsigned char* input, void* output);
int sha256_legacy(char input[], char output[2*SHA256_DIGEST_LENGTH+1]);
void sha256_salted(const uint8_t *input, size_t inputLen, const uint8_t *salt, size_t saltlen, uint8_t *output);

/**Thread.c**/
void createNewThread(void *function, void *arg);
THREAD_TYPE createNewThreadRetValue(void *function, void *arg);
bool isThreadStillRunning(THREAD_TYPE hThread);
int getThreadCount();

/**Tome.c**/
void tomeDBParser(MANGAS_DATA* mangaDB, unsigned char* buffer, size_t size);
void escapeTomeLineElement(META_TOME *ligne);
void refreshTomeList(MANGAS_DATA *mangaDB);
void checkTomeValable(MANGAS_DATA *mangaDB, int *dernierLu);
void getUpdatedTomeList(MANGAS_DATA *mangaDB);
int askForTome(MANGAS_DATA* mangaDB, int contexte);
void displayTemplateTome(MANGAS_DATA* mangaDB, PREFS_ENGINE data, int contexte, char texteTrad[SIZE_TRAD_ID_19][TRAD_LENGTH]);
DATA_ENGINE *generateTomeList(MANGAS_DATA* mangaDB, bool ordreCroissant, int contexte, char* stringAll, char* stringGeneric, PREFS_ENGINE * prefs);
void printTomeDatas(MANGAS_DATA mangaDB, char *bufferDL, int tome);
int extractNumFromConfigTome(char *input, int ID);

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
int plusOuMoins(int compare1, int compare2, int tolerance);
int sortNumbers(const void *a, const void *b);
int sortMangas(const void *a, const void *b);
int sortTomes(const void *a, const void *b);
void versionRak(char *output);
int positionnementApres(FILE* stream, char *stringToFind);
int positionnementApresChar(char* input, char *stringToFind);
void checkIfCharToEscapeFromPOST(char * input, uint length, char * output);
void teamOfProject(char nomProjet[LONGUEUR_NOM_MANGA_MAX], char nomTeam[LONGUEUR_NOM_MANGA_MAX]);
void createPath(char *output);
IMG_DATA* readFile(char * path);
#define isHexa(caract) ((caract >= '0' && caract <= '9') || (caract >= 'a' && caract <= 'f') || (caract >= 'A' && caract <= 'F'))?1:0
#define isNbr(caract) (caract >= '0' && caract <= '9')
#define swapValues(a, b) do { a ^= b; b ^= a; a ^= b; } while(0)
#define MIN(a, b) (a < b ? a : b)
void hexToDec(const char *input, unsigned char *output);
void hexToCGFloat(const char *input, uint32_t length, double *output);
void decToHex(const unsigned char *input, size_t length, char *output);
void MajToMin(char* input);
void minToMaj(char* input);
void unescapeLineReturn(char *input);
void openOnlineHelp();
bool isDownloadValid(char *input);
int isJPEG(void *input);
int isPNG(void *input);
void addToRegistry(bool firstStart);
void mergeSort(int * tab, size_t length);
int removeDuplicate(int * array, int length);

