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
void raffraichissmenent();
void affichageLancement();
void chargement(SDL_Renderer* rendererVar, int h, int w);
void loadPalette();
void loadIcon(SDL_Window *window_ptr);
SDL_Renderer* setupRendererSafe(SDL_Window *window_ptr);
SDL_Surface* createUIAlert(SDL_Surface* alertSurface, char texte[][100], int numberLine);
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
void displayTemplateChapitre(MANGAS_DATA* mangaDB, int contexte, int nombreElements, char texteTrad[SIZE_TRAD_ID_19][TRAD_LENGTH], int dernierLu);
int autoSelectionChapitre(MANGAS_DATA *mangaDB, int contexte);
MANGAS_DATA *generateChapterList(MANGAS_DATA mangaDB, bool ordreCroissant, int mode, char* stringAll, char* stringGeneric);

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
int checkWindowEventValid(int EventWindowEvent);
int checkNameFileZip(char fileToTest[256]);
int checkFileValide(FILE* file);
bool checkPathEscape(char *string, int length);
bool checkChapterReadable(MANGAS_DATA mangaDB, int *chapitre_ptr);
bool checkTomeReadable(MANGAS_DATA mangaDB, int ID);
bool checkReadable(MANGAS_DATA mangaDB, bool isTome, void *data);

/**CTCommon.c**/
int autoSelectionChapitreTome(MANGAS_DATA *mangaDB, bool isTome, int min, int max, int contexte);
void displayTemplateChapitreTome(MANGAS_DATA* mangaDB, int contexte, int isTome, int nombreElements, char texteTrad[SIZE_TRAD_ID_19][TRAD_LENGTH], int dernierLu);
void displayIconeChapOrTome(bool isTome);
int askForCT(MANGAS_DATA* mangaDB, bool *isTome, int contexte);
void getUpdatedCTList(MANGAS_DATA *mangaDB, bool isTome);

/**Database.c**/
MANGAS_DATA* miseEnCache(int mode);
MANGAS_DATA* allocateDatabase(size_t length);
void freeMangaData(MANGAS_DATA* mangasDB, size_t length);
void updateDataBase();
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
int download_UI(TMP_DL *output);
int download_mem(char* adresse, char *buffer_out, size_t buffer_length, int SSL_enabled);
int download_disk(char* adresse, char *file_name, int SSL_enabled);
int checkDLInProgress();

/**Engine.c**/
int displayMenu(char texte[][TRAD_LENGTH], int nombreElements, int hauteurBloc);
int displayMangas(MANGAS_DATA* mangaDB, int contexte, int nombreChapitre, int hauteurAffichage);
void generateChoicePanel(char trad[13][100], int enable[6]);
void showNumero(TTF_Font *police, int choix, int virgule, int hauteurNum);
int mangaSelection(int contexte, int tailleTexte[MANGAPARPAGE_TRI], int hauteurChapitre, int bordureLaterale, int largeurMaxManga, int *manuel);
int TRI_mangaToDisplay(int contexte, int limitationLettre, MANGAS_DATA mangaDB, int button_selected[6]);
void analysisOutputSelectionTricolonne(int contexte, int *mangaChoisis, MANGAS_DATA* mangaDB, int mangaColonne[3], int button_selected[6], int *changementDePage, int *pageSelection, int pageTotale, int manuel, int *limitationLettre, int *refreshMultiPage, bool modeLigne);
int letterLimitationEnforced(int letter, char firstLetterOfTheManga);
int buttonLimitationEnforced(int button_selected[8], int statusMangasToTest, int genreMangasToTest, int favorite);
void button_available(MANGAS_DATA* mangaDB, int button[8]);
SDL_Color getEngineColor(SDL_Color couleurUnread, SDL_Color couleurNew, SDL_Color couleurTexte, int contexte, MANGAS_DATA mangaDB);
void loadMultiPage(int nombreManga, int *pageTotale, int *pageSelection);

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
#define getLetterPushed(event) (event.text.text[0] >= 'A' && event.text.text[0] <= 'z')?event.text.text[0]:0

/**Keys.c**/
int getMasterKey(unsigned char *input);
void generateKey(unsigned char output[SHA256_DIGEST_LENGTH]);
int earlyInit();
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
int configFileLoader(MANGAS_DATA *mangaDB, bool isTome, int chapitre_tome, DATA_LECTURE* dataReader);
char ** loadChapterConfigDat(char* input, int *nombrePage);
SDL_Texture* loadControlBar(int favState);
void generateMessageInfoLecteur(MANGAS_DATA mangaDB, DATA_LECTURE dataReader, char localization[SIZE_TRAD_ID_21][TRAD_LENGTH], bool isTome, int fullscreen, int curPosIntoStruct, char* output, int sizeOut);
int changementDePage(MANGAS_DATA *mangaDB, DATA_LECTURE* dataReader, bool isTome, bool goToNextPage, int *changementPage, int *finDuChapitre, int *chapitreChoisis, int currentPosIntoStructure);
int changementDeChapitre(MANGAS_DATA* mangaDB, bool isTome, int posIntoStructToTest, int *chapitreChoisis);
void cleanMemory(DATA_LECTURE dataReader, SDL_Surface *chapitre, SDL_Texture *chapitre_texture, SDL_Surface *OChapitre, SDL_Surface *NChapitre, SDL_Surface* UIAlert, SDL_Surface* UI_PageAccesDirect, SDL_Texture *infoSurface, SDL_Texture *bandeauControle, TTF_Font *police);
void freeCurrentPage(SDL_Texture *texture);
void refreshScreen(SDL_Texture *chapitre, SDL_Rect positionSlide, SDL_Rect positionPage, SDL_Rect positionBandeauControle, SDL_Texture *bandeauControle, SDL_Texture *infoSurface, SDL_Rect positionInfos, int *restoreState, int *tempsDebutExplication, int *nouveauChapitreATelecharger, SDL_Surface *explication, SDL_Surface *UIAlert, int pageAccesDirect, SDL_Surface *UI_pageAccesDirect);
void slideOneStepDown(SDL_Surface *chapitre, SDL_Rect *positionSlide, SDL_Rect *positionPage, int ctrlPressed, int pageTropGrande, int move, int *noRefresh);
void slideOneStepUp(SDL_Surface *chapitre, SDL_Rect *positionSlide, SDL_Rect *positionPage, int ctrlPressed, int pageTropGrande, int move, int *noRefresh);
int clicOnButton(const int x, const int y, const int positionBandeauX);
void applyFullscreen(int *var_fullscreen, int *checkChange, int *changementEtat);
void *checkPasNouveauChapitreDansDepot(MANGAS_DATA* mangasDB, bool isTome, int chapitre);
void addtoDownloadListFromReader(MANGAS_DATA *mangaDB, DATA_LECTURE dataReader, bool isTome, void* new_data);

/**Mainfunctions.c**/
void mainRakshata();
int mainLecture();
int mainChoixDL();
void mainDL();

/**Menu.c**/
int ecranAccueil();
int section();
int showControls();

/**ModuleDL.c**/
int telechargement();
void installation(DATA_INSTALL* datas);
int ecritureDansImport(MANGAS_DATA mangaDB, bool isTome, int chapitreChoisis);
void DLmanager();
void lancementModuleDL();
void updateWindowSizeDL(int w, int h);

/**ModuleDL2_tool.c**/
char* MDL_craftDownloadURL(DATA_LOADED data);
char* internalCraftBaseURL(TEAMS_DATA teamData, int* length);
void MDL_displayDownloadDataMain1(SDL_Renderer *rendererVar, DATA_LOADED data, int pourcentageTotal, char localization[5][TRAD_LENGTH]);
DATA_LOADED ** MDL_loadDataFromImport(MANGAS_DATA* mangaDB, int *nombreMangaTotal);
DATA_LOADED ** MDL_updateDownloadList(MANGAS_DATA* mangaDB, int* nombreMangaTotal, DATA_LOADED ** oldDownloadList);
void startInstallation(DATA_LOADED datas, TMP_DL dataDownloaded, bool isTomeAndLastElem);
bool checkIfWebsiteAlreadyOpened(TEAMS_DATA teamToCheck, char ***historiqueTeam);
bool checkChapterAlreadyInstalled(DATA_LOADED dataToCheck);
void grabInfoPNG(MANGAS_DATA mangaToCheck);
DATA_LOADED** getTomeDetails(DATA_LOADED tomeDatas, int *outLength);

/**Native.c**/
FILE* fopenR(void *_path, char *right);
void removeR(char *path);
void renameR(char *initialName, char *newName);
void mkdirR(char *path);
void chdirR();
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
int createNewThread(void *function, void *arg);
void ouvrirSite(TEAMS_DATA* teams);
int lancementExternalBinary(char cheminDAcces[100]);
int checkPID(int PID);
int checkFileExist(char *filename);
int checkDirExist(char *dirname);

/**PBKDF2.c**/
int internal_pbkdf2(uint32_t prf_hlen, const uint8_t *pw, uint32_t pwlen, const uint8_t *salt, uint32_t saltlen, uint32_t count, uint32_t dklen, uint8_t *dk_ret);
void pbkdf2(uint8_t input[], uint8_t salt[], uint8_t output[]);

/**Repo.c**/
int ajoutRepo();
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
void Load_KillSwitch(char killswitch_string[NUMBER_MAX_TEAM_KILLSWITCHE][LONGUEUR_ID_TEAM]);
int checkKillSwitch(char killswitch_string[NUMBER_MAX_TEAM_KILLSWITCHE][LONGUEUR_ID_TEAM], TEAMS_DATA team_to_check);

/**Selection.c**/
int controleurManga(MANGAS_DATA* mangas_db, int contexte, int nombreChapitre);
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

/**Tome.c**/
void tomeDBParser(MANGAS_DATA* mangaDB, unsigned char* buffer, size_t size);
void refreshTomeList(MANGAS_DATA *mangaDB);
void checkTomeValable(MANGAS_DATA *mangaDB, int *dernierLu);
void getUpdatedTomeList(MANGAS_DATA *mangaDB);
int askForTome(MANGAS_DATA *mangaDB, int contexte);
void displayTemplateTome(MANGAS_DATA *mangaDB, int nombreElements, int contexte, char texteTrad[SIZE_TRAD_ID_19][TRAD_LENGTH], int dernierLu);
int autoSelectionTome(MANGAS_DATA *mangaDB, int contexte);
MANGAS_DATA *generateTomeList(MANGAS_DATA mangaDB, bool ordreCroissant, int contexte, char* stringAll, char* stringGeneric);
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
int sortMangasToDownload(const void *a, const void *b);
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
void restrictEvent();
int defineMaxTextureSize(int sizeIssue);
int isJPEG(void *input);
int isPNG(void *input);
