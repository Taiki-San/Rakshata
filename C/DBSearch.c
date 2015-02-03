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

static uint nbAuthor = 0, nbTag = 0, nbType = 0;

typedef struct buildTablePointer
{
	sqlite3_stmt * addAuthor;
	sqlite3_stmt * addTag;
	sqlite3_stmt * addType;
	sqlite3_stmt * getAuthorID;
	sqlite3_stmt * getTagID;
	sqlite3_stmt * getTypeID;
	sqlite3_stmt * addProject;
	sqlite3_stmt * updateProject;
	sqlite3_stmt * removeProject;
	sqlite3_stmt * readProject;
	
} * SEARCH_JUMPTABLE;

#define TABLE_NAME_AUTHOR	"rakSearch1"
#define TABLE_NAME_TAG		"rakSearch2"
#define TABLE_NAME_CORRES	"rakSearch3"

bool manipulateProjectSearch(SEARCH_JUMPTABLE table, bool wantInsert, PROJECT_DATA project);
void updateElementCount(byte type, int change);

void buildSearchTables(sqlite3 *_cache)
{
	if(_cache == NULL)
		return;
	
	sqlite3_stmt* request = NULL;
	
	if(sqlite3_prepare_v2(_cache, "CREATE TABLE "TABLE_NAME_AUTHOR" ("DBNAMETOID(RDB_authors)" TEXT UNIQUE ON CONFLICT FAIL, "DBNAMETOID(RDB_ID)" INTEGER PRIMARY KEY AUTOINCREMENT);", -1, &request, NULL) != SQLITE_OK || sqlite3_step(request) != SQLITE_DONE)
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
	if(_cache == NULL)
		_cache = cache;
		
	if(!initialized || _cache == NULL)
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

	if(sqlite3_prepare_v2(_cache, "UPDATE "TABLE_NAME_CORRES" SET "DBNAMETOID(RDBS_dataID)" = ?2 WHERE "DBNAMETOID(RDB_ID)" = ?1 AND "DBNAMETOID(RDBS_dataType)" = ?3;", -1, &(output->updateProject), NULL) != SQLITE_OK)
		goto fail;
	else
		stage++;
	
	if(sqlite3_prepare_v2(_cache, "DELETE FROM "TABLE_NAME_CORRES" WHERE "DBNAMETOID(RDB_ID)" = ?1", -1, &(output->removeProject), NULL) != SQLITE_OK)
		goto fail;
	else
		stage++;
	
	if(sqlite3_prepare_v2(_cache, "SELECT * FROM "TABLE_NAME_CORRES" WHERE "DBNAMETOID(RDB_ID)" = ?1;", -1, &(output->readProject), NULL) != SQLITE_OK)
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
		if(stage > 6)	sqlite3_finalize(output->addProject);
		if(stage > 7)	sqlite3_finalize(output->updateProject);
		if(stage > 8)	sqlite3_finalize(output->removeProject);
		
		free(output);
		output = NULL;
	}
	
	return output;
}

void flushSearchJumpTable(void * _table)
{
	SEARCH_JUMPTABLE table = _table;
	
	if(table == NULL)
		return;
	
	sqlite3_finalize(table->addAuthor);
	sqlite3_finalize(table->addTag);
	sqlite3_finalize(table->addType);
	sqlite3_finalize(table->getAuthorID);
	sqlite3_finalize(table->getTagID);
	sqlite3_finalize(table->getTypeID);
	sqlite3_finalize(table->addProject);
	sqlite3_finalize(table->updateProject);
	sqlite3_finalize(table->removeProject);
	sqlite3_finalize(table->readProject);
	
	free(table);
}

//API to manipulate the content

uint getFromSearch(void * _table, byte type, PROJECT_DATA project)
{
	SEARCH_JUMPTABLE table = _table;

	if(_table == NULL)
	{
		table = buildSearchJumpTable(cache);
		
		if(table == NULL)
			return UINT_MAX;
	}
	
	sqlite3_stmt * request = NULL;
	
	switch (type)
	{
		case PULL_SEARCH_AUTHORID:
		{
			request = table->getAuthorID;
			
			size_t length = wstrlen(project.authorName);
			char utf8[4 * length + 1];
			if(utf8 == NULL)
				return UINT_MAX;
			
			length = wchar_to_utf8(project.authorName, length, utf8, 4 * length + 1, 0);
			
			sqlite3_bind_text(request, 1, utf8, length, SQLITE_TRANSIENT);
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
	
	if(_table == NULL)
		flushSearchJumpTable(table);
	
	return output;
}

bool insertInSearch(void * _table, byte type, PROJECT_DATA project)
{
	SEARCH_JUMPTABLE table = _table;
	
	if(_table == NULL)
	{
		table = buildSearchJumpTable(cache);

		if(table == NULL)
			return false;
	}

	if(type == INSERT_PROJECT)
	{
		bool output = manipulateProjectSearch(table, true, project);
		
		if(_table == NULL)
			flushSearchJumpTable(table);
		
		return output;
	}
	
	sqlite3_stmt * request = NULL;
	byte requestType;
	
	switch (type)
	{
		case INSERT_AUTHOR:
		{
			requestType = RDBS_TYPE_AUTHOR;
			request = table->addAuthor;
			
			size_t length = wstrlen(project.authorName);
			char utf8[4 * length + 1];
			length = wchar_to_utf8(project.authorName, length, utf8, 4 * length + 1, 0);
			
			sqlite3_bind_text(request, 1, utf8, length, SQLITE_TRANSIENT);
			break;
		}
			
		case INSERT_TAG:
		{
			requestType = RDBS_TYPE_TAG;
			request = table->addTag;
			sqlite3_bind_int64(request, 1, project.tag);
			break;
		}
			
		case INSERT_TYPE:
		{
			requestType = RDBS_TYPE_TYPE;
			request = table->addType;
			sqlite3_bind_int64(request, 1, project.type);
			break;
		}
			
		default:
		{
			return false;
		}
	}
	
	bool output = sqlite3_step(request) == SQLITE_DONE;
	
	sqlite3_reset(request);
	
	if(_table == NULL)
		flushSearchJumpTable(table);
	
	if(output)
		updateElementCount(requestType, 1);

	return output;
}

bool removeFromSearch(void * _table, uint cacheID)
{
	SEARCH_JUMPTABLE table = _table;
	
	if(_table == NULL)
	{
		table = buildSearchJumpTable(cache);
		
		if(table == NULL)
			return false;
	}

	sqlite3_stmt * request = table->removeProject;
	sqlite3_bind_int(request, 1, cacheID);

	bool output = sqlite3_step(request) == SQLITE_DONE;
	
	sqlite3_reset(request);
	
	return output;
}

bool updateProjectSearch(void * _table, PROJECT_DATA project)
{
	SEARCH_JUMPTABLE table = _table;
	
	if(_table == NULL)
	{
		table = buildSearchJumpTable(cache);
		
		if(table == NULL)
			return false;
	}

	bool output = manipulateProjectSearch(table, false, project);
	
	if(_table == NULL)
		flushSearchJumpTable(table);

	return output;
}

bool manipulateProjectSearch(SEARCH_JUMPTABLE table, bool wantInsert, PROJECT_DATA project)
{
	uint typeID = getFromSearch(table, PULL_SEARCH_TYPEID, project), tagID = getFromSearch(table, PULL_SEARCH_TAGID, project), authorID = getFromSearch(table, PULL_SEARCH_AUTHORID, project);
	uint oldType, oldTag, oldAuthor;
	
	getProjectSearchData(table, project.cacheDBID, &oldAuthor, &oldTag, &oldType);
	
	if(typeID == UINT_MAX && insertInSearch(table, INSERT_TYPE, project))
		typeID = getFromSearch(table, PULL_SEARCH_TYPEID, project);
	
	if(tagID == UINT_MAX && insertInSearch(table, INSERT_TAG, project))
		tagID = getFromSearch(table, PULL_SEARCH_TAGID, project);
	
	if(authorID == UINT_MAX && insertInSearch(table, INSERT_AUTHOR, project))
		authorID = getFromSearch(table, PULL_SEARCH_AUTHORID, project);
	
	if(typeID == UINT_MAX || tagID == UINT_MAX || authorID == UINT_MAX)
		return false;
	
	bool fail = false;
	sqlite3_stmt * request = wantInsert ? table->addProject : table->updateProject;

	if(authorID != oldAuthor)
	{
		sqlite3_bind_int(request, 1, project.projectID);
		sqlite3_bind_int(request, 2, authorID);
		sqlite3_bind_int(request, 3, RDBS_TYPE_AUTHOR);
		
		fail = sqlite3_step(request) != SQLITE_DONE;
		sqlite3_reset(request);
		
		if(fail)
			return false;

		checkIfRemainingAndDelete(authorID, RDBS_TYPE_AUTHOR);
	}

	if(typeID != oldType)
	{
		sqlite3_bind_int(request, 1, project.projectID);
		sqlite3_bind_int(request, 2, typeID);
		sqlite3_bind_int(request, 3, RDBS_TYPE_TYPE);
		
		fail = sqlite3_step(request) != SQLITE_DONE;
		sqlite3_reset(request);
		
		if(fail)
			return false;
		
		checkIfRemainingAndDelete(typeID, RDBS_TYPE_TYPE);
	}
	
	if(tagID != oldTag)
	{
		sqlite3_bind_int(request, 1, project.projectID);
		sqlite3_bind_int(request, 2, tagID);
		sqlite3_bind_int(request, 3, RDBS_TYPE_TAG);
		
		fail = sqlite3_step(request) != SQLITE_DONE;
		sqlite3_reset(request);
		
		checkIfRemainingAndDelete(tagID, RDBS_TYPE_TAG);
	}

	return !fail;
}

//API to keep things kinda clean

void updateElementCount(byte type, int change)
{
	switch (type)
	{
		case RDBS_TYPE_AUTHOR:
		{
			nbAuthor += change;
			break;
		}
			
		case RDBS_TYPE_TAG:
		{
			nbTag += change;
			break;
		}
			
		case RDBS_TYPE_TYPE:
		{
			nbType += change;
			break;
		}
	}
}

void checkIfRemainingAndDelete(uint data, byte type)
{
	if(cache == NULL || data == UINT_MAX)
		return;
	
	sqlite3_stmt * request;
	
	if(sqlite3_prepare_v2(cache, "SELECT COUNT() FROM "TABLE_NAME_CORRES" WHERE "DBNAMETOID(RDBS_dataID)" = ?1 AND "DBNAMETOID(RDBS_dataType)" = ?2;", -1, &request, NULL) != SQLITE_OK)
		return;
	
	sqlite3_bind_int(request, 1, data);
	sqlite3_bind_int(request, 2, type);
	
	bool nothingRemaining = sqlite3_step(request) == SQLITE_DONE;
	
	sqlite3_finalize(request);
	
	//We have to delete the entry
	if(nothingRemaining)
	{
		if(type == RDBS_TYPE_AUTHOR && sqlite3_prepare_v2(cache, "DELETE FROM "TABLE_NAME_AUTHOR" WHERE "DBNAMETOID(RDB_ID)" = ?1", -1, &request, NULL) == SQLITE_OK);

		else if(type == RDBS_TYPE_TAG && sqlite3_prepare_v2(cache, "DELETE FROM "TABLE_NAME_TAG" WHERE "DBNAMETOID(RDB_ID)" = ?1 AND "DBNAMETOID(RDBS_tagType)" = "STRINGIZE(RDBS_TYPE_TAG)"", -1, &request, NULL) == SQLITE_OK);
		
		else if(type == RDBS_TYPE_TYPE && sqlite3_prepare_v2(cache, "DELETE FROM "TABLE_NAME_TAG" WHERE "DBNAMETOID(RDB_ID)" = ?1 AND "DBNAMETOID(RDBS_tagType)" = "STRINGIZE(RDBS_TYPE_TYPE)"", -1, &request, NULL) == SQLITE_OK);
		
		else
			return;
		
		sqlite3_bind_int(request, 1, data);
		
		if(sqlite3_step(request) == SQLITE_DONE)
			updateElementCount(type, -1);
			
		sqlite3_finalize(request);
	}
}

//API to read and work on those data

bool getProjectSearchData(void * table, uint cacheID, uint * authorID, uint * tagID, uint * typeID)
{
	if(authorID == NULL || tagID == NULL || typeID == NULL)
		return false;
	else
		*authorID = *tagID = *typeID = UINT_MAX;
	
	sqlite3_stmt * request;
	
	if(table != NULL)
		request = ((SEARCH_JUMPTABLE) table)->readProject;
	else
	{
		if(sqlite3_prepare_v2(cache, "SELECT "DBNAMETOID(RDBS_dataID)", "DBNAMETOID(RDBS_dataType)" FROM "TABLE_NAME_CORRES" WHERE "DBNAMETOID(RDB_ID)" = ?1;", -1, &request, NULL) != SQLITE_OK)
			return false;
	}

	sqlite3_bind_int(request, 1, cacheID);

	while(sqlite3_step(request) == SQLITE_ROW)
	{
		uint type = sqlite3_column_int(request, 1), data = sqlite3_column_int(request, 0);
		
		if(type == RDBS_TYPE_AUTHOR)
			*authorID = data;

		else if(type == RDBS_TYPE_TAG)
			*tagID = data;
		
		else if(type == RDBS_TYPE_TYPE)
			*typeID = data;
	}

	sqlite3_reset(request);
	
	if(table == NULL)
		sqlite3_finalize(request);
	
	return true;
}


uint * getSearchData(byte type, charType *** dataName, uint * dataLength)
{
	if(dataName == NULL || dataLength == NULL || cache == NULL)
		return NULL;
	
	sqlite3_stmt * request;

	if(type == RDBS_TYPE_AUTHOR)
	{
		*dataLength = nbAuthor;
		if(sqlite3_prepare_v2(cache, "SELECT * FROM "TABLE_NAME_AUTHOR" ORDER BY "DBNAMETOID(RDB_authors)" ASC;", -1, &request, NULL) != SQLITE_OK)
			return NULL;
	}
	else if(type == RDBS_TYPE_TAG)
	{
		*dataLength = nbTag;
		if(sqlite3_prepare_v2(cache, "SELECT "DBNAMETOID(RDBS_tagCode)", "DBNAMETOID(RDB_ID)" FROM "TABLE_NAME_TAG" WHERE "DBNAMETOID(RDBS_tagType)" = "STRINGIZE(RDBS_TYPE_TAG)";", -1, &request, NULL) != SQLITE_OK)
			return NULL;
	}
	else if(type == RDBS_TYPE_TYPE)
	{
		*dataLength = nbType;
		if(sqlite3_prepare_v2(cache, "SELECT "DBNAMETOID(RDBS_tagCode)", "DBNAMETOID(RDB_ID)" FROM "TABLE_NAME_TAG" WHERE "DBNAMETOID(RDBS_tagType)" = "STRINGIZE(RDBS_TYPE_TYPE)";", -1, &request, NULL) != SQLITE_OK)
			return NULL;
	}
	else
		return NULL;
	
	uint pos = 0, length = *dataLength, * codes = malloc(length * sizeof(uint));
	*dataName = malloc(length * sizeof(charType *));
	
	if(codes == NULL || *dataName == NULL)
	{
		free(codes);
		free(*dataName);	*dataName = NULL;
		
		sqlite3_finalize(request);
		return NULL;
	}
	
	while(pos < length && sqlite3_step(request) == SQLITE_ROW)
	{
		if(type == RDBS_TYPE_AUTHOR)
		{
			(*dataName)[pos] = malloc(REPO_NAME_LENGTH * sizeof(charType));
			if((*dataName)[pos] == NULL)
				continue;
			
			const char * utf8 = (const char *) sqlite3_column_text(request, 0);
			
			if(utf8 == NULL)
			{
				free((*dataName)[pos]);
				continue;
			}
			
			utf8_to_wchar(utf8, strlen(utf8), (*dataName)[pos], REPO_NAME_LENGTH, 0);
		}
		else
		{
			(*dataName)[pos] = wstrdup(getTagForCode(sqlite3_column_int(request, 0)));
			if((*dataName)[pos] == NULL)
				continue;
		}
		
		codes[pos++] = sqlite3_column_int(request, 1);
	}
	
	return codes;
}
