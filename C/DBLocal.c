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

#include "dbCache.h"

#pragma mark - Internal tools



#pragma mark - Public API

//Prevent reusing ID
static uint baseID = 1;

uint getEmptyLocalSlot(PROJECT_DATA project)
{
	char requestString[300];

	bool projectLocal = !project.repo->locale && project.locale;

	//Either we want the main local repo, or the local entries of a standard repo
	snprintf(requestString, sizeof(requestString), "SELECT "DBNAMETOID(RDB_projectID)" FROM "MAIN_CACHE" WHERE "DBNAMETOID(RDB_repo)" = %llu %s ORDER BY "DBNAMETOID(RDB_projectID)" ASC", getRepoID(project.repo), projectLocal ? "AND "DBNAMETOID(RDB_isLocal)" = 1" : "");

	sqlite3_stmt * request = createRequest(cache, requestString);

	if(request == NULL)
		return INVALID_VALUE;

	while(sqlite3_step(request) == SQLITE_ROW)
	{
		uint data = (uint) sqlite3_column_int(request, 0);

		if(data == baseID)
			baseID++;
		else if(data > baseID)
			break;
	}

	destroyRequest(request);
	return baseID;
}

void registerImportEntry(PROJECT_DATA_PARSED project, bool isTome, void* selection)
{
	if(selection == NULL)
		return;

	//Okay, we have quite some work to do
	//First, we need to check if we need to register the project

	char requestString[400];
	bool projectLocal = !project.project.repo->locale && project.project.locale;
	uint64_t repoID = getRepoID(project.project.repo);

	//Does the oroject exist?
	snprintf(requestString, sizeof(requestString), "SELECT "DBNAMETOID(RDB_ID)" FROM "MAIN_CACHE" WHERE "DBNAMETOID(RDB_projectID)" = %d AND "DBNAMETOID(RDB_repo)" = %llu %s ORDER BY "DBNAMETOID(RDB_ID)" ASC", project.project.projectID, repoID, projectLocal ? "AND "DBNAMETOID(RDB_isLocal)" = 1" : "");

	sqlite3_stmt * request = createRequest(cache, requestString);

	if(sqlite3_step(request) == SQLITE_OK)	//No match
		addToCache(NULL, project, repoID, true, false);

	destroyRequest(request);
}