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
#include "tag.h"

sqlite3 *immatureCache = NULL;

#pragma mark - Generate queries

sqlite3_stmt * tagUpdateQuery(sqlite3 * db, bool shouldUpdate)
{
	char body[] = "INTO "TABLE_TAGS" ("DBNAMETOID(RDB_tagID)", "DBNAMETOID(RDB_tagName)") VALUES (?1, ?2);", mainRequest[sizeof(body) + 50];
	snprintf(mainRequest, sizeof(mainRequest), "INSERT %s %s", shouldUpdate ? "OR REPLACE" : "", body);
	return createRequest(db, mainRequest);
}

sqlite3_stmt * catUpdateQuery(sqlite3 * db, bool shouldUpdate)
{
	char body[] = "INTO "TABLE_CATEGORY" ("DBNAMETOID(RDB_CAT_ID)", "DBNAMETOID(RDB_CAT_rootID)", "DBNAMETOID(RDB_CAT_name)") VALUES (?1, ?2, ?3);", mainRequest[sizeof(body) + 50];
	
	snprintf(mainRequest, sizeof(mainRequest), "INSERT %s %s", shouldUpdate ? " OR REPLACE" : "", body);
	
	return createRequest(db, mainRequest);
}

#pragma mark - Version query search

void updateTagDBVersion(uint newDBVersion)
{
	if(cache == NULL && immatureCache == NULL)
		return;
	
	sqlite3_stmt * request = createRequest(cache != NULL ? cache : immatureCache, "INSERT OR REPLACE INTO "TABLE_TAG_VERSION" (`"VERSION_COLUMN_NAME"`) VALUES(?1);");

	if(request == NULL)
		return;
	
	sqlite3_bind_int(request, 1, (int32_t) newDBVersion);
	sqlite3_step(request);
	destroyRequest(request);
}

uint getTagDBVersion()
{
	sqlite3_stmt * request = NULL;
	uint output;

	if((cache == NULL && immatureCache == NULL) ||
		(request = createRequest(cache != NULL ? cache : immatureCache, "SELECT `"VERSION_COLUMN_NAME"` FROM "TABLE_TAG_VERSION" LIMIT 1")) == NULL ||
	   	sqlite3_step(request) != SQLITE_ROW)
	{
		output = DEFAULT_TAG_VERSION;
	}
	else
		output = (uint32_t) sqlite3_column_int(request, 0);
	
	destroyRequest(request);
	return output;
}

#pragma mark - Internal API

bool createTagsTable(sqlite3 * mainCache)
{
	if(mainCache == NULL)
	{
		logR("Initialization is incomplete!");
		return false;
	}
	
	sqlite3_stmt * request = createRequest(mainCache, "CREATE TABLE "TABLE_TAGS" ("DBNAMETOID(RDB_tagID)" INTEGER PRIMARY KEY NOT NULL, "DBNAMETOID(RDB_tagName)" TEXT NOT NULL); CREATE INDEX hopperIsGud ON "TABLE_TAGS"("DBNAMETOID(RDB_tagID)");");

	if(request == NULL || sqlite3_step(request) != SQLITE_DONE)
	{
		logR("Initialization error small");
		
		destroyRequest(request);

		return false;
	}
	
	destroyRequest(request);
	
	if((request = createRequest(mainCache, "CREATE TABLE "TABLE_CATEGORY" ("DBNAMETOID(RDB_CAT_ID)" INTEGER PRIMARY KEY NOT NULL, "DBNAMETOID(RDB_CAT_rootID)" INTEGER, "DBNAMETOID(RDB_CAT_name)" TEXT NOT NULL); CREATE INDEX gdbOverlldb ON "TABLE_CATEGORY"("DBNAMETOID(RDB_CAT_ID)", "DBNAMETOID(RDB_CAT_rootID)");")) == NULL || sqlite3_step(request) != SQLITE_DONE)
	{
		logR("Initialization error big");
		
		destroyRequest(request);
		
		return false;
	}
	
	destroyRequest(request);
	
	if((request = createRequest(mainCache, "CREATE TABLE "TABLE_TAG_VERSION" (`"VERSION_COLUMN_NAME"` INTEGER);")) == NULL || sqlite3_step(request) != SQLITE_DONE)
	{
		logR("Initialization error WTF");
		
		destroyRequest(request);
		
		return false;
	}
	
	destroyRequest(request);
	
	return true;
}

void tagUpdateCachedEntryWithRequest(sqlite3_stmt * request, TAG_VERBOSE * newData, uint nbData)
{
	for(uint i = 0; i < nbData; i++)
	{
		//We get a UTF8 version of the name
		size_t length = wstrlen((charType *) newData[i].name);
		char utf8[4 * length + 1];
		length = wchar_to_utf8((charType *) newData[i].name, length, utf8, 4 * length + 1, 0);

		//We submit the data to the request, then run and clean it
		sqlite3_bind_int(request, 1, (int32_t) newData[i].ID);
		sqlite3_bind_text(request, 2, utf8, length, SQLITE_STATIC);
		
		sqlite3_step(request);
		sqlite3_reset(request);
	}

	destroyRequest(request);
}

void catUpdateCachedEntryWithRequest(sqlite3_stmt * request, CATEGORY_VERBOSE * newData, uint nbData)
{
	for(uint i = 0; i < nbData; i++)
	{
		//We get a UTF8 version of the name
		size_t length = wstrlen((charType *) newData[i].name);
		char utf8[4 * length + 1];
		length = wchar_to_utf8((charType *) newData[i].name, length, utf8, 4 * length + 1, 0);
		
		//We submit the data to the request, then run and clean it
		sqlite3_bind_int(request, 1, (int32_t) newData[i].ID);
		sqlite3_bind_int(request, 2, (int32_t) newData[i].rootID);
		sqlite3_bind_text(request, 3, utf8, length, SQLITE_STATIC);
		
		sqlite3_step(request);
		sqlite3_reset(request);
	}
	
	destroyRequest(request);
}

#pragma mark - Main API

void tagUpdateCachedEntry(TAG_VERBOSE * newData, uint nbData)
{
	if((cache == NULL && immatureCache == NULL) || newData == NULL || nbData == 0)
		return;

	tagUpdateCachedEntryWithRequest(tagUpdateQuery(cache != NULL ? cache : immatureCache, true), newData, nbData);
}

void catUpdateCachedEntry(CATEGORY_VERBOSE * newData, uint nbData)
{
	if((cache == NULL && immatureCache == NULL) || newData == NULL || nbData == 0)
		return;
	
	catUpdateCachedEntryWithRequest(catUpdateQuery(cache != NULL ? cache : immatureCache, true), newData, nbData);
}

void dumpTagCat(TAG_VERBOSE * tags, uint nbTags, CATEGORY_VERBOSE * category, uint nbCat)
{
	MUTEX_LOCK(concurentColdUpdate);
	
	//Delete a temporary file if Rak crashed while working on it
	if(checkFileExist(WIP_TAG_DB))
		remove(WIP_TAG_DB);
	
	//Open the database
	sqlite3 * newDB = NULL;
	if(sqlite3_open(WIP_TAG_DB, &newDB) != SQLITE_OK)
	{
		logR("Couldn't open the temporary database");
		return;
	}
	else
		createTagsTable(newDB);
	
	//Dump the content using our helpers
	tagUpdateCachedEntryWithRequest(tagUpdateQuery(newDB, false), tags, nbTags);
	catUpdateCachedEntryWithRequest(catUpdateQuery(newDB, false), category, nbCat);
	
	//Closing the DB, then swapping the files
	sqlite3_close(newDB);
	
	remove(OLD_TAG_DB);
	rename(TAG_DB, OLD_TAG_DB);
	rename(WIP_TAG_DB, TAG_DB);
	
	MUTEX_UNLOCK(concurentColdUpdate);
}

void initializeTags(void * mainCache)
{
	MUTEX_CREATE(concurentColdUpdate);
	
	sqlite3 * coldDB;
	
	//Create the tables in the main cache
	createTagsTable(mainCache);
	
	if((!checkFileExist(TAG_DB) || sqlite3_open(TAG_DB , &coldDB) != SQLITE_OK) && (!checkFileExist(WIP_TAG_DB) || sqlite3_open(WIP_TAG_DB, &coldDB) != SQLITE_OK))
	{
		//Error, we should reset it with the version we ship with then trigger an update
		resetTagsToLocal();
		
		if(!checkFileExist(TAG_DB) || sqlite3_open(TAG_DB , &coldDB) != SQLITE_OK)
		{
			logR("We have significant issues setting up our environment, this may be caused by permission issues, please contact us at contact@rakshata.com");
			exit(0);
		}
	}
	
	sqlite3_stmt * requestRead, *requestWrite;
	
	//Build the tag base
	if((requestRead = createRequest(coldDB, "SELECT "DBNAMETOID(RDB_tagID)", "DBNAMETOID(RDB_tagName)" FROM "TABLE_TAGS)) != NULL)
	{
		requestWrite = tagUpdateQuery(mainCache, false);
		
		if(requestWrite != NULL)
		{
			while(sqlite3_step(requestRead) == SQLITE_ROW)
			{
				sqlite3_bind_int(requestWrite, 1, sqlite3_column_int(requestRead, 0));
				sqlite3_bind_text(requestWrite, 2, (void *) sqlite3_column_text(requestRead, 1), -1, SQLITE_STATIC);
				
				if(sqlite3_step(requestWrite) != SQLITE_DONE)
				{
#ifdef EXTENSIVE_LOGGING
					uint ID = (uint32_t) sqlite3_column_int(requestRead, 0);
					const unsigned char * text = sqlite3_column_text(requestRead, 1);
					char logMessage[100 + (text != NULL ? ustrlen(text) : 0)];
					
					if(text == NULL)
						snprintf(logMessage, sizeof(logMessage), "Error building the tag DB for ID %d: no text!", ID);
					else
						snprintf(logMessage, sizeof(logMessage), "Error building the tag DB for ID %d of text %s!", ID, text);
					
					logR(logMessage);
#endif
				}
				sqlite3_reset(requestWrite);
			}
			destroyRequest(requestWrite);
		}
		destroyRequest(requestRead);
	}
	
	//Build the category base
	if((requestRead = createRequest(coldDB, "SELECT "DBNAMETOID(RDB_CAT_ID)", "DBNAMETOID(RDB_CAT_rootID)", "DBNAMETOID(RDB_CAT_name)" FROM "TABLE_CATEGORY)) != NULL)
	{
		requestWrite = catUpdateQuery(mainCache, false);
		
		if(requestWrite != NULL)
		{
			
			while(sqlite3_step(requestRead) == SQLITE_ROW)
			{
				sqlite3_bind_int(requestWrite, 1, sqlite3_column_int(requestRead, 0));
				sqlite3_bind_int(requestWrite, 2, sqlite3_column_int(requestRead, 1));
				sqlite3_bind_text(requestWrite, 3, (void *) sqlite3_column_text(requestRead, 2), -1, SQLITE_STATIC);
				
				for(byte i = 0; i < 32; i++)
				{
					sqlite3_bind_int(requestWrite, i + 4, sqlite3_column_int(requestRead, i + 3));
				}
				
				if(sqlite3_step(requestWrite) != SQLITE_DONE)
				{
#ifdef EXTENSIVE_LOGGING
					uint ID = (uint32_t) sqlite3_column_int(requestRead, 0);
					const unsigned char * text = sqlite3_column_text(requestRead, 2);
					char logMessage[100 + (text != NULL ? ustrlen(text) : 0)];
					
					if(text == NULL)
						snprintf(logMessage, sizeof(logMessage), "Error building the category DB for ID %d: no text!", ID);
					else
						snprintf(logMessage, sizeof(logMessage), "Error building the category DB for ID %d of text %s!", ID, text);
					
					logR(logMessage);
#endif
				}
				sqlite3_reset(requestWrite);
			}
			destroyRequest(requestWrite);
		}
		destroyRequest(requestRead);
	}
	sqlite3_close(coldDB);
}

TAG * duplicateTag(TAG * tagToDuplicate, uint32_t nbTag)
{
	TAG * output = malloc(nbTag * sizeof(TAG));
	if(output != NULL)
	{
		memcpy(output, tagToDuplicate, nbTag * sizeof(TAG));
	}

	return output;
}

#pragma mark - Query API

uint getRootCategoryIDForID(uint32_t categoryID)
{
	sqlite3_stmt * request = NULL;
	uint output;

	if(categoryID == CAT_NO_VALUE)
		return CAT_NO_VALUE;

	else if((request = createRequest(cache != NULL ? cache : immatureCache, "SELECT "DBNAMETOID(RDB_CAT_rootID)" FROM "TABLE_CATEGORY" WHERE "DBNAMETOID(RDB_CAT_ID)" = ?1;")) == NULL || sqlite3_step(request) != SQLITE_ROW)
		output = (uint32_t) sqlite3_column_int(request, 0);
	
	else
		output = CAT_NO_VALUE;
	
	destroyRequest(request);

	return output;
}

charType * getNameForRequestAndCode(sqlite3_stmt * request, uint32_t code)
{
	sqlite3_bind_int(request, 1, (int32_t) code);
	
	if(sqlite3_step(request) != SQLITE_ROW)
	{
		destroyRequest(request);
		return L"Unknown";
	}
	
	charType * output = getStringFromUTF8(sqlite3_column_text(request, 0));
	
	destroyRequest(request);
	
	return output;
}

charType * getCatNameForCode(uint32_t catID)
{
	if(cache == NULL || catID == CAT_NO_VALUE)
		return NULL;
	
	sqlite3_stmt * request;
	
	if((request = createRequest(cache, "SELECT "DBNAMETOID(RDB_CAT_name)" FROM "TABLE_CATEGORY" WHERE "DBNAMETOID(RDB_CAT_ID)" = ?1")) == NULL)
		return NULL;

	return getNameForRequestAndCode(request, catID);
}

bool doesCatOfIDExist(uint32_t catID)
{
	if(cache == NULL && immatureCache == NULL)
		return false;

	if(catID == CAT_NO_VALUE)
		return true;

	sqlite3_stmt * request;

	if((request = createRequest(cache == NULL ? immatureCache : cache, "SELECT COUNT() FROM "TABLE_CATEGORY" WHERE "DBNAMETOID(RDB_CAT_ID)" = ?1")) == NULL)
		return false;

	sqlite3_bind_int(request, 1, (int32_t) catID);

	bool retValue = (sqlite3_step(request) == SQLITE_ROW && sqlite3_column_int(request, 0) != 0);

	destroyRequest(request);

	return retValue;
}

charType * getTagNameForCode(uint32_t tagID)
{
	if(cache == NULL || tagID == CAT_NO_VALUE)
		return NULL;
	
	sqlite3_stmt * request;
	
	if((request = createRequest(cache, "SELECT "DBNAMETOID(RDB_tagName)" FROM "TABLE_TAGS" WHERE "DBNAMETOID(RDB_tagID)" = ?1")) == NULL)
		return NULL;
	
	return getNameForRequestAndCode(request, tagID);
}

bool doesTagOfIDExist(uint32_t tagID)
{
	if(cache == NULL && immatureCache == NULL)
		return false;

	if(tagID == CAT_NO_VALUE)
		return true;

	sqlite3_stmt * request;

	if((request = createRequest(cache == NULL ? immatureCache : cache, "SELECT COUNT() FROM "TABLE_TAGS" WHERE "DBNAMETOID(RDB_tagID)" = ?1")) == NULL)
		return false;

	sqlite3_bind_int(request, 1, (int32_t) tagID);

	bool retValue = (sqlite3_step(request) == SQLITE_ROW && sqlite3_column_int(request, 0) != 0);

	destroyRequest(request);

	return retValue;
}

bool getCopyOfTagsOrCat(bool wantTag, void ** newData, uint * nbData)
{
	//Check data sanity, create request...
	if(newData == NULL || nbData == NULL)
		return false;

	*newData = NULL;
	*nbData = 0;

	sqlite3_stmt * request;

	if(wantTag)
		request = createRequest(cache, "SELECT * FROM "TABLE_TAGS" ORDER BY "DBNAMETOID(RDB_tagName)" COLLATE "SORT_FUNC" ASC;");
	else
		request = createRequest(cache, "SELECT "DBNAMETOID(RDB_CAT_ID)", "DBNAMETOID(RDB_CAT_name)" FROM "TABLE_CATEGORY" ORDER BY "DBNAMETOID(RDB_CAT_name)" COLLATE "SORT_FUNC" ASC;");

	if(request == NULL)
		return false;

	uint currentSize = 1023, currentPos = 0, dataSize = wantTag ? sizeof(TAG_VERBOSE) : sizeof(CATEGORY_VERBOSE);
	void * buffer = malloc(currentSize * dataSize);
	if(buffer == NULL)
	{
		destroyRequest(request);
		return false;
	}

	//Main loop
	while(sqlite3_step(request) == SQLITE_ROW)
	{
		//We increase our buffer size if needed
		if(currentPos == currentSize)
		{
			currentSize += 1024;
			void * tmp = realloc(buffer, currentSize * dataSize);

			if(tmp == NULL)
			{
				if(wantTag)
				{
					while(currentPos-- > 0)
						free(((TAG_VERBOSE *) buffer)[currentPos].name);
				}
				else
				{
					while(currentPos-- > 0)
						free(((CATEGORY_VERBOSE *) buffer)[currentPos].name);
				}

				free(buffer);
				destroyRequest(request);
				return false;
			}
			else
				buffer = tmp;
		}

		if(wantTag)
		{
			((TAG_VERBOSE *) buffer)[currentPos].ID = (uint) sqlite3_column_int(request, 0);
			((TAG_VERBOSE *) buffer)[currentPos].name = getStringFromUTF8(sqlite3_column_text(request, 1));

			if(((TAG_VERBOSE *) buffer)[currentPos].name != NULL)
				currentPos++;
		}
		else
		{
			((CATEGORY_VERBOSE *) buffer)[currentPos].ID = (uint) sqlite3_column_int(request, 0);
			((CATEGORY_VERBOSE *) buffer)[currentPos].rootID = CAT_NO_VALUE;
			((CATEGORY_VERBOSE *) buffer)[currentPos].name = getStringFromUTF8(sqlite3_column_text(request, 1));

			if(((CATEGORY_VERBOSE *) buffer)[currentPos].name != NULL)
				currentPos++;
		}
	}

	//We reduce the allocated memory to the minimum
	if(currentPos == 0)
	{
		free(buffer);
		buffer = NULL;
	}
	else if(currentPos < currentSize)
	{
		void * tmp = realloc(buffer, currentPos * dataSize);
		if(tmp != NULL)
			buffer = tmp;
	}

	*newData = buffer;
	*nbData = currentPos;
	
	return true;
}

bool getCopyOfTags(TAG_VERBOSE ** newData, uint * nbData)
{
	return getCopyOfTagsOrCat(true, (void **) newData, nbData);
}

bool getCopyOfCats(CATEGORY_VERBOSE ** newData, uint * nbData)
{
	return getCopyOfTagsOrCat(false, (void **) newData, nbData);
}
