/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **		Source code and assets are property of Taiki, distribution is stricly forbidden		**
 **                                                                                         **
 *********************************************************************************************/

/*****************************************
 **										**
 **				  ERRORS				**
 **										**
 *****************************************/

enum
{
	COMPARE_UTF8,
	COMPARE_UTF32
};

int compareStrings(const void* a, uint lengthA, const void* b, uint lengthB, int compareEncoding);

/*****************************************
 **										**
 **				  GENERAL				**
 **										**
 *****************************************/

bool isSandboxed();
void configureSandbox();
void registerExtensions();
int getBuildID();


/*****************************************
 **										**
 **				   ERROR				**
 **										**
 *****************************************/

void createCrashFile();
void deleteCrashFile();
void sendToLog(const char * string);
void alertExit(const char * exitReason);

/*****************************************
 **										**
 **				  PREFS					**
 **										**
 *****************************************/

uint32_t getMainThread();
bool shouldDownloadFavorite();
void notifyEmailUpdate();
void restorePrefsFile();
bool removeProjectWithContent();

#ifdef VERBOSE_DB_MANAGEMENT
void logStack(void * address);
#endif

/*****************************************
 **										**
 **				 DB update				**
 **										**
 *****************************************/

void notifyFullUpdate();
void notifyUpdateRepo(REPO_DATA repo);
void notifyUpdateRootRepo(ROOT_REPO_DATA root);
void notifyUpdateProject(PROJECT_DATA project);
void notifyFullUpdateRepo();

/*****************************************
 **										**
 **			  Thumbnail update			**
 **										**
 *****************************************/

void notifyThumbnailUpdate(ICONS_UPDATE * payload);

/*****************************************
 **										**
 **			Restrictions update			**
 **										**
 *****************************************/

void notifyRestrictionChanged();

/*****************************************
 **										**
 **				  Series				**
 **										**
 *****************************************/

void updateRecentSeries();

/*****************************************
 **										**
 **				    MDL					**
 **										**
 *****************************************/

bool checkIfElementAlreadyInMDL(PROJECT_DATA data, bool isTome, uint element);
void addElementToMDL(PROJECT_DATA data, bool isTome, uint element, bool partOfBatch);
void notifyDownloadOver();

/*****************************************
 **										**
 **				   Proxy				**
 **										**
 *****************************************/

bool getSystemProxy(char ** _proxyAddress);