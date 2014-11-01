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

//Image cache suffixes
#define PROJ_IMG_SUFFIX_CT		"CT"		//CT miniature in the reduced CT tab (reader mode)
#define PROJ_IMG_SUFFIX_DD		"DD"		//Miniature during D&D
#define PROJ_IMG_SUFFIX_HEAD	"HEAD"		//CT Header when focus

unsigned long alreadyRefreshed;

/******		DBTools.c	  ******/
bool isRemoteRepoLineValid(char * data, int version);
bool parseRemoteRepoLine(char *data, TEAMS_DATA *previousData, int version, TEAMS_DATA *output);

uint defineBoundsTeamOnProjectDB(PROJECT_DATA * oldData, uint posBase, uint nbElem);
bool downloadedProjectListSeemsLegit(char *data, TEAMS_DATA* team);
uint getNumberLineReturn(char *input);
bool extractCurrentLine(char * input, uint * posInput, char * output, uint lengthOutput);
bool isProjectListSorted(PROJECT_DATA* data, uint length);
void updatePageInfoForProjects(PROJECT_DATA_EXTRA * project, uint nbElem);
void getPageInfo(TEAMS_DATA team, uint projectID, bool large, char * filename);
void applyChangesProject(PROJECT_DATA * oldData, uint magnitudeOldData, PROJECT_DATA * newData, uint magnitudeNewData);

void resetUpdateDBCache();
PROJECT_DATA getCopyOfProjectData(PROJECT_DATA data);
bool isPaidProject(PROJECT_DATA projectData);
bool isInstalled(char * basePath);

/**DBCache.c**/
int setupBDDCache();
void syncCacheToDisk(byte syncCode);
void flushDB();
sqlite3_stmt * getAddToCacheRequest();
bool addToCache(sqlite3_stmt* request, PROJECT_DATA data, uint posTeamIndex, bool isInstalled);
bool updateCache(PROJECT_DATA data, char whatCanIUse, uint projectID);
void removeFromCache(PROJECT_DATA data);
void consolidateCache();
bool copyOutputDBToStruct(sqlite3_stmt *state, PROJECT_DATA* output);
PROJECT_DATA * getCopyCache(uint maskRequest, uint* nbElemCopied);
bool isProjectUpdated(uint ID, byte context);
void setProjectUpdated(uint ID);
void signalProjectRefreshed(uint ID, short context);
bool updateIfRequired(PROJECT_DATA *data, short context);

//Teams
uint getDBTeamID(TEAMS_DATA * team);
uint getTeamID(TEAMS_DATA * team);
bool addRepoToDB(TEAMS_DATA newTeam);
TEAMS_DATA ** loadTeams(char * repoDB, uint *nbTeam);
TEAMS_DATA ** getCopyKnownTeams(uint *nbTeamToRefresh);
void freeTeam(TEAMS_DATA **data);
void getRideOfDuplicateInTeam(TEAMS_DATA ** data, uint *nombreTeam);
int getIndexOfTeam(char * URL);
void updateTeamCache(TEAMS_DATA ** teamData, uint newAmountOfTeam);

void freeProjectData(PROJECT_DATA* projectDB);

//Searches
PROJECT_DATA * getDataFromSearch (uint IDTeam, uint projectID, uint32_t context, bool installed);
void * getUpdatedCTForID(uint cacheID, bool wantTome, size_t * nbElemUpdated, uint ** price);
bool * getInstalledFromData(PROJECT_DATA * data, uint sizeData);
bool isProjectInstalledInCache (uint ID);
PROJECT_DATA getElementByID(uint cacheID, uint32_t context);
void updateTomeDetails(uint cacheID, uint nbTomes, META_TOME* tomeData);
void setInstalled(uint cacheID);

/**DBRefresh.c**/
void updateDatabase(bool forced);
void resetUpdateDBCache();
int getUpdatedRepo(char *buffer_repo, uint bufferSize, TEAMS_DATA teams);
void updateRepo();
int getUpdatedProjectOfTeam(char *projectBuf, TEAMS_DATA* teams);
void updateProjectsFromTeam(PROJECT_DATA* oldData, uint posBase, uint posEnd);
void updateProjects();
void deleteProject(PROJECT_DATA project, int elemToDel, bool isTome);
void setLastChapitreLu(PROJECT_DATA project, bool isTome, int dernierChapitre);
int databaseVersion(char* projectDB);

/**DBRecent.c**/
sqlite3* getPtrRecentDB();
bool checkRecentDBValid(sqlite3 * DB);

bool addRecentEntry(PROJECT_DATA data, bool wasItADL);
bool updateRecentEntry(PROJECT_DATA data, time_t timestamp, bool wasItADL);
void removeRecentEntry(PROJECT_DATA data);
void removeRecentEntryInternal(char * URLRepo, uint projectID);
PROJECT_DATA ** getRecentEntries (bool wantDL, uint8_t * nbElem);

/*Database*/

enum getCopyDBCodes
{
	RDB_LOADALL				= 0x0,
	RDB_LOADINSTALLED		= 0x1,
	RDB_LOADMASK			= 0x1,
	
	//Sorting type
	SORT_NAME				= 0x0,
	SORT_TEAM				= 0x2,
	RDB_SORTMASK			= 0x2,
	SORT_DEFAULT			= SORT_NAME,
		
	//Contexts
	RDB_CTXSERIES			= 0x100,
	RDB_CTXCT				= 0x200,
	RDB_CTXLECTEUR			= 0x400,
	RDB_CTXMDL				= 0x800,
	RDB_CTXSELMDL			= 0x1000,
	RDB_CTXFAVS				= 0x1000,
	RDB_UNUSED2				= 0x4000,
	RDB_UNUSED3				= 0x8000,
	RDB_CTXMASK				= 0xff00
};

enum syncCode
{
	SYNC_TEAM		= 0x1,
	SYNC_PROJECTS	= 0x2,
	SYNC_ALL		= SYNC_TEAM | SYNC_PROJECTS
};


enum RDB_CODES {
	RDB_UPDATE_ID = 1,
	RDB_UPDATE_TEAM = 2
};

//========= Obfuscation	==========//

enum
{
	RDB_ID = 1,
	RDB_team,
	RDB_projectID,
	RDB_isInstalled,
	RDB_projectName,
	RDB_description,
	RDB_authors,
	RDB_status,
	RDB_type,
	RDB_asianOrder,
	RDB_isPaid,
	RDB_category,
	RDB_nombreChapitre,
	RDB_chapitres,
	RDB_chapitresPrice,
	RDB_nombreTomes,
	RDB_tomes,
	RDB_favoris
};

#define RDB_REC_lastRead				1
#define RDB_REC_lastDL					2
#define RDB_REC_team					3
#define RDB_REC_projectID			4

#define DBNAMETOID(s) STRINGIZE(s)