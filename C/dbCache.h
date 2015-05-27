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
extern sqlite3 *immatureCache;
extern uint nbElemInCache;

extern ROOT_REPO_DATA ** rootRepoList;
extern uint lengthRootRepo;

extern REPO_DATA ** repoList;
extern uint lengthRepo;

extern char *isUpdated;
extern uint lengthIsUpdated;

extern bool mutexInitialized;
extern MUTEX_VAR cacheMutex, cacheParseMutex;

/**DBCache.c**/
sqlite3_stmt * getAddToCacheRequest();
uint addToCache(sqlite3_stmt* request, PROJECT_DATA data, uint64_t repoID, bool isInstalled, bool wantID);
void removeFromCache(PROJECT_DATA data);
void consolidateCache();
bool copyOutputDBToStruct(sqlite3_stmt *state, PROJECT_DATA* output, bool copyDynamic);

//Repository
ROOT_REPO_DATA ** loadRootRepo(char * repoDB, uint *nbRepo);
REPO_DATA ** loadRepo(ROOT_REPO_DATA ** root, uint nbRoot, uint * nbRepo);
uint getFreeRootRepoID();
void updateRepoCache(REPO_DATA ** repoData, uint newAmountOfRepo);
void getRidOfDuplicateInRepo(REPO_DATA ** data, uint nombreRepo);
void insertRootRepoCache(ROOT_REPO_DATA ** newRoot, uint newRootEntries);
void updateRootRepoCache(ROOT_REPO_DATA ** repoData);
void removeNonInstalledSubRepo(REPO_DATA ** _subRepo, uint nbSubRepo, bool haveExtra);
void getRideOfDuplicateInRootRepo(ROOT_REPO_DATA ** data, uint nombreRepo, REPO_DATA *** wantUpdatedRepo, uint *lengthUpdated);
bool isAppropriateNumberOfRepo(uint requestedNumber);

/**DBDebug.c**/
int createRequest(sqlite3 *db, const char *zSql, sqlite3_stmt **ppStmt);
int destroyRequest(sqlite3_stmt *pStmt);
#ifdef DEV_VERSION
void errorLogCallback(void *pArg, int iErrCode, const char *zMsg);
#endif

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
void updateRepo();
int getUpdatedProjectOfRepo(char **projectBuf, REPO_DATA* repo);
void * refreshRepoHelper(REPO_DATA * repo, bool standalone);
void updateProjects();

/******		DBTools.c	  ******/
bool parseRemoteRepoEntry(char *data, ROOT_REPO_DATA *previousData, int version, ROOT_REPO_DATA **output);
bool downloadedProjectListSeemsLegit(char *data);
uint defineBoundsRepoOnProjectDB(PROJECT_DATA * oldData, uint posBase, uint nbElem);

void * updateImagesForProjects(PROJECT_DATA_EXTRA * project, uint nbElem);
void applyChangesProject(PROJECT_DATA * oldData, uint magnitudeOldData, PROJECT_DATA * newData, uint magnitudeNewData);
int createCollate(sqlite3 * database);

/******		DBTags.c	*******/
void initializeTags(void * mainCache);

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
#define RDB_category		9
#define RDB_asianOrder		10
#define RDB_isPaid			11
#define RDB_mainTagID		12
#define RDB_tagMask			13
#define RDB_nombreChapitre	14
#define RDB_chapitres		15
#define RDB_chapitresPrice	16
#define RDB_nombreTomes		17
#define RDB_DRM				18
#define RDB_tomes			19
#define RDB_favoris			20

#define RDBS_dataID			21
#define RDBS_dataType		22
#define RDBS_tagType		23

#define RDB_tagID			24
#define RDB_tagName			25
#define RDB_CAT_ID			26
#define RDB_CAT_rootID		27
#define RDB_CAT_name		28
#define RDB_CAT_tag1		29
#define RDB_CAT_tag2		30
#define RDB_CAT_tag3		31
#define RDB_CAT_tag4		32
#define RDB_CAT_tag5		33
#define RDB_CAT_tag6		34
#define RDB_CAT_tag7		35
#define RDB_CAT_tag8		36
#define RDB_CAT_tag9		37
#define RDB_CAT_tag10		38
#define RDB_CAT_tag11		39
#define RDB_CAT_tag12		40
#define RDB_CAT_tag13		41
#define RDB_CAT_tag14		42
#define RDB_CAT_tag15		43
#define RDB_CAT_tag16		44
#define RDB_CAT_tag17		45
#define RDB_CAT_tag18		46
#define RDB_CAT_tag19		47
#define RDB_CAT_tag20		48
#define RDB_CAT_tag21		49
#define RDB_CAT_tag22		50
#define RDB_CAT_tag23		51
#define RDB_CAT_tag24		52
#define RDB_CAT_tag25		53
#define RDB_CAT_tag26		54
#define RDB_CAT_tag27		55
#define RDB_CAT_tag28		56
#define RDB_CAT_tag29		57
#define RDB_CAT_tag30		58
#define RDB_CAT_tag31		59
#define RDB_CAT_tag32		60

#define RDB_REC_lastRead	1
#define RDB_REC_lastDL		2
#define RDB_REC_team		3
#define RDB_REC_projectID	4

#define DBNAMETOID(s) "`"STRINGIZE(s)"`"

#define SORT_FUNC "LEGOLAS"

enum SEARCH_REQUEST
{
	INSERT_AUTHOR,
	INSERT_SOURCE,
	INSERT_TAG,
	INSERT_CAT,
	INSERT_PROJECT,
};
