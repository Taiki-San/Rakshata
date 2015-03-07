/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                          **
 **		Source code and assets are property of Taiki, distribution is stricly forbidden		**
 **                                                                                          **
 *********************************************************************************************/

//Public functions for our database

//Image cache suffixes
#define PROJ_IMG_SUFFIX_SRGRID	"GRID"		//Thumbnails in grid mode in SR
#define PROJ_IMG_SUFFIX_HEAD	"HEAD"		//CT Header when focus
#define PROJ_IMG_SUFFIX_CT		"CT"		//CT miniature in the reduced CT tab (reader mode)
#define PROJ_IMG_SUFFIX_DD		"DD"		//Miniature during D&D
#define REPO_IMG_NAME			"REPO"

enum getCopyDBCodes
{
	RDB_LOADALL				= 0x0,
	RDB_LOADINSTALLED		= 0x1,
	RDB_LOADMASK			= 0x1,
	
	//Sorting type
	SORT_NAME				= 0x0,
	SORT_REPO				= 0x2,
	RDB_SORTMASK			= 0x2,
	SORT_DEFAULT			= SORT_NAME,
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

/**DBCache.c**/
uint setupBDDCache();
void syncCacheToDisk(byte syncCode);
void flushDB();

PROJECT_DATA * getCopyCache(uint maskRequest, uint* nbElemCopied);
PROJECT_DATA getCopyOfProjectData(PROJECT_DATA data);
PROJECT_DATA getEmptyProject();

void freeProjectData(PROJECT_DATA* projectDB);

PROJECT_DATA * getDataFromSearch (uint64_t IDRepo, uint projectID, bool installed);
PROJECT_DATA getElementByID(uint cacheID);

bool * getInstalledFromData(PROJECT_DATA * data, uint sizeData);
bool isProjectInstalledInCache (uint ID);

//Repository
void ** getCopyKnownRepo(uint * nbRepo, bool wantRoot);
void removeRepoFromCache(REPO_DATA repo);
void deleteSubRepo(uint64_t repoID);
void freeRootRepo(ROOT_REPO_DATA ** root);
void freeRepo(REPO_DATA ** repos);

uint64_t getRepoID(REPO_DATA * repo);
uint getRootFromRepoID(uint64_t repoID);
uint getSubrepoFromRepoID(uint64_t repoID);
uint64_t getRepoIndexFromURL(char * URL);
REPO_DATA * getRepoForID(uint64_t repoID);
void setUninstalled(bool isRoot, uint repoID);

/**DBRecent.c**/
PROJECT_DATA ** getRecentEntries (bool wantDL, uint8_t * nbElem);
bool addRecentEntry(PROJECT_DATA data, bool wasItADL);

void deleteProject(PROJECT_DATA project, int elemToDel, bool isTome);
void setLastChapitreLu(PROJECT_DATA project, bool isTome, int dernierChapitre);

/**DBRefresh.c**/
void updateDatabase(bool forced);

/**DBSearch.c**/
uint getFromSearch(void * _table, byte type, PROJECT_DATA project);
uint getIDForTag(byte type, uint code);

uint _getFromSearch(void * _table, byte type, void * data);

uint * getFilteredProject(uint * dataLength, const char * searchQuery);
char ** getProjectNameStartingWith(const char * start, uint * nbProject);

/**DBTools.c**/
void resetUpdateDBCache();

bool isPaidProject(PROJECT_DATA projectData);

/**tagManagement.c**/
charType * getTypeForCode(uint32_t tagID);
charType * getTagForCode(uint tagID);
