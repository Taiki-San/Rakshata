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

#define DB_CACHE_EXPIRENCY			5*60*1000	//5 minutes

unsigned long alreadyRefreshed;

enum getCopyDBCodes
{
	RDB_LOADALL				= 0x0,
	RDB_LOADINSTALLED		= 0x1,
	RDB_LOADMASK			= 0x1,
	
	//Sorting type
	SORT_NAME				= 0x0,
	SORT_REPO				= 0x2,
	SORT_ID					= 0x4,
	RDB_SORTMASK			= 0x6,
	SORT_DEFAULT			= SORT_ID,
};

enum
{
	PULL_SEARCH_AUTHORID = 10,
	PULL_SEARCH_TAGID,
	PULL_SEARCH_TYPEID
};

enum syncCode
{
	SYNC_REPO		= 0x1,
	SYNC_PROJECTS	= 0x2,
	SYNC_ALL		= SYNC_REPO | SYNC_PROJECTS
};

enum RDB_CODES
{
	RDB_UPDATE_ID = 1,
	RDB_UPDATE_TEAM = 2
};

#define RDBS_TYPE_AUTHOR 	1
#define RDBS_TYPE_SOURCE	2
#define RDBS_TYPE_TAG		3
#define RDBS_TYPE_CAT		4
#define RDBS_TYPE_UNUSED	5

//Image cache suffixes
#define PROJ_IMG_SUFFIX_SRGRID	"GRID"		//Thumbnails in grid mode in SR
#define PROJ_IMG_SUFFIX_HEAD	"HEAD"		//CT Header when focus
#define PROJ_IMG_SUFFIX_CT		"CT"		//CT miniature in the reduced CT tab (reader mode)
#define PROJ_IMG_SUFFIX_DD		"DD"		//Miniature during D&D
#define REPO_IMG_NAME			"REPO"

//Structure to update icons
typedef struct icon_update_waitlist ICONS_UPDATE;

struct icon_update_waitlist
{
	char * URL;
	char * filename;
	
	ICONS_UPDATE * next;
};

/**DBCache.c**/
uint setupBDDCache();
void syncCacheToDisk(byte syncCode);
void flushDB();

PROJECT_DATA * getCopyCache(uint maskRequest, uint* nbElemCopied);
PROJECT_DATA getCopyOfProjectData(PROJECT_DATA data);
PROJECT_DATA getEmptyProject();
bool updateCache(PROJECT_DATA data, char whatCanIUse, uint projectID);

void freeProjectData(PROJECT_DATA* projectDB);

PROJECT_DATA * getDataFromSearch (uint64_t IDRepo, uint projectID, bool installed);
PROJECT_DATA getElementByID(uint cacheID);

bool * getInstalledFromData(PROJECT_DATA * data, uint sizeData);
bool isProjectInstalledInCache (uint ID);

//Repository
void addRootRepoToDB(ROOT_REPO_DATA ** newRepo, const uint nbRoot);
void ** getCopyKnownRepo(uint * nbRepo, bool wantRoot);
void activateRepo(REPO_DATA repo);
void removeRepoFromCache(REPO_DATA repo);
void deleteSubRepo(uint64_t repoID);
void freeRootRepo(ROOT_REPO_DATA ** root);
void freeSingleRootRepo(ROOT_REPO_DATA * root);
void freeRepo(REPO_DATA ** repos);

uint64_t getRepoID(REPO_DATA * repo);
uint getRootFromRepoID(uint64_t repoID);
uint getSubrepoFromRepoID(uint64_t repoID);
uint64_t getRepoIndexFromURL(char * URL);
REPO_DATA * getRepoForID(uint64_t repoID);
void setUninstalled(bool isRoot, uint repoID);

//Searches
void * getUpdatedCTForID(uint cacheID, bool wantTome, size_t * nbElemUpdated, uint ** price);
void setInstalled(uint cacheID);

/**DBSearch.c**/
bool getProjectSearchData(void * table, uint cacheID, uint * authorID, uint * tagID, uint * typeID);
uint64_t * getSearchData(byte type, charType *** dataName, uint * dataLength);
bool updateProjectSearch(void * _table, PROJECT_DATA project);

bool insertRestriction(uint64_t code, byte type);
bool removeRestriction(uint64_t code, byte type);

uint getFromSearch(void * _table, byte type, PROJECT_DATA project);
uint getIDForTag(byte type, uint code);

uint _getFromSearch(void * _table, byte type, void * data);

uint * getFilteredProject(uint * dataLength, const char * searchQuery);
char ** getProjectNameStartingWith(const char * start, uint * nbProject);

/**DBRecent.c**/
void flushRecentMutex();

PROJECT_DATA ** getRecentEntries (bool wantDL, uint8_t * nbElem);
bool addRecentEntry(PROJECT_DATA data, bool wasItADL);

void deleteProject(PROJECT_DATA project, int elemToDel, bool isTome);
void setLastChapitreLu(PROJECT_DATA project, bool isTome, int dernierChapitre);

/**DBRefresh.c**/
void updateDatabase(bool forced);
int getUpdatedRepo(char **buffer_repo, size_t * bufferSize, ROOT_REPO_DATA repo);
void * enforceRepoExtra(ROOT_REPO_DATA * root, bool getRidOfThemAfterward);
void * updateProjectsFromRepo(PROJECT_DATA* oldData, uint posBase, uint posEnd, bool standalone);

/******		DBTools.c	  ******/
bool parseRemoteRootRepo(char * data, int version, ROOT_REPO_DATA ** output);
void updateProjectImages(void * _todo);
bool isInstalled(char * basePath);
void resetUpdateDBCache();
bool isPaidProject(PROJECT_DATA projectData);
uint getNumberInstalledProjectForRepo(bool isRoot, void * repo);

/**tagManagement.c**/
charType * getTypeForCode(uint32_t tagID);
charType * getTagForCode(uint tagID);