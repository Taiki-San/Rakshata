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

#define INITIAL_BUFFER_SIZE			1024
#define MAX_PROJECT_LINE_LENGTH		(LENGTH_PROJECT_NAME + LONGUEUR_COURT + 7 * 11 + 30)		//7 nombres * 10 digits + 30 pour de la marge, les espace, toussa
#define MAX_TEAM_LINE_LENGTH		(LENGTH_PROJECT_NAME + LONGUEUR_COURT + LONGUEUR_TYPE_TEAM + LONGUEUR_URL + LONGUEUR_SITE + 25)
#define DB_CACHE_EXPIRENCY			5*60*1000	//5 minutes

unsigned long alreadyRefreshed;

/******		DBTools.c	  ******/
bool parseRemoteRepoEntry(char *data, ROOT_REPO_DATA *previousData, int version, ROOT_REPO_DATA **output);

uint defineBoundsRepoOnProjectDB(PROJECT_DATA * oldData, uint posBase, uint nbElem);
bool downloadedProjectListSeemsLegit(char *data);
uint getNumberLineReturn(char *input);
bool extractCurrentLine(char * input, uint * posInput, char * output, uint lengthOutput);
bool isProjectListSorted(PROJECT_DATA* data, uint length);
void updatePageInfoForProjects(PROJECT_DATA_EXTRA * project, uint nbElem);
void getPageInfo(REPO_DATA repo, uint projectID, bool large, char * filename);
void applyChangesProject(PROJECT_DATA * oldData, uint magnitudeOldData, PROJECT_DATA * newData, uint magnitudeNewData);

void resetUpdateDBCache();
bool isInstalled(char * basePath);

/**DBCache.c**/
int setupBDDCache();
void syncCacheToDisk(byte syncCode);
void flushDB();
sqlite3_stmt * getAddToCacheRequest();
bool addToCache(sqlite3_stmt* request, PROJECT_DATA data, uint64_t repoID, bool isInstalled);
bool updateCache(PROJECT_DATA data, char whatCanIUse, uint projectID);
void removeFromCache(PROJECT_DATA data);
void consolidateCache();
bool copyOutputDBToStruct(sqlite3_stmt *state, PROJECT_DATA* output);

//Repository
uint getRepoIndex(REPO_DATA * repo);
bool addRepoToDB(ROOT_REPO_DATA * newRepo);
ROOT_REPO_DATA ** loadRootRepo(char * repoDB, uint *nbRepo);
REPO_DATA ** loadRepo(ROOT_REPO_DATA ** root, uint nbRoot, uint * nbRepo);
void ** getCopyKnownRepo(uint * nbRepo, bool wantRoot);
int getIndexOfRepo(uint parentID, uint repoID);
uint getFreeRootRepoID();
void updateRepoCache(REPO_DATA ** repoData, uint newAmountOfRepo);
void getRidOfDuplicateInRepo(REPO_DATA ** data, uint *nombreRepo);
void updateRootRepoCache(ROOT_REPO_DATA ** repoData, uint newAmountOfRepo);
void removeNonInstalledSubRepo(REPO_DATA ** _subRepo, uint * nbSubRepo, bool haveExtra);
void getRideOfDuplicateInRootRepo(ROOT_REPO_DATA ** data, uint *nombreRepo);
bool isAppropriateNumberOfRepo(uint requestedNumber);
void freeRootRepo(ROOT_REPO_DATA ** root);
void freeRepo(REPO_DATA ** repos);

//Searches
void * getUpdatedCTForID(uint cacheID, bool wantTome, size_t * nbElemUpdated, uint ** price);
void updateTomeDetails(uint cacheID, uint nbTomes, META_TOME* tomeData);
void setInstalled(uint cacheID);

/**DBSearch.c**/
void buildSearchTables(sqlite3 *_cache);
void * buildSearchJumpTable(sqlite3 * _cache);
void flushSearchJumpTable(void * _table);

bool insertInSearch(void * _table, byte type, PROJECT_DATA project);
bool removeFromSearch(void * _table, uint cacheID);
bool updateProjectSearch(void * _table, PROJECT_DATA project);

void checkIfRemainingAndDelete(uint data, byte type);

bool insertRestriction(uint64_t code, byte type);
bool removeRestriction(uint64_t code, byte type);

bool getProjectSearchData(void * table, uint cacheID, uint * authorID, uint * tagID, uint * typeID);
uint64_t * getSearchData(byte type, charType *** dataName, uint * dataLength);

/**DBRefresh.c**/
void resetUpdateDBCache();
int getUpdatedRepo(char *buffer_repo, uint bufferSize, ROOT_REPO_DATA repo);
void updateRepo();
int getUpdatedProjectOfRepo(char *projectBuf, REPO_DATA* repo);
void refreshRepo(REPO_DATA * repo, bool standalone);
void updateProjectsFromRepo(PROJECT_DATA* oldData, uint posBase, uint posEnd, bool standalone);
void updateProjects();
int databaseVersion(char* projectDB);

/**DBRecent.c**/
sqlite3* getPtrRecentDB();
bool checkRecentDBValid(sqlite3 * DB);

bool updateRecentEntry(PROJECT_DATA data, time_t timestamp, bool wasItADL);
void removeRecentEntry(PROJECT_DATA data);
void removeRecentEntryInternal(char * URLRepo, uint projectID);

/*Database*/

enum syncCode
{
	SYNC_REPO		= 0x1,
	SYNC_PROJECTS	= 0x2,
	SYNC_ALL		= SYNC_REPO | SYNC_PROJECTS
};


enum RDB_CODES {
	RDB_UPDATE_ID = 1,
	RDB_UPDATE_TEAM = 2
};

//========= Obfuscation	==========//

//An enum won't be expanded by STRINGIZE, so we must use define
#define RDB_ID 				1
#define RDB_team 			2
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
#define RDB_tomes			17
#define RDB_favoris			18

#define RDBS_dataID			19
#define RDBS_dataType		20

#define RDBS_tagCode		21
#define RDBS_tagType		22
#define RDBS_tagID			23


enum SEARCH_REQUEST
{
	INSERT_AUTHOR,
	INSERT_SOURCE,
	INSERT_TAG,
	INSERT_TYPE,
	INSERT_PROJECT,
};

#define RDBS_TYPE_AUTHOR 	1
#define RDBS_TYPE_SOURCE	2
#define RDBS_TYPE_TAG		3
#define RDBS_TYPE_TYPE		4
#define RDBS_TYPE_UNUSED	5

#define RDB_REC_lastRead	1
#define RDB_REC_lastDL		2
#define RDB_REC_team		3
#define RDB_REC_projectID	4

#define DBNAMETOID(s) "`"STRINGIZE(s)"`"