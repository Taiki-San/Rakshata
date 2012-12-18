/*********************************************
**	        	 Rakshata v1.1 		        **
**     Licence propriétaire, code source    **
**        confidentiel, distribution        **
**          formellement interdite          **
**********************************************/

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
void checkJustUpdated(char *argv);
int check_evt();
int checkProjet(char projet[LONGUEUR_NOM_MANGA_MAX]);
int checkLancementUpdate();
void networkAndVersionTest();
int checkRestore();
int checkRestoreAvailable();
int checkInfopngUpdate(char teamLong[100], char nomProjet[100], int valeurAChecker);
int checkNameFileZip(char fileToTest[256]);
void checkHostNonModifie();
int clicNotSlide(SDL_Event event);
int checkPasNouveauChapitreDansDepot(char team[LONGUEUR_NOM_MANGA_MAX], char mangaLong[LONGUEUR_NOM_MANGA_MAX], int chapitre);
int checkFileExist(char filename[]);
int checkButtonPressed(int button_selected[6]);
int checkFirstLineButtonPressed(int button_selected[6]);
int checkSecondLineButtonPressed(int button_selected[6]);
int checkFileValide(FILE* file);
int checkChapitreUnread(char nomManga[LONGUEUR_NOM_MANGA_MAX]);
int checkChapterEncrypted(char teamLong[], char mangaDispo[], int chapitreChoisis);
int checkWindowEventValid(int EventWindowEvent);

/**Database.c**/
void miseEnCache(char mangaDispo[NOMBRE_MANGA_MAX][LONGUEUR_NOM_MANGA_MAX], char mangaDispoCourt[NOMBRE_MANGA_MAX][LONGUEUR_COURT], int categorie[NOMBRE_MANGA_MAX], int premierChapitreDispo[NOMBRE_MANGA_MAX], int dernierChapitreDispo[NOMBRE_MANGA_MAX], char teamsLong[NOMBRE_MANGA_MAX][LONGUEUR_NOM_MANGA_MAX], char teamsCourt[NOMBRE_MANGA_MAX][LONGUEUR_COURT], int mode);
void updateDataBase();
int deleteManga();
int isItNew(char mangaATester[LONGUEUR_NOM_MANGA_MAX]);
void lastChapitreLu(char nomManga[LONGUEUR_NOM_MANGA_MAX], int dernierChapitre);
void get_update_repo(char *buffer_repo, char mode[10], char URL[LONGUEUR_URL]);
void get_update_mangas(char *buffer_repo, char mode[10], char URL[LONGUEUR_URL]);
void update_repo();
void update_mangas();
int internal_deleteChapitre(int firstChapter, int lastChapter, int lastRead, int chapitreDelete, char mangaDispo[LONGUEUR_NOM_MANGA_MAX], char teamsLong[LONGUEUR_NOM_MANGA_MAX]);

/**Donwload.c**/
int download(char *adresse, char *repertoire, int activation);

/**Engine.c**/
int mangaUnicolonne(TTF_Font *police, char mangaDispo[NOMBRE_MANGA_MAX][LONGUEUR_NOM_MANGA_MAX], int sectionChoisis, int nombreMangaElligible, int mangaElligibles[NOMBRE_MANGA_MAX]);
int mangaTriColonne(char mangaDispo[NOMBRE_MANGA_MAX][LONGUEUR_NOM_MANGA_MAX], int sectionChoisis, int sectionsMangas[NOMBRE_MANGA_MAX], int nombreChapitre, int hauteurAffichage);
int mangaSelection(int mode, int tailleTexte[MANGAPARPAGE_TRI], int hauteurChapitre, int *manuel);
void showNumero(TTF_Font *police, int choix, int hauteurNum);
void analysisOutputSelectionTricolonne(int sectionChoisis, int *mangaChoisis, char mangaDispo[NOMBRE_MANGA_MAX][LONGUEUR_NOM_MANGA_MAX], int sectionsMangas[NOMBRE_MANGA_MAX], int mangaColonne[3], int button_selected[6], int *changementDePage, int *pageSelection, int pageTotale, int manuel, int *limitationLettre, int *refreshMultiPage);
void generateChoicePanel(char trad[13][100], int enable[6]);
void loadMultiPage(int nombreManga, int *pageTotale, int *pageSelection);
int letterLimitationEnforced(int letter, char firstLetterOfTheManga);
int buttonLimitationEnforced(int button_selected[6], int sectionsMangasToTest);
void button_available(int sectionsMangas[NOMBRE_MANGA_MAX], int button[6]);

/**Interface.c**/
void updateSectionMessage(char messageVersion[5]);
void checkSectionMessageUpdate();

/**IO.c**/
int nombreEntree(SDL_Event event);
int waitEnter();
int waitClavier(int nombreMax, int startFromX, int startFromY, char *retour);
int getLetterPushed(SDL_Event event);
int checkIfNumber (int c);
void logR(char *error);
void connexionNeededToAllowANewComputer();

/**Keys.c**/
int createSecurePasswordDB(unsigned char *key_sent);
int getMasterKey(unsigned char *input);
void generateKey(unsigned char output[HASH_LENGTH]);
int get_compte_infos();
int logon();
int check_login(char adresseEmail[100]);
int checkPass(char adresseEmail[100], char password[100], int login);
int sendPassToServ(unsigned char key[HASH_LENGTH]);
void recoverPassToServ(unsigned char key[HASH_LENGTH], int mode);

/**Langue.c**/
int changementLangue();

/**Lecteur.c**/
int lecteur(int *chapitreChoisis, int *fullscreen, char mangaDispo[LONGUEUR_NOM_MANGA_MAX], char team[LONGUEUR_NOM_MANGA_MAX]);
int configFileLoader(char* input, int *nombrePage, char output[NOMBRE_PAGE_MAX][LONGUEUR_NOM_PAGE]);
SDL_Texture* loadControlBar();
int changementDePage(int direction, int *changementPage, int *finDuChapitre, int *pageEnCoursDeLecture, int pageTotal, int *chapitreChoisis, char mangaDispo[LONGUEUR_NOM_MANGA_MAX]);
void cleanMemory(SDL_Surface *chapitre, SDL_Texture *chapitre_texture, SDL_Surface *OChapitre, SDL_Surface *NChapitre, SDL_Texture *infoSurface, SDL_Texture *bandeauControle, TTF_Font *police);
void refreshScreen(SDL_Texture *chapitre, SDL_Rect positionSlide, SDL_Rect positionPage, SDL_Rect positionBandeauControle, SDL_Texture *bandeauControle, SDL_Texture *infoSurface, SDL_Rect positionInfos, int *restoreState, int *tempsDebutExplication, int *nouveauChapitreATelecharger, SDL_Surface *explication, SDL_Surface *UIAlert, int pageAccesDirect, SDL_Surface *UI_pageAccesDirect);
void slideOneStepDown(SDL_Surface *chapitre, SDL_Rect *positionSlide, SDL_Rect *positionPage, int ctrlPressed, int pageTropGrande, int move, int *noRefresh);
void slideOneStepUp(SDL_Surface *chapitre, SDL_Rect *positionSlide, SDL_Rect *positionPage, int ctrlPressed, int pageTropGrande, int move, int *noRefresh);
void anythingNew(int extremes[2], char mangaChoisis[LONGUEUR_NOM_MANGA_MAX]);
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
int ecritureDansImport(char mangaDispoLong[LONGUEUR_NOM_MANGA_MAX], char mangaDispoCourt[LONGUEUR_COURT], int chapitreChoisis, char teamsCourt[LONGUEUR_COURT]);
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
void ustrcpy(void* output, void* input);
void SDL_FreeSurfaceS(SDL_Surface *surface);
void SDL_DestroyTextureS(SDL_Texture *texture);

void removeFolder(char *path);
int createNewThread(void *function);
void ouvrirSite(char team[LONGUEURMANGA]);
void updateDirectory(char *argv);
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
void Load_KillSwitch(char killswitch_string[NUMBER_MAX_TEAM_KILLSWITCHE][LONGUEUR_ID_MAX]);
int checkKillSwitch(char killswitch_string[NUMBER_MAX_TEAM_KILLSWITCHE][LONGUEUR_ID_MAX], char ID_To_Test[LONGUEUR_ID_MAX]);
void killswitchEnabled(char nomTeamCourt[5]);
void screenshotSpoted(char team[LONGUEUR_NOM_MANGA_MAX], char manga[LONGUEUR_NOM_MANGA_MAX], int chapitreChoisis);
void pbkdf2(uint8_t input[], uint8_t salt[], uint8_t output[]);

/**Selection.c**/
int section();
int manga(int sectionChoisis, int sectionManga[NOMBRE_MANGA_MAX], char mangaDispo[NOMBRE_MANGA_MAX][LONGUEUR_NOM_MANGA_MAX], int nombreChapitre);
int chapitre(char team[LONGUEUR_NOM_MANGA_MAX], char mangaSoumis[LONGUEUR_NOM_MANGA_MAX], int mode);

/**SHA256.c**/
int sha256(unsigned char* input, void* output);
int sha256_legacy(char input[], char output[HASH_LENGTH]);
void sha256_salted(const uint8_t *input, uint32_t inputLen, const uint8_t *salt, uint32_t saltlen, uint8_t *output);

/**Translation.c**/
void loadTrad(char trad[][100], int IDTrad);

/**Unzip.c**/
int miniunzip (char *inputZip, char *outputZip, char *passwordZip, int type);

/**Utilitaires.c**/
void crashTemp(void *temp, int longueur);
void conversionAvant(char mangaDispo[NOMBRE_MANGA_MAX][LONGUEUR_NOM_MANGA_MAX]);
void conversionApres(char mangaDispo[NOMBRE_MANGA_MAX][LONGUEUR_NOM_MANGA_MAX]);
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
int isHexa(char caract);
void hexToDec(const char *input, unsigned char *output);
void decToHex(const unsigned char *input, size_t length, char *output);
void MajToMin(char* input);
void minToMaj(char* input);
void restrictEvent();
