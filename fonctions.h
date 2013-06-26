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

/**Affichage.c**/
void welcome();
void initialisationAffichage();
void raffraichissmenent(bool forced);
void affichageLancement();
void chargement(SDL_Renderer* rendererVar, int h, int w);
void loadPalette();
void loadIcon(SDL_Window *window_ptr);
SDL_Renderer* setupRendererSafe(SDL_Window *window_ptr);
SDL_Surface* createUIAlert(SDL_Surface* alertSurface, char texte[][TRAD_LENGTH], int numberLine);
SDL_Texture * TTF_Write(SDL_Renderer *render, TTF_Font *font, const char *text, SDL_Color fg);
void applyBackground(SDL_Renderer *renderVar, int x, int y, int w, int h);
int getWindowSize(int w1h2);
void updateWindowSize(int w, int h);
void getResolution();
void restartEcran();
void nameWindow(SDL_Window* windows, const int value);

/**Chapitre.c**/
void refreshChaptersList(MANGAS_DATA *mangaDB);
void checkChapitreValable(MANGAS_DATA *mangaDB, int *dernierLu);
void getUpdatedChapterList(MANGAS_DATA *mangaDB);
int askForChapter(MANGAS_DATA *mangaDB, int mode);
void displayTemplateChapitre(MANGAS_DATA* mangaDB, DATA_ENGINE data, int contexte, char texteTrad[SIZE_TRAD_ID_19][TRAD_LENGTH]);
DATA_ENGINE *generateChapterList(MANGAS_DATA mangaDB, bool ordreCroissant, int contexte, char* stringAll, char* stringGeneric);

/**check.c**/
int check_evt();
int checkLancementUpdate();
void networkAndVersionTest();
int checkNetworkState(int state);
void checkHostNonModifie();
int checkRestore();
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
void displayTemplateChapitreTome(MANGAS_DATA* mangaDB, int contexte, int isTome, DATA_ENGINE data, char texteTrad[SIZE_TRAD_ID_19][TRAD_LENGTH]);
void displayIconeChapOrTome(bool isTome);
int askForCT(MANGAS_DATA* mangaDB, bool *isTome, int contexte);
void getUpdatedCTList(MANGAS_DATA *mangaDB, bool isTome);

/**Database.c**/
MANGAS_DATA* miseEnCache(int mode);
MANGAS_DATA* allocateDatabase(size_t length);
void freeMangaData(MANGAS_DATA* mangaDB, size_t length);
void updateDataBase(bool forced);
int get_update_repo(char *buffer_repo, TEAMS_DATA* teams);
bool checkValidationRepo(char *bufferDL, int isPaid);
void update_repo();
int get_update_mangas(char *buffer_manga, TEAMS_DATA* teams);
void update_mangas();
int deleteManga();
int internalDeleteCT(MANGAS_DATA mangaDB, bool isTome, int selection);
int internalDeleteTome(MANGAS_DATA mangaDB, int tomeDelete);
int internalDeleteChapitre(MANGAS_DATA mangaDB, int chapitreDelete);
void lastChapitreLu(MANGAS_DATA* mangasDB, bool isTome, int dernierChapitre);
int databaseVersion(char* mangaDB);

/**Donwload.c**/
int download_mem(char* adresse, char *buffer_out, size_t buffer_length, int SSL_enabled);
int download_disk(char* adresse, char *file_name, int SSL_enabled);
int checkDLInProgress();

/**Engine.c**/
int displayMenu(char texte[][TRAD_LENGTH], int nombreElements, int hauteurBloc);
int engineCore(DATA_ENGINE* input, int contexte, int hauteurAffichage, bool *selectMangaDLRightClick);
int engineSelection(int contexte, DATA_ENGINE* input, int tailleTexte[ENGINE_NOMBRE_COLONNE][ENGINE_NOMBRE_LIGNE][2], int hauteurChapitre, int *outputType);
int engineAnalyseOutput(int contexte, int output, int outputType, int *elementChoisis, DATA_ENGINE *input, int elementParColonne[ENGINE_NOMBRE_COLONNE], int button_selected[8], int *pageCourante, int pageTotale, int *limitationLettre, bool modeLigne);

int letterLimitationEnforced(int letter, char firstLetterOfTheManga);
int buttonLimitationEnforced(int button_selected[8], MANGAS_DATA mangaDB);
bool engineCheckIfToDisplay(int contexte, DATA_ENGINE input, int limitationLettre, int button_selected[8]);

int engineDefineElementClicked(int x, int y, int tailleTexte[ENGINE_NOMBRE_COLONNE][ENGINE_NOMBRE_LIGNE][2], int hauteurDebut, int nombreMaxElem);

void loadMultiPage(int nombreManga, int *pageSelection, int *pageTotale);
void engineLoadCurrentPage(int nombreElement, int pageCourante, int out[3]);
void engineDisplayPageControls(char localization[SIZE_TRAD_ID_21][TRAD_LENGTH], int pageSelection, int pageTotale);

void displayBigMainMenuIcon();
SDL_Color getEngineColor(DATA_ENGINE input, DATA_ENGINE input0, int contexte, SDL_Color couleurUnread, SDL_Color couleurNew, SDL_Color couleurTexte);

void generateChoicePanel(char trad[SIZE_TRAD_ID_11][TRAD_LENGTH], int enable[8]);

void button_available(DATA_ENGINE* input, int button[8]);
void engineDisplayDownloadButtons(int nombreChapitreDejaSelect, char localization[SIZE_TRAD_ID_11][TRAD_LENGTH]);
void engineDisplayCurrentTypedChapter(int choix, int virgule, int hauteurNum);

void engineDisplayTomeInfos(DATA_ENGINE input);
void enfineEraseDisplayedTomeInfos();

/**Error.c**/
void logR(char *error);
void connexionNeededToAllowANewComputer();
int libcurlErrorCode(CURLcode code);
int erreurReseau();
int showError();
int rienALire();
int affichageRepoIconnue();
int UI_Alert(char* titre, char* contenu);
int errorEmptyCTList(int contexte, int isTome, char trad[SIZE_TRAD_ID_19][TRAD_LENGTH]);
void memoryError(size_t size);

/**Favoris.c**/
bool checkIfFaved(MANGAS_DATA* mangaDB, char **favs);
void updateFavorites();
void getNewFavs();

/**Interface.c**/
void updateSectionMessage(char messageVersion[5]);
void checkSectionMessageUpdate();

/**IO.c**/
int nombreEntree(char input);
int waitEnter(SDL_Renderer* rendererVar);
int waitClavier(SDL_Renderer *rendererVar, char *retour, int nombreMax, int showTyped, bool allowedToQuitWithEscape, int startFromX, int startFromY);
int haveInputFocus(SDL_Event *event, SDL_Window *windows);

/**Keys.c**/
int getMasterKey(unsigned char *input);
void generateKey(unsigned char output[SHA256_DIGEST_LENGTH]);
int earlyInit(int argc, char *argv[]);
int get_compte_infos();
int logon();
int check_login(char adresseEmail[100]);
int getPassword(char password[100], int dlUI, int salt);
void passToLoginData(char password[100]);
int checkPass(char adresseEmail[100], char password[100], int login);
int createSecurePasswordDB(unsigned char *key_sent);
int createNewMK(char password[50], unsigned char key[SHA256_DIGEST_LENGTH]);
void recoverPassFromServ(unsigned char key[SHA256_DIGEST_LENGTH]);

/**Lecteur.c**/
int lecteur(MANGAS_DATA *mangaDB, int *chapitreChoisis, bool isTome, int *fullscreen);
char ** loadChapterConfigDat(char* input, int *nombrePage);

/**Mainfunctions.c**/
void mainRakshata();
int mainLecture();
int mainChoixDL();

/**Menu.c**/
int ecranAccueil();
int section();
int showControls();

/**ModuleDL2_tool.c**/
int ecritureDansImport(MANGAS_DATA mangaDB, bool isTome, int chapitreChoisis);
void lancementModuleDL();

/**Native.c**/
FILE* fopenR(void *_path, char *right);
void removeR(char *path);
void renameR(char *initialName, char *newName);
void mkdirR(char *path);
void chdirR();
void resetOriginalCHDir(int *argc, char** argv);
int strend(char *recepter, size_t length, const char *sender);
char* mergeS(char* input1, char* input2);
void *ralloc(size_t length);
int charToInt(char *input);
void fscanfs(FILE* stream, const char *format, ...);
int sscanfs(char *char_input, const char *format, ...);
size_t ustrlen(void *input);
void usstrcpy(void* output, size_t length, const void* input);
void ustrcpy(void* output, const void* input);
void SDL_FreeSurfaceS(SDL_Surface *surface);
void SDL_DestroyTextureS(SDL_Texture *texture);

void removeFolder(char *path);
#define ouvrirSiteTeam(structTeam) ouvrirSite(structTeam->site)
void ouvrirSite(char *URL);
int lancementExternalBinary(char cheminDAcces[100]);
int checkPID(int PID);
#define checkFileExist(filename) (access(filename, F_OK) != -1)
int checkDirExist(char *dirname);

/**PBKDF2.c**/
int internal_pbkdf2(uint32_t prf_hlen, const uint8_t *pw, uint32_t pwlen, const uint8_t *salt, uint32_t saltlen, uint32_t count, uint32_t dklen, uint8_t *dk_ret);
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
void decryptPage(void *_password, unsigned char *buffer_int, unsigned char *buffer_out, size_t length);
void generateFingerPrint(unsigned char output[SHA256_DIGEST_LENGTH+1]);
void get_file_date(const char *filename, char *date);
void killswitchEnabled(char nomTeamCourt[5]);
void screenshotSpoted(char team[LONGUEUR_NOM_MANGA_MAX], char manga[LONGUEUR_NOM_MANGA_MAX], int chapitreChoisis);
SDL_Surface *IMG_LoadS(char *pathRoot, char *pathPage, int numeroChapitre, int page);
void getPasswordArchive(char *fileName, char password[300]);
void Load_KillSwitch(char killswitch_string[NUMBER_MAX_TEAM_KILLSWITCHE][2*SHA256_DIGEST_LENGTH+1]);
int checkKillSwitch(char killswitch_string[NUMBER_MAX_TEAM_KILLSWITCHE][2*SHA256_DIGEST_LENGTH+1], TEAMS_DATA team_to_check);

/**Selection.c**/
int controleurManga(MANGAS_DATA* mangaDB, int contexte, int nombreChapitre, bool *selectMangaDLRightClick);
int checkProjet(MANGAS_DATA mangaDB);
int controleurChapTome(MANGAS_DATA* mangaDB, bool *isTome, int contexte);

/**Settings.c**/
int affichageMenuGestion(); //Remplacer gestion par setting
int menuGestion();
char *loadPrefFile();
void addToPref(char flag, char *stringToAdd);
void removeFromPref(char flag);
void updatePrefs(char flag, char *stringToAdd);
int loadLangueProfile();
int loadEmailProfile();
char* loadLargePrefs(char flag);
void setPrefs(MANGAS_DATA* mangaDB);

/**SHA256.c**/
int sha256(unsigned char* input, void* output);
int sha256_legacy(char input[], char output[2*SHA256_DIGEST_LENGTH+1]);
void sha256_salted(const uint8_t *input, uint32_t inputLen, const uint8_t *salt, uint32_t saltlen, uint8_t *output);

/**Thread.c**/
void createNewThread(void *function, void *arg);
THREAD_TYPE createNewThreadRetValue(void *function, void *arg);
bool isThreadStillRunning(THREAD_TYPE hThread);

/**Tome.c**/
void tomeDBParser(MANGAS_DATA* mangaDB, unsigned char* buffer, size_t size);
void escapeTomeLineElement(META_TOME *ligne);
void refreshTomeList(MANGAS_DATA *mangaDB);
void checkTomeValable(MANGAS_DATA *mangaDB, int *dernierLu);
void getUpdatedTomeList(MANGAS_DATA *mangaDB);
int askForTome(MANGAS_DATA *mangaDB, int contexte);
void displayTemplateTome(MANGAS_DATA* mangaDB, DATA_ENGINE data, int contexte, char texteTrad[SIZE_TRAD_ID_19][TRAD_LENGTH]);
DATA_ENGINE *generateTomeList(MANGAS_DATA mangaDB, bool ordreCroissant, int contexte, char* stringAll, char* stringGeneric);
void printTomeDatas(MANGAS_DATA mangaDB, char *bufferDL, int tome);
int extractNumFromConfigTome(char *input, int ID);

/**Translation.c**/
void loadTrad(char trad[][TRAD_LENGTH], int IDTrad);
int changementLangue();
int tradAvailable();

/**Unzip.c**/
int unzip(char *path, char *output);
int miniunzip (char *inputZip, char *outputZip, char *passwordZip, size_t size, size_t type);

/**Update.c**/
void checkUpdate();
void checkJustUpdated();

/**Utilitaires.c**/
#define crashTemp(string, length) memset(string, 0, length)
void changeTo(char *string, int toFind, int toPut);
int plusOuMoins(int compare1, int compare2, int tolerance);
int sortNumbers(const void *a, const void *b);
int sortMangas(const void *a, const void *b);
int sortTomes(const void *a, const void *b);
void applyWindowsPathCrap(void *input);
void versionRak(char *output);
void setupBufferDL(char *buffer, int size1, int size2, int size3, int size4);
int positionnementApres(FILE* stream, char *stringToFind);
int positionnementApresChar(char* input, char *stringToFind);
void teamOfProject(char nomProjet[LONGUEUR_NOM_MANGA_MAX], char nomTeam[LONGUEUR_NOM_MANGA_MAX]);
void createPath(char *output);
#define isHexa(caract) ((caract >= '0' && caract <= '9') || (caract >= 'a' && caract <= 'f') || (caract >= 'A' && caract <= 'F'))?1:0
#define isNbr(caract) (caract >= '0' && caract <= '9')
void hexToDec(const char *input, unsigned char *output);
void decToHex(const unsigned char *input, size_t length, char *output);
void MajToMin(char* input);
void minToMaj(char* input);
void unescapeLineReturn(char *input);
void restrictEvent();
int defineMaxTextureSize(int sizeIssue);
int isJPEG(void *input);
int isPNG(void *input);
void addToRegistry();
