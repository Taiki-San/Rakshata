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

void createCrashFile();
void deleteCrashFile();

/*****************************************
 **										**
 **				  PREFS					**
 **										**
 *****************************************/

uint32_t getMainThread();
bool shouldDownloadFavorite();
void sendToLog(char * string);
void notifyEmailUpdate();
void restorePrefsFile();

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

bool checkIfElementAlreadyInMDL(PROJECT_DATA data, bool isTome, int element);
void addElementToMDL(PROJECT_DATA data, bool isTome, int element, bool partOfBatch);
void notifyDownloadOver();