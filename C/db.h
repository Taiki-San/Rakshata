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

int64_t alreadyRefreshed;

//Repo ID are the combinaison of two 32 bits integer, thus, it can not be below 2^32 + 1 so, we're safe
#define LOCAL_REPO_ID 42

enum getCopyDBCodes
{
	//Type of data
	RDB_LOADALL				= 0x0,
	RDB_LOADINSTALLED		= 0x1,
	RDB_LOAD_FAVORITE		= 0x2,
	
	//Sorting type
	SORT_NAME				= 0x0,
	SORT_REPO				= 0x4,
	SORT_ID					= 0x8,
	SORT_DEFAULT			= SORT_ID,
	
	//Amount of detail
	RDB_COPY_ALL			= 0x0,
	RDB_EXCLUDE_DYNAMIC		= 0x10,

	//Want only remote or also the locally imported data
	RDB_INCLUDE_LOCAL		= 0x0,
	RDB_REMOTE_ONLY			= 0x20,
	RDB_PARSED_OUTPUT		= 0x40,
	RDB_INCLUDE_TAGS		= 0x80
};

enum
{
	PULL_SEARCH_AUTHORID = 10,
	PULL_SEARCH_TAGID,
	PULL_SEARCH_CATID
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
#define IMAGE_CACHE_DIR 		"imageCache"
#define PROJ_IMG_SUFFIX_SRGRID	"GRID"		//Thumbnails in grid mode in SR
#define PROJ_IMG_SUFFIX_HEAD	"HEAD"		//CT Header when focus
#define PROJ_IMG_SUFFIX_CT		"CT"		//CT miniature in the reduced CT tab (reader mode)
#define PROJ_IMG_SUFFIX_DD		"DD"		//Miniature during D&D
#define REPO_IMG_NAME			"REPO"

#define NOTIFICATION_THUMBNAIL_UPDATE_SRGRID @"RakNotificationNewThumbnailsYummyYummy1"
#define NOTIFICATION_THUMBNAIL_UPDATE_HEAD @"RakNotificationNewThumbnailsYummyYummy2"
#define NOTIFICATION_THUMBNAIL_UPDATE_CT @"RakNotificationNewThumbnailsYummyYummy3"

enum
{
	THUMBID_SRGRID,
	THUMBID_HEAD,
	THUMBID_CT,
	THUMBID_DD
};

//Structure to update icons
typedef struct icon_update_waitlist ICONS_UPDATE;

struct icon_update_waitlist
{
	char * URL;
	char * filename;

	uint64_t repoID;

	char crc32[9];
	byte updateType;
	bool isRetina;

	uint projectID;
	
	ICONS_UPDATE * next;
};

/**DBCache.c**/
uint setupBDDCache();
void syncCacheToDisk(byte syncCode);
void flushDB();

void * getCopyCache(uint maskRequest, uint* nbElemCopied);
PROJECT_DATA getCopyOfProjectData(PROJECT_DATA data);
PROJECT_DATA getEmptyProject();
PROJECT_DATA_PARSED getEmptyParsedProject();
PROJECT_DATA_EXTRA getEmptyExtraProject();
REPO_DATA getEmptyRepo();
bool updateCache(PROJECT_DATA_PARSED data, char whatCanIUse, uint projectID);

void freeParseProjectData(PROJECT_DATA_PARSED * projectDB);
void freeProjectData(PROJECT_DATA* projectDB);

void * _getProjectFromSearch (uint64_t IDRepo, uint projectID, bool locale, bool installed, bool copyDynamic, bool wantParsed, bool wantTags);
PROJECT_DATA * getProjectFromSearch (uint64_t IDRepo, uint projectID, bool locale, bool installed);
PROJECT_DATA_PARSED getProjectByIDHelper(uint cacheID, bool copyDynamic, bool wantParsed, bool wantTags);
PROJECT_DATA_PARSED getParsedProjectByID(uint cacheID);
PROJECT_DATA getProjectByID(uint cacheID);

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
void _freeSingleRootRepo(ROOT_REPO_DATA * root, bool releaseMemory);
void freeRepo(REPO_DATA ** repos);

uint64_t getRepoID(REPO_DATA * repo);
uint getRootFromRepoID(uint64_t repoID);
uint getSubrepoFromRepoID(uint64_t repoID);
uint64_t getRepoIndexFromURL(const char * URL);
REPO_DATA * getRepoForID(uint64_t repoID);
ROOT_REPO_DATA * getRootRepoForID(uint repoID);
void setFavoriteForID(uint cacheID, bool isFavorite);
bool copyRootRepo(const ROOT_REPO_DATA original, ROOT_REPO_DATA * copy);
void setUninstalled(bool isRoot, uint64_t repoID);

//Searches
void * getUpdatedCTForID(uint cacheID, bool wantTome, size_t * nbElemUpdated, uint ** price);
void setInstalled(uint cacheID);
uint * getFavoritesID(uint * nbFavorites);

/**DBSearch.c**/
bool getProjectSearchData(void * table, uint cacheID, uint * authorID, uint * tagID, uint * typeID);
uint64_t * getSearchData(byte type, charType *** dataName, uint * dataLength);
bool updateProjectSearch(void * _table, PROJECT_DATA project);

bool insertRestriction(uint64_t code, byte type);
bool removeRestriction(uint64_t code, byte type);
bool flushRestriction();

uint getFromSearch(void * _table, byte type, PROJECT_DATA project);
uint getIDForTag(byte type, uint code);

uint _getFromSearch(void * _table, byte type, void * data);

uint * getFilteredProject(uint * dataLength, const char * searchQuery, bool wantInstalledOnly, bool wantFreeOnly);
char ** getProjectNameStartingWith(const char * start, uint * nbProject);

/**DBRecent.c**/
void flushRecentMutex();

PROJECT_DATA ** getRecentEntries (bool wantDL, uint8_t * nbElem);
bool addRecentEntry(PROJECT_DATA data, bool wasItADL);

void deleteProject(PROJECT_DATA project, int elemToDel, bool isTome);
void setLastChapitreLu(PROJECT_DATA project, bool isTome, int dernierChapitre);

/**DBRefresh.c**/
void updateDatabase(bool forced);
void refreshRepo(REPO_DATA * repo);
int getUpdatedRepo(char **buffer_repo, size_t * bufferSize, ROOT_REPO_DATA repo);
void * enforceRepoExtra(ROOT_REPO_DATA * root, bool getRidOfThemAfterward);
void * updateProjectsFromRepo(PROJECT_DATA_PARSED* oldData, uint posBase, uint posEnd, bool standalone);

/******		DBTools.c	  ******/
bool parseRemoteRootRepo(char * data, int version, ROOT_REPO_DATA ** output);
void updateProjectImages(void * _todo);
bool isInstalled(PROJECT_DATA project, char * basePath);
void resetUpdateDBCache();
bool isPaidProject(PROJECT_DATA projectData);
uint getNumberInstalledProjectForRepo(bool isRoot, void * repo);
void * generateIconUpdateWorkload(PROJECT_DATA_EXTRA * project, uint nbElem);
charType * getStringFromUTF8(const unsigned char * rawString);

/******		DBLocal.c		*******/
uint getEmptyLocalSlot(PROJECT_DATA project);
void registerImportEntry(PROJECT_DATA_PARSED project, bool isTome);

/**tagManagement.c**/
uint getRootCategoryIDForID(uint32_t categoryID);

charType * getCatNameForCode(uint32_t catID);
charType * getTagNameForCode(uint tagID);

bool doesCatOfIDExist(uint32_t catID);
bool doesTagOfIDExist(uint32_t tagID);

bool getCopyOfTags(TAG_VERBOSE ** newData, uint * nbData);
bool getCopyOfCats(CATEGORY_VERBOSE ** newData, uint * nbData);

/**		TagUpdate.c		**/
void checkIfRefreshTag();
