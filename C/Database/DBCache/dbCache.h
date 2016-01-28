/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
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

extern bool mutexInitialized;
extern MUTEX_VAR cacheMutex, cacheParseMutex;

/**DBCache.c**/
sqlite3_stmt * getAddToCacheRequest(sqlite3 * db);
uint addToCache(sqlite3_stmt* request, PROJECT_DATA_PARSED data, uint64_t repoID, bool isInstalled, bool wantID);
void removeFromCache(PROJECT_DATA_PARSED data);
void consolidateCache();
bool copyOutputDBToStruct(sqlite3_stmt *state, PROJECT_DATA* output, bool copyDynamic, bool evenWantTags);
bool copyParsedDBToStruct(sqlite3_stmt * state, PROJECT_DATA_PARSED * output, bool copyDynamic);

//Repository
ROOT_REPO_DATA ** loadRootRepo(char * repoDB, uint *nbRepo);
REPO_DATA ** loadRepo(ROOT_REPO_DATA ** root, uint nbRoot, uint * nbRepo);
uint getFreeRootRepoID();
void getRidOfDuplicateInRepo(REPO_DATA ** data, uint nbRepo);
void insertRootRepoCache(ROOT_REPO_DATA ** newRoot, uint newRootEntries);
void updateRootRepoCache(ROOT_REPO_DATA ** repoData);
void removeNonInstalledSubRepo(REPO_DATA ** _subRepo, uint nbSubRepo, bool haveExtra);
void getRideOfDuplicateInRootRepo(ROOT_REPO_DATA ** data, uint nbRepo, REPO_DATA *** wantUpdatedRepo, uint *lengthUpdated);
bool isAppropriateNumberOfRepo(uint requestedNumber);

/**DBDebug.c**/
sqlite3_stmt * createRequest(sqlite3 *db, const char *zSql);
int destroyRequest(sqlite3_stmt *pStmt);
#ifdef EXTENSIVE_LOGGING
void errorLogCallback(void *pArg, int iErrCode, const char *zMsg);
#endif

/**DBSearch.c**/
void buildSearchTables(sqlite3 *_cache);
void * buildSearchJumpTable(sqlite3 * _cache);
void flushSearchJumpTable(void * _table);

bool insertInSearch(void * _table, byte type, PROJECT_DATA project);
bool removeFromSearch(void * _table, PROJECT_DATA project);

void checkIfRemainingAndDelete(uint data, byte type);

/**DBRecent.c**/
sqlite3* getPtrRecentDB();
void closeRecentDB(sqlite3 * database);
uint checkRecentDBValid(sqlite3 * DB);

bool updateRecentEntry(sqlite3 *database, PROJECT_DATA data, time_t timestamp, bool wasItADL);
void removeRecentEntry(PROJECT_DATA data);
void removeRecentEntryInternal(uint64_t repoID, uint projectID, bool isLocal);

/**DBRefresh.c**/
void updateRepo();
int getUpdatedProjectOfRepo(char **projectBuf, REPO_DATA* repo);
void * refreshRepoHelper(REPO_DATA * repo, bool standalone);
void updateProjects();

/******		DBTools.c	  ******/
bool parseRemoteRepoEntry(char *data, ROOT_REPO_DATA *previousData, int version, ROOT_REPO_DATA **output);
bool downloadedProjectListSeemsLegit(char *data);
uint defineBoundsRepoOnProjectDB(PROJECT_DATA_PARSED * oldData, uint posBase, uint nbElem);

void applyChangesProject(PROJECT_DATA_PARSED * oldData, uint magnitudeOldData, PROJECT_DATA_PARSED * newData, uint magnitudeNewData);
int createCollate(sqlite3 * database);

/******		DBTags.c	*******/
void initializeTags(void * mainCache);
TAG * duplicateTag(TAG * tagToDuplicate, uint32_t nbTag);

/******		DBLocal.c	*******/
void migrateRemovedInstalledToLocal(PROJECT_DATA_PARSED oldProject, PROJECT_DATA_PARSED * newProject);

//========= Obfuscation	==========//

//An enum won't be expanded by STRINGIZE, so we must use define
#define RDB_ID 						1
#define RDB_repo 					2
#define RDB_projectID				3
#define RDB_isInstalled				4
#define RDB_projectName				5
#define RDB_description				6
#define RDB_authors					7
#define RDB_status					8
#define RDB_category				9
#define RDB_asianOrder				10
#define RDB_isPaid					11
#define RDB_mainTagID				12
#define RDB_tagData					13
#define RDB_nbTagData				14
#define RDB_nbChapter				15
#define RDB_chapitres				16
#define RDB_chapitreRemote			17
#define RDB_chapitreRemoteLength	18
#define RDB_chapitreLocal			19
#define RDB_chapitreLocalLength		20
#define RDB_chapitresPrice			21
#define RDB_nbVolumes				22
#define RDB_DRM						23
#define RDB_tomes					24
#define RDB_tomeRemote				25
#define RDB_tomeRemoteLength		26
#define RDB_tomeLocal				27
#define RDB_tomeLocalLength			28
#define RDB_favoris					29
#define RDB_isLocal					30

#define RDBS_dataID					21
#define RDBS_dataType				22
#define RDBS_tagType				23

#define RDB_tagID					24
#define RDB_tagName					25
#define RDB_CAT_ID					26
#define RDB_CAT_rootID				27
#define RDB_CAT_name				28

#define RDB_REC_lastRead			1
#define RDB_REC_lastDL				2
#define RDB_REC_repo				3
#define RDB_REC_projectID			4
#define RDB_REC_lastIsTome			5
#define RDB_REC_lastCTID			6
#define RDB_REC_lastPage			7
#define RDB_REC_wasLastPageOfCT		8
#define RDB_REC_lastZoom			9
#define RDB_REC_lastScrollerX		10
#define RDB_REC_lastScrollerY		11
#define RDB_REC_lastChange			12

#define DBNAMETOID(s) "`"STRINGIZE(s)"`"

#define MAIN_CACHE "thisWouldBeADumbName"
#define SORT_FUNC "LEGOLAS"

enum SEARCH_REQUEST
{
	INSERT_AUTHOR,
	INSERT_SOURCE,
	INSERT_TAG,
	INSERT_CAT,
	INSERT_PROJECT,
};
