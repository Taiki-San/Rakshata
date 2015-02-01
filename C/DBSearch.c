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

#include "db.h"

extern sqlite3 *cache;
static bool initialized = false;

typedef struct buildTablePointer
{
	sqlite3_stmt * addAuthor;
	sqlite3_stmt * addTag;
	sqlite3_stmt * addType;
	sqlite3_stmt * getAuthorID;
	sqlite3_stmt * getTagID;
	sqlite3_stmt * getTypeID;
	sqlite3_stmt * addProject;
	
} * SEARCH_JUMPTABLE;

#define TABLE_NAME_AUTHOR	"rakSearch1"
#define TABLE_NAME_TAG		"rakSearch2"
#define TABLE_NAME_CORRES	"rakSearch3"

#define RDBS_TYPE_AUTHOR 	1
#define RDBS_TYPE_TAG		3
#define RDBS_TYPE_TYPE		3
#define RDBS_TYPE_SOURCE	4

void buildSearchTables(sqlite3 *_cache)
{
	if(_cache == NULL)
		return;
	
	sqlite3_stmt* request = NULL;
	
	if(sqlite3_prepare_v2(_cache, "CREATE TABLE "TABLE_NAME_AUTHOR" ("DBNAMETOID(RDB_authors)" BLOB UNIQUE ON CONFLICT FAIL, "DBNAMETOID(RDB_ID)" INTEGER PRIMARY KEY AUTOINCREMENT);", -1, &request, NULL) != SQLITE_OK || sqlite3_step(request) != SQLITE_DONE)
	{
		initialized = false;
		sqlite3_finalize(request);
		return;
	}
	
	sqlite3_finalize(request);
	
	if(sqlite3_prepare_v2(_cache, "CREATE TABLE "TABLE_NAME_TAG"("DBNAMETOID(RDB_ID)" INTEGER PRIMARY KEY AUTOINCREMENT, "DBNAMETOID(RDBS_tagCode)" INTEGER NOT NULL, "DBNAMETOID(RDBS_tagType)" INTEGER NOT NULL);", -1, &request, NULL) != SQLITE_OK || sqlite3_step(request) != SQLITE_DONE)
	{
		initialized = false;
		sqlite3_finalize(request);
		return;
	}

	sqlite3_finalize(request);

	if(sqlite3_prepare_v2(_cache, "CREATE TABLE "TABLE_NAME_CORRES" ("DBNAMETOID(RDB_ID)" INTEGER NOT NULL, "DBNAMETOID(RDBS_dataID)" INTEGER NOT NULL, "DBNAMETOID(RDBS_dataType)" INTEGER NOT NULL);", -1, &request, NULL) != SQLITE_OK || sqlite3_step(request) != SQLITE_DONE)
	{
		initialized = false;
		sqlite3_finalize(request);
		return;
	}

	initialized = true;
	sqlite3_finalize(request);
}

void * buildSearchJumpTable(sqlite3 * _cache)
{
	if(!initialized)
		return NULL;
	
	byte stage = 0;
	SEARCH_JUMPTABLE output = malloc(sizeof(struct buildTablePointer));
	
	if(output == NULL)
		return NULL;
	
	if(sqlite3_prepare_v2(_cache, "INSERT INTO "TABLE_NAME_AUTHOR"("DBNAMETOID(RDB_authors)") values(?1);", -1, &(output->addAuthor), NULL) != SQLITE_OK)
		goto fail;
	else
		stage++;
	
	if(sqlite3_prepare_v2(_cache, "INSERT INTO "TABLE_NAME_TAG"("DBNAMETOID(RDBS_tagCode)", "DBNAMETOID(RDBS_tagType)") values(?1, "STRINGIZE(RDBS_TYPE_TAG)");", -1, &(output->addTag), NULL) != SQLITE_OK)
		goto fail;
	else
		stage++;
	
	if(sqlite3_prepare_v2(_cache, "INSERT INTO "TABLE_NAME_TAG"("DBNAMETOID(RDBS_tagCode)", "DBNAMETOID(RDBS_tagType)") values(?1, "STRINGIZE(RDBS_TYPE_TYPE)");", -1, &(output->addType), NULL) != SQLITE_OK)
		goto fail;
	else
		stage++;
	
	if(sqlite3_prepare_v2(_cache, "SELECT "DBNAMETOID(RDB_ID)" FROM "TABLE_NAME_AUTHOR" WHERE "DBNAMETOID(RDB_authors)" = ?1;", -1, &(output->getAuthorID), NULL) != SQLITE_OK)
		goto fail;
	else
		stage++;
	
	if(sqlite3_prepare_v2(_cache, "SELECT "DBNAMETOID(RDB_ID)" FROM "TABLE_NAME_TAG" WHERE "DBNAMETOID(RDBS_tagType)" = "STRINGIZE(RDBS_TYPE_TAG)" AND "DBNAMETOID(RDBS_tagCode)" = ?1;", -1, &(output->getTagID), NULL) != SQLITE_OK)
		goto fail;
	else
		stage++;
	
	if(sqlite3_prepare_v2(_cache, "SELECT "DBNAMETOID(RDB_ID)" FROM "TABLE_NAME_TAG" WHERE "DBNAMETOID(RDBS_tagType)" = "STRINGIZE(RDBS_TYPE_TYPE)" AND "DBNAMETOID(RDBS_tagCode)" = ?1;", -1, &(output->getTypeID), NULL) != SQLITE_OK)
		goto fail;
	else
		stage++;
	
	if(sqlite3_prepare_v2(_cache, "INSERT INTO "TABLE_NAME_CORRES" ("DBNAMETOID(RDB_ID)", "DBNAMETOID(RDBS_dataID)", "DBNAMETOID(RDBS_dataType)") values(?1, ?2, ?3);", -1, &(output->addProject), NULL) != SQLITE_OK)
		goto fail;
	else
		stage++;
	
	if(0)
	{
fail:
		if(stage > 0)	sqlite3_finalize(output->addAuthor);
		if(stage > 1)	sqlite3_finalize(output->addTag);
		if(stage > 2)	sqlite3_finalize(output->addType);
		if(stage > 3)	sqlite3_finalize(output->getAuthorID);
		if(stage > 4)	sqlite3_finalize(output->getTagID);
		if(stage > 5)	sqlite3_finalize(output->getTypeID);
		
		free(output);
		output = NULL;
	}
	
	return output;
}

uint getFromSearch(void * _table, byte type, PROJECT_DATA project)
{
	if(_table == NULL)
		return UINT_MAX;
	
	SEARCH_JUMPTABLE table = _table;
	sqlite3_stmt * request = NULL;
	
	switch (type)
	{
		case PULL_SEARCH_AUTHORID:
		{
			request = table->getAuthorID;
			sqlite3_bind_blob(request, 1, project.authorName, sizeof(project.authorName), SQLITE_STATIC);
			break;
		}
			
		case PULL_SEARCH_TAGID:
		{
			request = table->getTagID;
			sqlite3_bind_int64(request, 1, project.tag);
			break;
		}
			
		case PULL_SEARCH_TYPEID:
		{
			request = table->getTypeID;
			sqlite3_bind_int64(request, 1, project.type);
			break;
		}
			
		default:
		{
			return UINT_MAX;
		}
	}
	
	uint output = UINT_MAX;
	
	if((output = sqlite3_step(request)) == SQLITE_ROW)
		output = sqlite3_column_int(request, 0);
	else
		output = UINT_MAX;
	
	sqlite3_reset(request);
	
	return output;
}

bool insertInSearch(void * _table, byte type, PROJECT_DATA project)
{
	if(_table == NULL)
		return false;
	
	SEARCH_JUMPTABLE table = _table;	
	sqlite3_stmt * request = NULL;
	
	switch (type)
	{
		case INSERT_AUTHOR:
		{
			request = table->addAuthor;
			sqlite3_bind_blob(request, 1, project.authorName, sizeof(project.authorName), SQLITE_STATIC);
			break;
		}
			
		case INSERT_TAG:
		{
			request = table->addTag;
			sqlite3_bind_int64(request, 1, project.tag);
			break;
		}
			
		case INSERT_TYPE:
		{
			request = table->addType;
			sqlite3_bind_int64(request, 1, project.type);
			break;
		}
			
		case INSERT_PROJECT:
		{
			request = table->addProject;
			
			bool fail = false;
			uint typeID = getFromSearch(table, PULL_SEARCH_TYPEID, project), tagID = getFromSearch(table, PULL_SEARCH_TAGID, project), authorID = getFromSearch(table, PULL_SEARCH_AUTHORID, project);
			
			if(typeID == UINT_MAX && insertInSearch(_table, INSERT_TYPE, project))
				typeID = getFromSearch(table, PULL_SEARCH_TYPEID, project);

			if(tagID == UINT_MAX && insertInSearch(_table, INSERT_TAG, project))
				tagID = getFromSearch(table, PULL_SEARCH_TAGID, project);
			
			if(authorID == UINT_MAX && insertInSearch(_table, INSERT_AUTHOR, project))
				authorID = getFromSearch(table, PULL_SEARCH_AUTHORID, project);
			
			if(typeID == UINT_MAX || tagID == UINT_MAX || authorID == UINT_MAX)
				return false;
			
			sqlite3_bind_int(request, 1, project.projectID);
			sqlite3_bind_int(request, 2, authorID);
			sqlite3_bind_int(request, 3, RDBS_TYPE_AUTHOR);
			
			fail = sqlite3_step(request) != SQLITE_DONE;
			sqlite3_reset(request);
			
			if(fail)
				return false;

			sqlite3_bind_int(request, 1, project.projectID);
			sqlite3_bind_int(request, 2, typeID);
			sqlite3_bind_int(request, 3, RDBS_TYPE_TYPE);
			
			fail = sqlite3_step(request) != SQLITE_DONE;
			sqlite3_reset(request);
			
			if(fail)
				return false;
			
			sqlite3_bind_int(request, 1, project.projectID);
			sqlite3_bind_int(request, 2, tagID);
			sqlite3_bind_int(request, 3, RDBS_TYPE_TAG);
			break;
		}
			
		default:
		{
			return false;
		}
	}
	
	bool output = sqlite3_step(request) == SQLITE_DONE;
	
	sqlite3_reset(request);
	
	return output;
}