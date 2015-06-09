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

static bool initialized = false;

static int64_t nbAuthor = 0, nbTag = 0, nbType = 0;

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
	sqlite3_stmt * flushCategories;
	sqlite3_stmt * readProject;
	
} * SEARCH_JUMPTABLE;

#define TABLE_NAME_AUTHOR		"rakSearch1"
#define TABLE_NAME_TAG			"rakSearch2"
#define TABLE_NAME_CORRES		"rakSearch3"
#define TABLE_NAME_RESTRICTIONS	"rakSearch4"

bool manipulateProjectSearch(SEARCH_JUMPTABLE table, bool wantInsert, PROJECT_DATA project);
void updateElementCount(byte type, int change);

void buildSearchTables(sqlite3 *_cache)
{
	if(_cache == NULL)
		return;
	
	sqlite3_stmt* request = NULL;
	
	if(createRequest(_cache, "CREATE TABLE "TABLE_NAME_AUTHOR" ("DBNAMETOID(RDB_authors)" TEXT UNIQUE ON CONFLICT FAIL, "DBNAMETOID(RDB_ID)" INTEGER PRIMARY KEY AUTOINCREMENT);", &request) != SQLITE_OK || sqlite3_step(request) != SQLITE_DONE)
	{
		initialized = false;
		destroyRequest(request);
		return;
	}
	
	destroyRequest(request);
	
	if(createRequest(_cache, "CREATE TABLE "TABLE_NAME_TAG"("DBNAMETOID(RDB_ID)" INTEGER PRIMARY KEY AUTOINCREMENT, "DBNAMETOID(RDB_tagID)" INTEGER NOT NULL, "DBNAMETOID(RDBS_tagType)" INTEGER NOT NULL);", &request) != SQLITE_OK || sqlite3_step(request) != SQLITE_DONE)
	{
		initialized = false;
		destroyRequest(request);
		return;
	}

	destroyRequest(request);
	
	if(createRequest(_cache, "CREATE TABLE "TABLE_NAME_CORRES" ("DBNAMETOID(RDB_ID)" INTEGER NOT NULL, "DBNAMETOID(RDBS_dataID)" INTEGER NOT NULL, "DBNAMETOID(RDBS_dataType)" INTEGER NOT NULL);", &request) != SQLITE_OK || sqlite3_step(request) != SQLITE_DONE)
	{
		initialized = false;
		destroyRequest(request);
		return;
	}
	
	destroyRequest(request);
	
	if(createRequest(_cache, "CREATE TABLE "TABLE_NAME_RESTRICTIONS" ("DBNAMETOID(RDBS_dataType)" INTEGER NOT NULL, "DBNAMETOID(RDBS_dataID)" INTEGER NOT NULL);", &request) != SQLITE_OK || sqlite3_step(request) != SQLITE_DONE)
	{
		initialized = false;
		destroyRequest(request);
		return;
	}
	
	//We need at least one (invalid) data :/
	if(createRequest(_cache, "INSERT INTO "TABLE_NAME_RESTRICTIONS" ("DBNAMETOID(RDBS_dataType)", "DBNAMETOID(RDBS_dataID)") values("STRINGIZE(RDBS_TYPE_UNUSED)", "STRINGIZE(RDBS_TYPE_UNUSED)");", &request) != SQLITE_OK || sqlite3_step(request) != SQLITE_DONE)
	{
		initialized = false;
		destroyRequest(request);
		return;
	}
	
	if(createCollate(_cache) != SQLITE_OK)
	{
		initialized = false;
		return;
	}
	
	initialized = true;
	destroyRequest(request);
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
	
	if(createRequest(_cache, "INSERT INTO "TABLE_NAME_AUTHOR"("DBNAMETOID(RDB_authors)") values(?1);", &(output->addAuthor)) != SQLITE_OK)
		goto fail;
	else
		stage++;
	
	if(createRequest(_cache, "INSERT INTO "TABLE_NAME_TAG"("DBNAMETOID(RDB_tagID)", "DBNAMETOID(RDBS_tagType)") values(?1, "STRINGIZE(RDBS_TYPE_TAG)");", &(output->addTag)) != SQLITE_OK)
		goto fail;
	else
		stage++;
	
	if(createRequest(_cache, "INSERT INTO "TABLE_NAME_TAG"("DBNAMETOID(RDB_tagID)", "DBNAMETOID(RDBS_tagType)") values(?1, "STRINGIZE(RDBS_TYPE_CAT)");", &(output->addType)) != SQLITE_OK)
		goto fail;
	else
		stage++;
	
	if(createRequest(_cache, "SELECT "DBNAMETOID(RDB_ID)" FROM "TABLE_NAME_AUTHOR" WHERE "DBNAMETOID(RDB_authors)" = ?1;", &(output->getAuthorID)) != SQLITE_OK)
		goto fail;
	else
		stage++;
	
	if(createRequest(_cache, "SELECT "DBNAMETOID(RDB_ID)" FROM "TABLE_NAME_TAG" WHERE "DBNAMETOID(RDBS_tagType)" = "STRINGIZE(RDBS_TYPE_TAG)" AND "DBNAMETOID(RDB_tagID)" = ?1;", &(output->getTagID)) != SQLITE_OK)
		goto fail;
	else
		stage++;
	
	if(createRequest(_cache, "SELECT "DBNAMETOID(RDB_ID)" FROM "TABLE_NAME_TAG" WHERE "DBNAMETOID(RDBS_tagType)" = "STRINGIZE(RDBS_TYPE_CAT)" AND "DBNAMETOID(RDB_tagID)" = ?1;", &(output->getTypeID)) != SQLITE_OK)
		goto fail;
	else
		stage++;
	
	if(createRequest(_cache, "INSERT INTO "TABLE_NAME_CORRES" ("DBNAMETOID(RDB_ID)", "DBNAMETOID(RDBS_dataID)", "DBNAMETOID(RDBS_dataType)") values(?1, ?2, ?3);", &(output->addProject)) != SQLITE_OK)
		goto fail;
	else
		stage++;

	if(createRequest(_cache, "UPDATE "TABLE_NAME_CORRES" SET "DBNAMETOID(RDBS_dataID)" = ?2 WHERE "DBNAMETOID(RDB_ID)" = ?1 AND "DBNAMETOID(RDBS_dataType)" = ?3;", &(output->updateProject)) != SQLITE_OK)
		goto fail;
	else
		stage++;
	
	if(createRequest(_cache, "DELETE FROM "TABLE_NAME_CORRES" WHERE "DBNAMETOID(RDB_ID)" = ?1", &(output->removeProject)) != SQLITE_OK)
		goto fail;
	else
		stage++;
	
	if(createRequest(_cache, "DELETE FROM "TABLE_NAME_CORRES" WHERE "DBNAMETOID(RDB_ID)" = ?1 AND "DBNAMETOID(RDBS_dataType)" = "STRINGIZE(RDBS_TYPE_CAT)";", &(output->flushCategories)) != SQLITE_OK)
		goto fail;
	else
		stage++;
	
	if(createRequest(_cache, "SELECT * FROM "TABLE_NAME_CORRES" WHERE "DBNAMETOID(RDB_ID)" = ?1;", &(output->readProject)) != SQLITE_OK)
		goto fail;
	else
		stage++;
	
	if(0)
	{
fail:
		if(stage > 0)	destroyRequest(output->addAuthor);
		if(stage > 1)	destroyRequest(output->addTag);
		if(stage > 2)	destroyRequest(output->addType);
		if(stage > 3)	destroyRequest(output->getAuthorID);
		if(stage > 4)	destroyRequest(output->getTagID);
		if(stage > 5)	destroyRequest(output->getTypeID);
		if(stage > 6)	destroyRequest(output->addProject);
		if(stage > 7)	destroyRequest(output->updateProject);
		if(stage > 8)	destroyRequest(output->removeProject);
		if(stage > 9)	destroyRequest(output->flushCategories);
		
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
	
	destroyRequest(table->addAuthor);
	destroyRequest(table->addTag);
	destroyRequest(table->addType);
	destroyRequest(table->getAuthorID);
	destroyRequest(table->getTagID);
	destroyRequest(table->getTypeID);
	destroyRequest(table->addProject);
	destroyRequest(table->updateProject);
	destroyRequest(table->removeProject);
	destroyRequest(table->flushCategories);
	destroyRequest(table->readProject);
	
	free(table);
}

#pragma mark - Manipulate the content

uint getFromSearch(void * _table, byte type, PROJECT_DATA project)
{
	return _getFromSearch(_table, type, type == PULL_SEARCH_AUTHORID ? (void*) &(project.authorName) : (type == PULL_SEARCH_TAGID ? &(project.mainTag) : &(project.category)));
}

uint getIDForTag(byte type, uint code)
{
	return _getFromSearch(NULL, type, &code);
}

uint _getFromSearch(void * _table, byte type, void * data)
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
			
			size_t length = wstrlen((charType *) data);
			char utf8[4 * length + 1];
			length = wchar_to_utf8((charType *) data, length, utf8, 4 * length + 1, 0);
			
			sqlite3_bind_text(request, 1, utf8, length, SQLITE_TRANSIENT);
			break;
		}
			
		case PULL_SEARCH_TAGID:
		{
			request = table->getTagID;
			sqlite3_bind_int64(request, 1, *(uint *) data);
			break;
		}
			
		case PULL_SEARCH_CATID:
		{
			request = table->getTypeID;
			sqlite3_bind_int64(request, 1, *(uint *) data);
			break;
		}
			
		default:
		{
			return UINT_MAX;
		}
	}
	
	uint output = UINT_MAX;
	
	if((output = (uint) sqlite3_step(request)) == SQLITE_ROW)
		output = (uint32_t) sqlite3_column_int(request, 0);
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
			sqlite3_bind_int64(request, 1, project.mainTag);
			break;
		}
			
		case INSERT_CAT:
		{
			requestType = RDBS_TYPE_CAT;
			request = table->addType;
			sqlite3_bind_int64(request, 1, project.category);
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
	sqlite3_bind_int(request, 1, (int32_t) cacheID);

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
	uint catID = getFromSearch(table, PULL_SEARCH_CATID, project), tagID = getFromSearch(table, PULL_SEARCH_TAGID, project), authorID = getFromSearch(table, PULL_SEARCH_AUTHORID, project);
	uint oldCat, oldTag, oldAuthor;
	
	getProjectSearchData(table, project.cacheDBID, &oldAuthor, &oldTag, &oldCat);
	
	if(catID == UINT_MAX && insertInSearch(table, INSERT_CAT, project))
		catID = getFromSearch(table, PULL_SEARCH_CATID, project);
	
	if(tagID == UINT_MAX && insertInSearch(table, INSERT_TAG, project))
		tagID = getFromSearch(table, PULL_SEARCH_TAGID, project);
	
	if(authorID == UINT_MAX && insertInSearch(table, INSERT_AUTHOR, project))
		authorID = getFromSearch(table, PULL_SEARCH_AUTHORID, project);
	
	if(catID == UINT_MAX || tagID == UINT_MAX || authorID == UINT_MAX)
		return false;
	
	bool fail = false;
	sqlite3_stmt * request = wantInsert ? table->addProject : table->updateProject;

	if(authorID != oldAuthor)
	{
		sqlite3_bind_int(request, 1, (int32_t) project.cacheDBID);
		sqlite3_bind_int(request, 2, (int32_t) authorID);
		sqlite3_bind_int(request, 3, RDBS_TYPE_AUTHOR);
		
		fail = sqlite3_step(request) != SQLITE_DONE;
		sqlite3_reset(request);
		
		if(fail)
			return false;

		checkIfRemainingAndDelete(oldAuthor, RDBS_TYPE_AUTHOR);
	}

	if(catID != oldCat)
	{
		if(!wantInsert)
		{
			//We have to flush the previous category (as all parents are there)
			sqlite3_bind_int(table->flushCategories, 1, (int32_t) project.cacheDBID);
			sqlite3_step(table->flushCategories);
			sqlite3_reset(table->flushCategories);
		}
		
		//Insert the catID and its parents
		while (!fail && catID != TAG_NO_VALUE)
		{
			sqlite3_bind_int(request, 1, (int32_t) project.cacheDBID);
			sqlite3_bind_int(request, 2, (int32_t) catID);
			sqlite3_bind_int(request, 3, RDBS_TYPE_CAT);
			
			fail = sqlite3_step(request) != SQLITE_DONE;
			sqlite3_reset(request);
			
			if(!fail)
				catID = getRootCategoryIDForID(catID);
		}
		
		if(fail)
			return false;
		
		checkIfRemainingAndDelete(oldCat, RDBS_TYPE_CAT);
	}
	
	if(tagID != oldTag)
	{
		sqlite3_bind_int(request, 1, (int32_t) project.cacheDBID);
		sqlite3_bind_int(request, 2, (int32_t) tagID);
		sqlite3_bind_int(request, 3, RDBS_TYPE_TAG);
		
		fail = sqlite3_step(request) != SQLITE_DONE;
		sqlite3_reset(request);
		
		checkIfRemainingAndDelete(oldTag, RDBS_TYPE_TAG);
	}
	
	if(wantInsert && !fail)
	{
		sqlite3_bind_int(request, 1, (int32_t) project.cacheDBID);
		sqlite3_bind_int64(request, 2, (int64_t) getRepoID(project.repo));
		sqlite3_bind_int(request, 3, RDBS_TYPE_SOURCE);
		
		fail = sqlite3_step(request) != SQLITE_DONE;
		sqlite3_reset(request);
	}

	return !fail;
}

#pragma mark - Manipulate tags

bool insertRestriction(uint64_t code, byte type)
{
	if(cache == NULL || code == UINT_MAX || (type != RDBS_TYPE_AUTHOR && type != RDBS_TYPE_SOURCE && type != RDBS_TYPE_TAG && type != RDBS_TYPE_CAT))
		return false;
	
	sqlite3_stmt * request;
	
	if(createRequest(cache, "SELECT COUNT() FROM "TABLE_NAME_RESTRICTIONS" WHERE "DBNAMETOID(RDBS_dataType)" = ?1 AND "DBNAMETOID(RDBS_dataID)" = ?2 LIMIT 1", &request) != SQLITE_OK)
		return false;
	
	sqlite3_bind_int(request, 1, type);
	sqlite3_bind_int64(request, 2, (int64_t) code);
	
	if(sqlite3_step(request) != SQLITE_ROW || sqlite3_column_int(request, 0) != 0)
	{
		destroyRequest(request);
		return false;
	}
	
	destroyRequest(request);

	if(createRequest(cache, "INSERT INTO "TABLE_NAME_RESTRICTIONS" ("DBNAMETOID(RDBS_dataType)", "DBNAMETOID(RDBS_dataID)") values(?1, ?2);", &request) != SQLITE_OK)
		return false;
	
	sqlite3_bind_int(request, 1, type);
	sqlite3_bind_int64(request, 2, (int64_t) code);
	
	bool output = sqlite3_step(request) == SQLITE_DONE;
	
	destroyRequest(request);
	
	notifyRestrictionChanged();
	
	return output;
}

bool removeRestriction(uint64_t code, byte type)
{
	if(cache == NULL || code == UINT_MAX || (type != RDBS_TYPE_AUTHOR && type != RDBS_TYPE_SOURCE && type != RDBS_TYPE_TAG && type != RDBS_TYPE_CAT))
		return false;
	
	sqlite3_stmt * request;
	
	if(createRequest(cache, "DELETE FROM "TABLE_NAME_RESTRICTIONS" WHERE "DBNAMETOID(RDBS_dataType)" = ?1 AND "DBNAMETOID(RDBS_dataID)" = ?2;", &request) != SQLITE_OK)
		return false;
	
	sqlite3_bind_int(request, 1, type);
	sqlite3_bind_int64(request, 2, (int64_t) code);
	
	bool output = sqlite3_step(request) == SQLITE_DONE;
	
	destroyRequest(request);
	
	notifyRestrictionChanged();
	
	return output;
}

bool flushRestriction()
{
	if(cache == NULL)
		return false;
	
	sqlite3_stmt * request;
	
	if(createRequest(cache, "DELETE FROM "TABLE_NAME_RESTRICTIONS";", &request) != SQLITE_OK)
		return false;
	
	bool output = sqlite3_step(request) == SQLITE_DONE;
	
	destroyRequest(request);
	
	notifyRestrictionChanged();
	
	return output;
}

#pragma mark - Maintenance API

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
			
		case RDBS_TYPE_CAT:
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
	
	if(createRequest(cache, "SELECT COUNT() FROM "TABLE_NAME_CORRES" WHERE "DBNAMETOID(RDBS_dataID)" = ?1 AND "DBNAMETOID(RDBS_dataType)" = ?2;", &request) != SQLITE_OK)
		return;
	
	sqlite3_bind_int64(request, 1, data);
	sqlite3_bind_int(request, 2, type);
	
	bool nothingRemaining = sqlite3_step(request) == SQLITE_DONE;
	
	destroyRequest(request);
	
	//We have to delete the entry
	if(nothingRemaining)
	{
		if(type == RDBS_TYPE_AUTHOR && createRequest(cache, "DELETE FROM "TABLE_NAME_AUTHOR" WHERE "DBNAMETOID(RDB_ID)" = ?1", &request) == SQLITE_OK);

		else if(type == RDBS_TYPE_TAG && createRequest(cache, "DELETE FROM "TABLE_NAME_TAG" WHERE "DBNAMETOID(RDB_ID)" = ?1 AND "DBNAMETOID(RDBS_tagType)" = "STRINGIZE(RDBS_TYPE_TAG)"", &request) == SQLITE_OK);
		
		else if(type == RDBS_TYPE_CAT && createRequest(cache, "DELETE FROM "TABLE_NAME_TAG" WHERE "DBNAMETOID(RDB_ID)" = ?1 AND "DBNAMETOID(RDBS_tagType)" = "STRINGIZE(RDBS_TYPE_CAT)"", &request) == SQLITE_OK);
		
		else
			return;
		
		sqlite3_bind_int(request, 1, (int32_t) data);
		
		if(sqlite3_step(request) == SQLITE_DONE)
			updateElementCount(type, -1);
			
		destroyRequest(request);
	}
}

#pragma mark - Read and work on those data

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
		if(createRequest(cache, "SELECT "DBNAMETOID(RDBS_dataID)", "DBNAMETOID(RDBS_dataType)" FROM "TABLE_NAME_CORRES" WHERE "DBNAMETOID(RDB_ID)" = ?1;", &request) != SQLITE_OK)
			return false;
	}

	sqlite3_bind_int(request, 1, (int32_t) cacheID);

	while(sqlite3_step(request) == SQLITE_ROW)
	{
		uint type = (uint32_t) sqlite3_column_int(request, 1), data = (uint32_t) sqlite3_column_int(request, 0);
		
		if(type == RDBS_TYPE_AUTHOR)
			*authorID = data;

		else if(type == RDBS_TYPE_TAG)
			*tagID = data;
		
		else if(type == RDBS_TYPE_CAT)
			*typeID = data;
	}

	sqlite3_reset(request);
	
	if(table == NULL)
		destroyRequest(request);
	
	return true;
}

uint64_t * getSearchData(byte type, charType *** dataName, uint * dataLength)
{
	if(dataName == NULL || dataLength == NULL || cache == NULL)
		return NULL;
	
	sqlite3_stmt * request;

	if(type == RDBS_TYPE_AUTHOR)
	{
		*dataLength = nbAuthor;
		if(createRequest(cache, "SELECT * FROM "TABLE_NAME_AUTHOR" ORDER BY "DBNAMETOID(RDB_authors)" COLLATE "SORT_FUNC" ASC;", &request) != SQLITE_OK)
			return NULL;
	}
	else if(type == RDBS_TYPE_TAG)
	{
		*dataLength = nbTag;
		if(createRequest(cache, "SELECT "DBNAMETOID(RDB_tagID)", "DBNAMETOID(RDB_ID)" FROM "TABLE_NAME_TAG" WHERE "DBNAMETOID(RDBS_tagType)" = "STRINGIZE(RDBS_TYPE_TAG)";", &request) != SQLITE_OK)
			return NULL;
	}
	else if(type == RDBS_TYPE_CAT)
	{
		*dataLength = nbType;
		if(createRequest(cache, "SELECT "DBNAMETOID(RDB_tagID)", "DBNAMETOID(RDB_ID)" FROM "TABLE_NAME_TAG" WHERE "DBNAMETOID(RDBS_tagType)" = "STRINGIZE(RDBS_TYPE_CAT)";", &request) != SQLITE_OK)
			return NULL;
	}
	else
		return NULL;
	
	uint pos = 0, length = *dataLength;
	uint64_t * codes = malloc(length * sizeof(uint64_t));
	*dataName = malloc(length * sizeof(charType *));
	
	if(codes == NULL || *dataName == NULL)
	{
		free(codes);
		free(*dataName);	*dataName = NULL;
		
		destroyRequest(request);
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
		else if(type == RDBS_TYPE_CAT)
		{
			(*dataName)[pos] = wstrdup(getCatNameForCode((uint32_t) sqlite3_column_int(request, 0)));
			if((*dataName)[pos] == NULL)
				continue;
		}
		else
		{
			(*dataName)[pos] = wstrdup(getTagNameForCode((uint32_t) sqlite3_column_int(request, 0)));
			if((*dataName)[pos] == NULL)
				continue;
		}
		
		codes[pos++] = (uint64_t) sqlite3_column_int64(request, 1);
	}
	
	destroyRequest(request);
	
	return codes;
}

#pragma mark - Most highlevel API

uint * getFilteredProject(uint * dataLength, const char * searchQuery, bool wantInstalledOnly, bool wantFreeOnly)
{
	if(dataLength == NULL)
		return NULL;
	else
		*dataLength = UINT_MAX;
	
	sqlite3_stmt * request;
	
	size_t maxLength = nbElemInCache;
	uint * output = malloc(maxLength * sizeof(uint));	//We allocate more space, but will reduce at the end
	
	if(output == NULL)
		return NULL;
	
	//Bon, on va essayer de documenter un poil ce joujou
	//Cette requête est composée de deux morceaux fonctionnellements très proches, je vais documenter la version compilée de cette requêtes, les constantes n'ayant pas un sens critique
	//Cette requête est un INTERSECT de deux requêtes de cette forme:
	//SELECT `1` FROM rakSearch3 list JOIN rakSearch4 rest WHERE (SELECT COUNT() = 0 FROM rakSearch4 WHERE rakSearch4.`20` = 2) OR (rest.`20` = 2 AND list.`19` = rest.`19` AND list.`20` = rest.`20`) GROUP BY `1` HAVING COUNT(`1`) >= (SELECT COUNT() FROM rakSearch4 WHERE rakSearch4.`20` = 2);
	//Nous faisons une jointure entre la table de correspondance et la table contenant les restrictions
	//Ensuite, soit aucune restriction compatible n'est trouvée, et tout est alors autorisé (COUNT = 0 -> TRUE), soit une restriction du type considéré est présente, auquel cas elle est filtrée (`20` = 2) et la restriction est appliquée
	//Enfin, nous regroupons les correspondances (GROUP BY) dans le cas ou toutes les conditions ont étés remplies (HAVING). Nous utilisons >= pour fonctionner dans le cas où aucune restriction n'a été trouvée
	//Nous supportons deux types de restrictions, les AND et les OR, ceci était une AND, forçant que toutes ses conditions soient remplies à travers son HAVING
	//Les OR remplacent GROUP BY ... HAVING ... par SELECT DISTINCT, retirant les collisions
	//La donnée que nous extrayons est l'ID des éléments valides
	
	//Current recipe: (AUTHOR(|) OR SOURCE(|) OR TYPE(|)) AND TAG(&)
	
	//Futur maintainer, I wish you good luck
	//If TABLE_NAME_RESTRICTIONS is empty, it seems the request return no data, asshole
	//We insert an invalid data when creating it to solve the issue
	
	uint searchLength = searchQuery == NULL ? 0 : strlen(searchQuery);
	char requestString[1024 + 2 * searchLength];
	
	if(!searchLength && !wantInstalledOnly && !wantFreeOnly)
		strncpy(requestString, "SELECT DISTINCT "DBNAMETOID(RDB_ID)" FROM "TABLE_NAME_CORRES" list JOIN "TABLE_NAME_RESTRICTIONS" rest WHERE (SELECT COUNT() = 0 FROM "TABLE_NAME_RESTRICTIONS" WHERE "TABLE_NAME_RESTRICTIONS"."DBNAMETOID(RDBS_dataType)" IN ("STRINGIZE(RDBS_TYPE_AUTHOR)", "STRINGIZE(RDBS_TYPE_SOURCE)", "STRINGIZE(RDBS_TYPE_CAT)")) OR (rest."DBNAMETOID(RDBS_dataType)" IN ("STRINGIZE(RDBS_TYPE_AUTHOR)", "STRINGIZE(RDBS_TYPE_SOURCE)", "STRINGIZE(RDBS_TYPE_CAT)") AND list."DBNAMETOID(RDBS_dataID)" = rest."DBNAMETOID(RDBS_dataID)" AND list."DBNAMETOID(RDBS_dataType)" = rest."DBNAMETOID(RDBS_dataType)") INTERSECT SELECT "DBNAMETOID(RDB_ID)" FROM "TABLE_NAME_CORRES" list JOIN "TABLE_NAME_RESTRICTIONS" rest WHERE (SELECT COUNT() = 0 FROM "TABLE_NAME_RESTRICTIONS" WHERE "TABLE_NAME_RESTRICTIONS"."DBNAMETOID(RDBS_dataType)" = "STRINGIZE(RDBS_TYPE_TAG)") OR (rest."DBNAMETOID(RDBS_dataType)" = "STRINGIZE(RDBS_TYPE_TAG)" AND list."DBNAMETOID(RDBS_dataID)" = rest."DBNAMETOID(RDBS_dataID)" AND list."DBNAMETOID(RDBS_dataType)" = rest."DBNAMETOID(RDBS_dataType)") GROUP BY "DBNAMETOID(RDB_ID)" HAVING COUNT("DBNAMETOID(RDB_ID)") >= (SELECT COUNT() FROM "TABLE_NAME_RESTRICTIONS" WHERE "TABLE_NAME_RESTRICTIONS"."DBNAMETOID(RDBS_dataType)" = "STRINGIZE(RDBS_TYPE_TAG)") ORDER BY "DBNAMETOID(RDB_ID)" ASC;", sizeof(requestString));
	else
	{
		char additionnalRequest[150 + searchLength];	additionnalRequest[0] = 0;
		
		if(searchLength)
			snprintf(additionnalRequest, sizeof(additionnalRequest), " AND cache."DBNAMETOID(RDB_projectName)" LIKE \"%s%%\"", searchQuery);
		
		if(wantFreeOnly)
		{
			uint currentLength = strlen(additionnalRequest);
			snprintf(&(additionnalRequest[currentLength]), sizeof(additionnalRequest) - currentLength, " AND cache."DBNAMETOID(RDB_isPaid)" = 0");
		}
		else if(wantInstalledOnly)
		{
			uint currentLength = strlen(additionnalRequest);
			snprintf(&(additionnalRequest[currentLength]), sizeof(additionnalRequest) - currentLength, " AND cache."DBNAMETOID(RDB_isInstalled)" = 1");
		}
			
		snprintf(requestString, sizeof(requestString), "SELECT DISTINCT list."DBNAMETOID(RDB_ID)" FROM "TABLE_NAME_CORRES" list JOIN "TABLE_NAME_RESTRICTIONS" rest, "MAIN_CACHE" cache ON cache."DBNAMETOID(RDB_ID)" = list."DBNAMETOID(RDB_ID)" WHERE ((SELECT COUNT() = 0 FROM "TABLE_NAME_RESTRICTIONS" WHERE "TABLE_NAME_RESTRICTIONS"."DBNAMETOID(RDBS_dataType)" IN ("STRINGIZE(RDBS_TYPE_AUTHOR)", "STRINGIZE(RDBS_TYPE_SOURCE)", "STRINGIZE(RDBS_TYPE_CAT)")) OR (rest."DBNAMETOID(RDBS_dataType)" IN ("STRINGIZE(RDBS_TYPE_AUTHOR)", "STRINGIZE(RDBS_TYPE_SOURCE)", "STRINGIZE(RDBS_TYPE_CAT)") AND list."DBNAMETOID(RDBS_dataID)" = rest."DBNAMETOID(RDBS_dataID)" AND list."DBNAMETOID(RDBS_dataType)" = rest."DBNAMETOID(RDBS_dataType)")) %s INTERSECT SELECT "DBNAMETOID(RDB_ID)" FROM "TABLE_NAME_CORRES" list JOIN "TABLE_NAME_RESTRICTIONS" rest WHERE (SELECT COUNT() = 0 FROM "TABLE_NAME_RESTRICTIONS" WHERE "TABLE_NAME_RESTRICTIONS"."DBNAMETOID(RDBS_dataType)" = "STRINGIZE(RDBS_TYPE_TAG)") OR (rest."DBNAMETOID(RDBS_dataType)" = "STRINGIZE(RDBS_TYPE_TAG)" AND list."DBNAMETOID(RDBS_dataID)" = rest."DBNAMETOID(RDBS_dataID)" AND list."DBNAMETOID(RDBS_dataType)" = rest."DBNAMETOID(RDBS_dataType)") GROUP BY "DBNAMETOID(RDB_ID)" HAVING COUNT("DBNAMETOID(RDB_ID)") >= (SELECT COUNT() FROM "TABLE_NAME_RESTRICTIONS" WHERE "TABLE_NAME_RESTRICTIONS"."DBNAMETOID(RDBS_dataType)" = "STRINGIZE(RDBS_TYPE_TAG)") ORDER BY "DBNAMETOID(RDB_ID)" ASC;", additionnalRequest);

	}
	
	if(createRequest(cache, requestString, &request) != SQLITE_OK)
	{
#ifdef DEV_VERSION
		logR(requestString);
#endif
		free(output);
		return NULL;
	}
	
	size_t realLength = 0;
	while (realLength < nbElemInCache && sqlite3_step(request) == SQLITE_ROW)
	{
		output[realLength++] = (uint32_t) sqlite3_column_int(request, 0);
	}
	
	destroyRequest(request);
	
	*dataLength = realLength;

	if(realLength < nbElemInCache)
	{
		if(realLength == 0)	//No data :/
		{
			free(output);
			return NULL;
		}
			
		void * tmp = realloc(output, realLength * sizeof(uint));
		if(tmp != NULL)
			output = tmp;
	}
	
	return output;
}

char ** getProjectNameStartingWith(const char * start, uint * nbProject)
{
	char ** output = calloc(nbElemInCache, sizeof(char *));
	if(output == NULL)
		return NULL;
	
	uint length = strlen(start);
	char requestText[length + 200];
	snprintf(requestText, sizeof(requestText), "SELECT "DBNAMETOID(RDB_projectName)" FROM "MAIN_CACHE" WHERE "DBNAMETOID(RDB_projectName)" LIKE \"%s%%\" ORDER BY "DBNAMETOID(RDB_projectName)" COLLATE "SORT_FUNC" ASC", start);
	
	sqlite3_stmt * request;
	
	if(createRequest(cache, requestText, &request) != SQLITE_OK)
	{
		free(output);
		return NULL;
	}
	
	size_t realLength = 0;
	while (realLength < nbElemInCache && sqlite3_step(request) == SQLITE_ROW)
	{
		output[realLength] = strdup((void*) sqlite3_column_text(request, 0));
		
		if(output[realLength] != NULL)
			realLength++;
	}
	
	destroyRequest(request);
	
	if(nbProject != NULL)
		*nbProject = realLength;
	
	return output;
}