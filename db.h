/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite  **
 **                                                                                         **
 *********************************************************************************************/

#include "sqlite3.h"

#define INITIAL_BUFFER_SIZE			1024
#define MAX_PROJECT_LINE_LENGTH		(LONGUEUR_NOM_MANGA_MAX + LONGUEUR_COURT + 7 * 11 + 30)		//7 nombres * 10 digits + 30 pour de la marge, les espace, toussa
#define MAX_TEAM_LINE_LENGTH		(LONGUEUR_NOM_MANGA_MAX + LONGUEUR_COURT + LONGUEUR_TYPE_TEAM + LONGUEUR_URL + LONGUEUR_SITE + 25)
#define DB_CACHE_EXPIRENCY			5*60*1000	//5 minutes

//Options
#define KEEP_UNUSED_TEAMS				//If droped, they won't be refreshed, nor their manga DB will be updated, so bad idea for now
//#define DELETE_UNLISTED_PROJECT

unsigned long alreadyRefreshed;

/******		DBTools.c	  ******/
bool isRemoteRepoLineValid(char * data, int version);
bool parseRemoteRepoLine(char *data, TEAMS_DATA *previousData, int version, TEAMS_DATA *output);

uint defineBoundsTeamOnProjectDB(MANGAS_DATA * oldData, uint posBase, uint nbElem);
bool downloadedProjectListSeemsLegit(char *data, TEAMS_DATA* team);
uint getNumberLineReturn(char *input);
bool extractCurrentLine(char * input, uint * posInput, char * output, uint lengthOutput);
bool parseCurrentProjectLine(char * input, int version, MANGAS_DATA * output);
void parseDetailsBlock(char * input, MANGAS_DATA *data, char *teamName, uint lengthOfBlock);
bool isProjectListSorted(MANGAS_DATA* data, uint length);
void applyChangesProject(MANGAS_DATA * oldData, uint magnitudeOldData, MANGAS_DATA * newData, uint magnitudeNewData);

void resetUpdateDBCache();

/**DBCache.c**/
int setupBDDCache();
void syncCacheToDisk(byte syncCode);
void flushDB();
sqlite3_stmt * getAddToCacheRequest();
bool addToCache(sqlite3_stmt* request, MANGAS_DATA data, uint posTeamIndex, bool isInstalled);
bool updateCache(MANGAS_DATA data, char whatCanIUse, char * mangaNameShort);
void removeFromCache(MANGAS_DATA data);
void consolidateCache();
bool copyOutputDBToStruct(sqlite3_stmt *state, bool dropChaptersAndTomes, MANGAS_DATA* output);
MANGAS_DATA * getCopyCache(uint maskRequest, uint* nbElemCopied);
bool isProjectUpdated(uint ID, byte context);
void setProjectUpdated(uint ID);
void signalProjectRefreshed(uint ID, short context);
void updateIfRequired(MANGAS_DATA *data, short context);

//Teams
uint getDBTeamID(TEAMS_DATA * team);
bool addRepoToDB(TEAMS_DATA newTeam);
TEAMS_DATA ** getCopyKnownTeams(uint *nbTeamToRefresh);
void freeTeam(TEAMS_DATA **data);
void getRideOfDuplicateInTeam(TEAMS_DATA ** data, uint *nombreTeam);
void updateTeamCache(TEAMS_DATA ** teamData, uint newAmountOfTeam);

void freeMangaData(MANGAS_DATA* mangaDB);

void teamOfProject(char nomProjet[LONGUEUR_NOM_MANGA_MAX], char nomTeam[LONGUEUR_NOM_MANGA_MAX]);

/**DBRefresh.c**/
void updateDatabase(bool forced);
void resetUpdateDBCache();
int getUpdatedRepo(char *buffer_repo, TEAMS_DATA* teams);
void updateRepo();
int getUpdatedProjectOfTeam(char *buffer_manga, TEAMS_DATA* teams);
void updateProjectsFromTeam(MANGAS_DATA* oldData, uint posBase, uint posEnd, bool updateDB);
void updateProjects();
int deleteManga();
void setLastChapitreLu(MANGAS_DATA mangasDB, bool isTome, int dernierChapitre);
int databaseVersion(char* mangaDB);


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
	
	//Copy chapters to output struct?
	RDB_NOCTCOPY			= 0x4,
	
	//Contexts
	RDB_CTXSERIES			= 0x100,
	RDB_CTXCT				= 0x200,
	RDB_CTXLECTEUR			= 0x400,
	RDB_CTXMDL				= 0x800,
	RDB_CTXDEL				= 0x1000,
	RDB_CTXFAVS				= 0x1000,
	RDB_CTXSELMDL			= 0x4000,
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

#define RDB_ID							1
#define RDB_team						2
#define RDB_mangaNameShort				3
#define RDB_isInstalled					4
#define RDB_mangaName					5
#define RDB_status						6
#define RDB_genre						7
#define RDB_pageInfos					8
#define RDB_firstChapter				9
#define RDB_lastChapter					10
#define RDB_nombreChapitreSpeciaux		11
#define RDB_nombreChapitre				12
#define RDB_chapitres					13
#define RDB_firstTome					14
#define RDB_nombreTomes					15
#define RDB_tomes						16
#define RDB_contentDownloadable			17
#define RDB_favoris						18

#define STRINGIZE2(s) "`"#s"`"
#define DBNAMETOID(s) STRINGIZE2(s)