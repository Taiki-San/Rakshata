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
#define PROJ_IMG_SUFFIX_CT		"CT"		//CT miniature in the reduced CT tab (reader mode)
#define PROJ_IMG_SUFFIX_DD		"DD"		//Miniature during D&D
#define PROJ_IMG_SUFFIX_HEAD	"HEAD"		//CT Header when focus
#define PROJ_IMG_SUFFIX_SRGRID	"GRID"		//Thumbnails in grid mode in SR

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

/**DBCache.c**/
PROJECT_DATA * getCopyCache(uint maskRequest, uint* nbElemCopied);
PROJECT_DATA getCopyOfProjectData(PROJECT_DATA data);

void freeProjectData(PROJECT_DATA* projectDB);

PROJECT_DATA * getDataFromSearch (uint IDRepo, uint projectID, bool installed);
PROJECT_DATA getElementByID(uint cacheID);

bool * getInstalledFromData(PROJECT_DATA * data, uint sizeData);
bool isProjectInstalledInCache (uint ID);

//Repository
uint64_t getRepoID(REPO_DATA * repo);
uint getRepoIndexFromURL(char * URL);

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

uint * getFilteredProject(uint * dataLength, const char * searchQuery);
char ** getProjectNameStartingWith(const char * start, uint * nbProject);

/**DBTools.c**/
bool isPaidProject(PROJECT_DATA projectData);

/**tagManagement.c**/
charType * getTagForCode(uint tagID);
