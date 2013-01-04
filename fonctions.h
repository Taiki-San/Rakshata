/*********************************************************************************************
**      __________         __           .__            __                ____     ____      **
**      \______   \_____  |  | __  _____|  |__ _____ _/  |______    /\  /_   |   /_   |     **
**       |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \   \/   |   |    |   |     **
**       |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_ /\   |   |    |   |     **
**       |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  / \/   |___| /\ |___|     **
**              \/      \/     \/     \/     \/     \/          \/             \/           **
**                                                                                          **
**   Licence propriétaire, code source confidentiel, distribution formellement interdite    **
**                                                                                          **
*********************************************************************************************/

/**Affichage.c**/
int showError();
void initialisationAffichage();
int erreurReseau();
int affichageMenuGestion();
void raffraichissmenent();
void affichageLancement();
int rienALire();
SDL_Surface* createUIAlert(SDL_Surface* alertSurface, char texte[][100], int numberLine);
SDL_Texture * TTF_Write(SDL_Renderer *render, TTF_Font *font, const char *text, SDL_Color fg);
int getWindowSize(int w1h2);
void updateWindowSize(int w, int h);

/**check.c**/
void checkUpdate();
void checkJustUpdated();
void checkRenderBugPresent();
int check_evt();
int checkProjet(MANGAS_DATA mangaDB);
int checkLancementUpdate();
void networkAndVersionTest();
int checkRestore();
int checkRestoreAvailable();
int checkInfopngUpdate(char teamLong[100], char nomProjet[100], int valeurAChecker);
int checkNameFileZip(char fileToTest[256]);
void checkHostNonModifie();
int checkPasNouveauChapitreDansDepot(MANGAS_DATA mangasDB, int chapitre);
int checkFileExist(char filename[]);
int checkButtonPressed(int button_selected[6]);
int checkFirstLineButtonPressed(int button_selected[6]);
int checkSecondLineButtonPressed(int button_selected[6]);
int checkFileValide(FILE* file);
int checkChapitreUnread(MANGAS_DATA mangasDB);
int checkChapterEncrypted(MANGAS_DATA mangasDB, int chapitreChoisis);
int checkWindowEventValid(int EventWindowEvent);
int checkNetworkState(int state);

/**Database.c**/
MANGAS_DATA* miseEnCache(int mode);
MANGAS_DATA* allocateDatabase(size_t length);
void freeMangaData(MANGAS_DATA* mangasDB, size_t length);
void updateDataBase();
int deleteManga();
int isItNew(MANGAS_DATA mangasDB);
void lastChapitreLu(MANGAS_DATA* mangasDB, int dernierChapitre);
void get_update_repo(char *buffer_repo, TEAMS_DATA* teams);
void get_update_mangas(char *buffer_manga, TEAMS_DATA* teams);
void update_repo();
void update_mangas();
int internal_deleteChapitre(int firstChapter, int lastChapter, int lastRead, int chapitreDelete, char mangaDispo[LONGUEUR_NOM_MANGA_MAX], char teamsLong[LONGUEUR_NOM_MANGA_MAX]);

/**Donwload.c**/
int download(char *adresse, char *repertoire, int activation);

/**Engine.c**/
int displayMangas(MANGAS_DATA* mangaDB, int sectionChoisis, int nombreChapitre, int hauteurAffichage);
int mangaSelection(int mode, int tailleTexte[MANGAPARPAGE_TRI], int hauteurChapitre, int *manuel);
int displayMenu(char texte[][TRAD_LENGTH], int nombreElements, int hauteurBloc);
void showNumero(TTF_Font *police, int choix, int hauteurNum);
void analysisOutputSelectionTricolonne(int sectionChoisis, int *mangaChoisis, MANGAS_DATA* mangaDB, int mangaColonne[3], int button_selected[6], int *changementDePage, int *pageSelection, int pageTotale, int manuel, int *limitationLettre, int *refreshMultiPage);
void generateChoicePanel(char trad[13][100], int enable[6]);
void loadMultiPage(int nombreManga, int *pageTotale, int *pageSelection);
int letterLimitationEnforced(int letter, char firstLetterOfTheManga);
int buttonLimitationEnforced(int button_selected[6], int statusMangasToTest, int genreMangasToTest);
void button_available(MANGAS_DATA* mangaDB, int button[6]);

/**Error.c**/
void logR(char *error);
void connexionNeededToAllowANewComputer();
int libcurlErrorCode(CURLcode code);

/**Interface.c**/
void updateSectionMessage(char messageVersion[5]);
void checkSectionMessageUpdate();

/**IO.c**/
int nombreEntree(SDL_Event event);
int waitEnter();
int waitClavier(int nombreMax, int startFromX, int startFromY, char *retour);
#define getLetterPushed(event) (event.text.text[0] >= 'A' && event.text.text[0] <= 'z')?event.text.text[0]:0
#define checkIfNumber(c) (c >= '0' && c <= '9')?1:0

/**Keys.c**/
int createSecurePasswordDB(unsigned char *key_sent);
int getMasterKey(unsigned char *input);
void generateKey(unsigned char output[HASH_LENGTH]);
int get_compte_infos();
int logon();
int check_login(char adresseEmail[100]);
int checkPass(char adresseEmail[100], char password[50], int login);
int sendPassToServ(unsigned char key[HASH_LENGTH]);
void recoverPassToServ(unsigned char key[SHA256_DIGEST_LENGTH], int mode);

/**Lecteur.c**/
int lecteur(MANGAS_DATA mangaDB, int *chapitreChoisis, int *fullscreen);
int configFileLoader(char* input, int *nombrePage, char output[NOMBRE_PAGE_MAX][LONGUEUR_NOM_PAGE]);
SDL_Texture* loadControlBar();
int changementDePage(int direction, int *changementPage, int *finDuChapitre, int *pageEnCoursDeLecture, int pageTotal, int *chapitreChoisis, MANGAS_DATA mangaDB);
void cleanMemory(SDL_Surface *chapitre, SDL_Texture *chapitre_texture, SDL_Surface *OChapitre, SDL_Surface *NChapitre, SDL_Texture *infoSurface, SDL_Texture *bandeauControle, TTF_Font *police);
void refreshScreen(SDL_Texture *chapitre, SDL_Rect positionSlide, SDL_Rect positionPage, SDL_Rect positionBandeauControle, SDL_Texture *bandeauControle, SDL_Texture *infoSurface, SDL_Rect positionInfos, int *restoreState, int *tempsDebutExplication, int *nouveauChapitreATelecharger, SDL_Surface *explication, SDL_Surface *UIAlert, int pageAccesDirect, SDL_Surface *UI_pageAccesDirect);
void slideOneStepDown(SDL_Surface *chapitre, SDL_Rect *positionSlide, SDL_Rect *positionPage, int ctrlPressed, int pageTropGrande, int move, int *noRefresh);
void slideOneStepUp(SDL_Surface *chapitre, SDL_Rect *positionSlide, SDL_Rect *positionPage, int ctrlPressed, int pageTropGrande, int move, int *noRefresh);
void anythingNew(int extremes[2], MANGAS_DATA mangaDB);
int clicOnButton(const int x, const int y, const int positionBandeauX);
void applyFullscreen(int *var_fullscreen, int *checkChange, int *changementEtat);

/**Mainfunctions.c**/
void mainRakshata();
int mainLecture();
int mainChoixDL();
void mainDL();

/**Menu.c**/
int ecranAccueil();
int showControls();
int menuGestion();

/**ModuleDL.c**/
int telechargement();
#ifdef _WIN32
DWORD WINAPI installation(LPVOID datas);
#else
void* installation(void* datas);
#endif
int interditWhileDL();
int ecritureDansImport(MANGAS_DATA mangaDB, int chapitreChoisis);
void DLmanager();
void lancementModuleDL();

/**Native.c**/
FILE* fopenR(void *_path, char *right);
void removeR(char *path);
void renameR(char *initialName, char *newName);
void mkdirR(char *path);
void chdirR();
void strend(char *recepter, const char *sender);
int charToInt(char *input);
void fscanfs(FILE* stream, const char *format, ...);
int sscanfs(char *char_input, const char *format, ...);
size_t ustrlen(void *input);
void usstrcpy(void* output, size_t length, void* input);
void ustrcpy(void* output, void* input);
void SDL_FreeSurfaceS(SDL_Surface *surface);
void SDL_DestroyTextureS(SDL_Texture *texture);

void removeFolder(char *path);
int createNewThread(void *function);
void ouvrirSite(TEAMS_DATA* teams);
void updateDirectory();
int lancementExternalBinary(char cheminDAcces[100]);
int unzip(char *path, char *output);
int checkPID(int PID);
void get_file_date(const char *filename, char *date);
int tradAvailable();

/**PBKDF2.c**/
int I2pbkdf2(uint32_t prf_hlen, const uint8_t *pw, uint32_t pwlen, const uint8_t *salt, uint32_t saltlen, uint32_t count, uint32_t dklen, uint8_t *dk_ret);

/**Repo.c**/
int defineTypeRepo(char *URL);
int ajoutRepo();
int deleteRepo();
int affichageRepoIconnue();
int confirmationRepo(char team[LONGUEUR_NOM_MANGA_MAX]);

/**Securite.c**/
int AESEncrypt(void *_password, void *_path_input, void *_path_output, int cryptIntoMemory);
int AESDecrypt(void *_password, void *_path_input, void *_path_output, int cryptIntoMemory);
SDL_Surface *IMG_LoadS(SDL_Surface *surface_page, char teamLong[LONGUEUR_NOM_MANGA_MAX], char mangas[LONGUEUR_NOM_MANGA_MAX], int numeroChapitre, char nomPage[LONGUEUR_NOM_PAGE], int page);
void generateFingerPrint(unsigned char output[SHA256_DIGEST_LENGTH]);
int getPassword(char password[100]);
void getPasswordArchive(char *fileName, char password[300]);
void Load_KillSwitch(char killswitch_string[NUMBER_MAX_TEAM_KILLSWITCHE][LONGUEUR_ID_TEAM]);
int checkKillSwitch(char killswitch_string[NUMBER_MAX_TEAM_KILLSWITCHE][LONGUEUR_ID_TEAM], char ID_To_Test[LONGUEUR_ID_TEAM]);
void killswitchEnabled(char nomTeamCourt[5]);
void screenshotSpoted(char team[LONGUEUR_NOM_MANGA_MAX], char manga[LONGUEUR_NOM_MANGA_MAX], int chapitreChoisis);
void pbkdf2(uint8_t input[], uint8_t salt[], uint8_t output[]);

/**Selection.c**/
int section();
int manga(int sectionChoisis, MANGAS_DATA* mangas_db, int nombreChapitre);
int chapitre(MANGAS_DATA mangaDB, int mode);

/**SHA256.c**/
int sha256(unsigned char* input, void* output);
int sha256_legacy(char input[], char output[HASH_LENGTH]);
void sha256_salted(const uint8_t *input, uint32_t inputLen, const uint8_t *salt, uint32_t saltlen, uint8_t *output);

/**Translation.c**/
void loadTrad(char trad[][100], int IDTrad);
int changementLangue();
int loadLangueProfile();

/**Unzip.c**/
int miniunzip (char *inputZip, char *outputZip, char *passwordZip, size_t size, size_t type);

/**Utilitaires.c**/
#define crashTemp(string, length) memset(string, 0, length)
void changeTo(char *string, int toFind, int toPut);
int plusOuMoins(int compare1, int compare2, int tolerance);
int compare(const void *a, const void *b);
void restartEcran();
void chargement();
void applyWindowsPathCrap(void *input);
void applyBackground(int x, int y, int w, int h);
void nameWindow(const int value);
void version(char *output);
void setupBufferDL(char *buffer, int size1, int size2, int size3, int size4);
int positionnementApres(FILE* stream, char *stringToFind);
void teamOfProject(char nomProjet[LONGUEUR_NOM_MANGA_MAX], char nomTeam[LONGUEUR_NOM_MANGA_MAX]);
void createPath(char output[]);
void getResolution();
#define isHexa(caract) ((caract >= '0' && caract <= '9') || (caract >= 'a' && caract <= 'f') || (caract >= 'A' && caract <= 'F'))?1:0
void hexToDec(const char *input, unsigned char *output);
void decToHex(const unsigned char *input, size_t length, char *output);
void MajToMin(char* input);
void minToMaj(char* input);
void restrictEvent();
