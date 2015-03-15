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

#include "sqlite3.h"

//Extremly limited header, have direct access to critical DB 
extern sqlite3 *cache;
extern uint nbElemInCache;

extern ROOT_REPO_DATA ** rootRepoList;
extern uint lengthRootRepo;

extern REPO_DATA ** repoList;
extern uint lengthRepo;

extern char *isUpdated;
extern uint lengthIsUpdated;

extern bool mutexInitialized;
extern MUTEX_VAR cacheMutex;

/**DBCache.c**/
sqlite3_stmt * getAddToCacheRequest();
bool addToCache(sqlite3_stmt* request, PROJECT_DATA data, uint64_t repoID, bool isInstalled);
void removeFromCache(PROJECT_DATA data);
void consolidateCache();
bool copyOutputDBToStruct(sqlite3_stmt *state, PROJECT_DATA* output);

//Repository
void addRootRepoToDB(ROOT_REPO_DATA * newRepo);
ROOT_REPO_DATA ** loadRootRepo(char * repoDB, uint *nbRepo);
REPO_DATA ** loadRepo(ROOT_REPO_DATA ** root, uint nbRoot, uint * nbRepo);
uint getFreeRootRepoID();
void updateRepoCache(REPO_DATA ** repoData, uint newAmountOfRepo);
void getRidOfDuplicateInRepo(REPO_DATA ** data, uint nombreRepo);
void insertRootRepoCache(ROOT_REPO_DATA ** newRoot, const uint newRootEntries);
void updateRootRepoCache(ROOT_REPO_DATA ** repoData);
void removeNonInstalledSubRepo(REPO_DATA ** _subRepo, uint nbSubRepo, bool haveExtra);
void getRideOfDuplicateInRootRepo(ROOT_REPO_DATA ** data, uint nombreRepo);
bool isAppropriateNumberOfRepo(uint requestedNumber);

/**DBSearch.c**/
void buildSearchTables(sqlite3 *_cache);
void * buildSearchJumpTable(sqlite3 * _cache);
void flushSearchJumpTable(void * _table);

bool insertInSearch(void * _table, byte type, PROJECT_DATA project);
bool removeFromSearch(void * _table, uint cacheID);

void checkIfRemainingAndDelete(uint data, byte type);

/**DBRecent.c**/
sqlite3* getPtrRecentDB();
void closeRecentDB(sqlite3 * database);
bool checkRecentDBValid(sqlite3 * DB);

bool updateRecentEntry(sqlite3 *database, PROJECT_DATA data, time_t timestamp, bool wasItADL);
void removeRecentEntry(PROJECT_DATA data);
void removeRecentEntryInternal(char * URLRepo, uint projectID);

/**DBRefresh.c**/
int getUpdatedRepo(char *buffer_repo, uint bufferSize, ROOT_REPO_DATA repo);
void updateRepo();
int getUpdatedProjectOfRepo(char *projectBuf, REPO_DATA* repo);
void * refreshRepo(REPO_DATA * repo, bool standalone);
void updateProjects();

/******		DBTools.c	  ******/
bool parseRemoteRepoEntry(char *data, ROOT_REPO_DATA *previousData, int version, ROOT_REPO_DATA **output);
bool downloadedProjectListSeemsLegit(char *data);
uint defineBoundsRepoOnProjectDB(PROJECT_DATA * oldData, uint posBase, uint nbElem);

void * updateImagesForProjects(PROJECT_DATA_EXTRA * project, uint nbElem);
void applyChangesProject(PROJECT_DATA * oldData, uint magnitudeOldData, PROJECT_DATA * newData, uint magnitudeNewData);

//========= Obfuscation	==========//

//An enum won't be expanded by STRINGIZE, so we must use define
#define RDB_ID 				1
#define RDB_repo 			2
#define RDB_projectID		3
#define RDB_isInstalled		4
#define RDB_projectName		5
#define RDB_description		6
#define RDB_authors			7
#define RDB_status			8
#define RDB_type			9
#define RDB_asianOrder		10
#define RDB_isPaid			11
#define RDB_tag				12
#define RDB_nombreChapitre	13
#define RDB_chapitres		14
#define RDB_chapitresPrice	15
#define RDB_nombreTomes		16
#define RDB_DRM				17
#define RDB_tomes			18
#define RDB_favoris			19

#define RDBS_dataID			20
#define RDBS_dataType		21

#define RDBS_tagCode		22
#define RDBS_tagType		23
#define RDBS_tagID			24

#define RDB_REC_lastRead	1
#define RDB_REC_lastDL		2
#define RDB_REC_team		3
#define RDB_REC_projectID	4

#define DBNAMETOID(s) "`"STRINGIZE(s)"`"

enum SEARCH_REQUEST
{
	INSERT_AUTHOR,
	INSERT_SOURCE,
	INSERT_TAG,
	INSERT_TYPE,
	INSERT_PROJECT,
};
