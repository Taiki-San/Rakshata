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
	sqlite3_stmt * request = NULL;

	char body[] = "INTO "TABLE_TAGS" ("DBNAMETOID(RDB_tagID)", "DBNAMETOID(RDB_tagName)") VALUES (?1, ?2);", mainRequest[sizeof(body) + 50];
	snprintf(mainRequest, sizeof(mainRequest), "INSERT %s %s", shouldUpdate ? " OR REPLACE" : "", body);
	
	if(createRequest(db, mainRequest, &request) != SQLITE_OK)
		return NULL;
	
	return request;
}

sqlite3_stmt * catUpdateQuery(sqlite3 * db, bool shouldUpdate)
{
	sqlite3_stmt * request = NULL;
	
	char body[] = "INTO "TABLE_CATEGORY" ("DBNAMETOID(RDB_CAT_ID)", "DBNAMETOID(RDB_CAT_rootID)", "DBNAMETOID(RDB_CAT_name)", "DBNAMETOID(RDB_CAT_tag1)", "DBNAMETOID(RDB_CAT_tag2)", "DBNAMETOID(RDB_CAT_tag3)", "DBNAMETOID(RDB_CAT_tag4)", "DBNAMETOID(RDB_CAT_tag5)", "DBNAMETOID(RDB_CAT_tag6)", "DBNAMETOID(RDB_CAT_tag7)", "DBNAMETOID(RDB_CAT_tag8)", "DBNAMETOID(RDB_CAT_tag9)", "DBNAMETOID(RDB_CAT_tag10)", "DBNAMETOID(RDB_CAT_tag11)", "DBNAMETOID(RDB_CAT_tag12)", "DBNAMETOID(RDB_CAT_tag13)", "DBNAMETOID(RDB_CAT_tag14)", "DBNAMETOID(RDB_CAT_tag15)", "DBNAMETOID(RDB_CAT_tag16)", "DBNAMETOID(RDB_CAT_tag17)", "DBNAMETOID(RDB_CAT_tag18)", "DBNAMETOID(RDB_CAT_tag19)", "DBNAMETOID(RDB_CAT_tag20)", "DBNAMETOID(RDB_CAT_tag21)", "DBNAMETOID(RDB_CAT_tag22)", "DBNAMETOID(RDB_CAT_tag23)", "DBNAMETOID(RDB_CAT_tag24)", "DBNAMETOID(RDB_CAT_tag25)", "DBNAMETOID(RDB_CAT_tag26)", "DBNAMETOID(RDB_CAT_tag27)", "DBNAMETOID(RDB_CAT_tag28)", "DBNAMETOID(RDB_CAT_tag29)", "DBNAMETOID(RDB_CAT_tag30)", "DBNAMETOID(RDB_CAT_tag31)", "DBNAMETOID(RDB_CAT_tag32)") VALUES (?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12, ?13, ?14, ?15, ?16, ?17, ?18, ?19, ?20, ?21, ?22, ?23, ?24, ?25, ?26, ?27, ?28, ?29, ?30, ?31, ?32, ?33, ?34, ?35);", mainRequest[sizeof(body) + 50];
	
	snprintf(mainRequest, sizeof(mainRequest), "INSERT %s %s", shouldUpdate ? " OR REPLACE" : "", body);
	
	if(createRequest(db, mainRequest, &request) != SQLITE_OK)
		return NULL;
	
	return request;
}

#pragma mark - Internal API

bool createTagsTable(sqlite3 * mainCache)
{
	if(mainCache == NULL)
	{
		logR("Initialization is incomplete!");
		return false;
	}
	
	sqlite3_stmt * request = NULL;
	
	if(createRequest(mainCache, "CREATE TABLE "TABLE_TAGS" ("DBNAMETOID(RDB_tagID)" INTEGER PRIMARY KEY NOT NULL, "DBNAMETOID(RDB_tagName)" TEXT NOT NULL); CREATE INDEX hopperIsGud ON "TABLE_TAGS"("DBNAMETOID(RDB_tagID)");", &request) != SQLITE_OK || sqlite3_step(request) != SQLITE_DONE)
	{
		logR("Initialization error small");
		
		destroyRequest(request);

		return false;
	}
	
	destroyRequest(request);

	if(createRequest(mainCache, "CREATE TABLE "TABLE_CATEGORY" ("DBNAMETOID(RDB_CAT_ID)" INTEGER PRIMARY KEY NOT NULL, "DBNAMETOID(RDB_CAT_rootID)" INTEGER, "DBNAMETOID(RDB_CAT_name)" TEXT NOT NULL, "DBNAMETOID(RDB_CAT_tag1)" INTEGER, "DBNAMETOID(RDB_CAT_tag2)" INTEGER, "DBNAMETOID(RDB_CAT_tag3)" INTEGER, "DBNAMETOID(RDB_CAT_tag4)" INTEGER, "DBNAMETOID(RDB_CAT_tag5)" INTEGER, "DBNAMETOID(RDB_CAT_tag6)" INTEGER, "DBNAMETOID(RDB_CAT_tag7)" INTEGER, "DBNAMETOID(RDB_CAT_tag8)" INTEGER, "DBNAMETOID(RDB_CAT_tag9)" INTEGER, "DBNAMETOID(RDB_CAT_tag10)" INTEGER, "DBNAMETOID(RDB_CAT_tag11)" INTEGER, "DBNAMETOID(RDB_CAT_tag12)" INTEGER, "DBNAMETOID(RDB_CAT_tag13)" INTEGER, "DBNAMETOID(RDB_CAT_tag14)" INTEGER, "DBNAMETOID(RDB_CAT_tag15)" INTEGER, "DBNAMETOID(RDB_CAT_tag16)" INTEGER, "DBNAMETOID(RDB_CAT_tag17)" INTEGER, "DBNAMETOID(RDB_CAT_tag18)" INTEGER, "DBNAMETOID(RDB_CAT_tag19)" INTEGER, "DBNAMETOID(RDB_CAT_tag20)" INTEGER, "DBNAMETOID(RDB_CAT_tag21)" INTEGER, "DBNAMETOID(RDB_CAT_tag22)" INTEGER, "DBNAMETOID(RDB_CAT_tag23)" INTEGER, "DBNAMETOID(RDB_CAT_tag24)" INTEGER, "DBNAMETOID(RDB_CAT_tag25)" INTEGER, "DBNAMETOID(RDB_CAT_tag26)" INTEGER, "DBNAMETOID(RDB_CAT_tag27)" INTEGER, "DBNAMETOID(RDB_CAT_tag28)" INTEGER, "DBNAMETOID(RDB_CAT_tag29)" INTEGER, "DBNAMETOID(RDB_CAT_tag30)" INTEGER, "DBNAMETOID(RDB_CAT_tag31)" INTEGER, "DBNAMETOID(RDB_CAT_tag32)" INTEGER); CREATE INDEX gdbOverlldb ON "TABLE_CATEGORY"("DBNAMETOID(RDB_CAT_ID)", "DBNAMETOID(RDB_CAT_rootID)");", &request) != SQLITE_OK || sqlite3_step(request) != SQLITE_DONE)
	{
		logR("Initialization error big");

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
		sqlite3_bind_int(request, 1, newData[i].ID);
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
		sqlite3_bind_int(request, 1, newData[i].ID);
		sqlite3_bind_int(request, 2, newData[i].rootID);
		sqlite3_bind_text(request, 3, utf8, length, SQLITE_STATIC);
		
		for(byte tagCode = 0; tagCode < 32; tagCode++)
		{
			sqlite3_bind_int(request, tagCode + 4, newData[i].tags[tagCode].ID);
		}

		
		sqlite3_step(request);
		sqlite3_reset(request);
	}
	
	destroyRequest(request);
}

#pragma mark - Main API

void tagUpdateCachedEntry(TAG_VERBOSE * newData, uint nbData)
{
	if(newData == NULL || nbData == 0)
		return;

	tagUpdateCachedEntryWithRequest(tagUpdateQuery(cache, true), newData, nbData);
}

void catUpdateCachedEntry(CATEGORY_VERBOSE * newData, uint nbData)
{
	if(newData == NULL || nbData == 0)
		return;
	
	catUpdateCachedEntryWithRequest(catUpdateQuery(cache, true), newData, nbData);
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
	
	if(sqlite3_open(TAG_DB , &coldDB) != SQLITE_OK)
	{
		//Error, we should reset it with the version we ship with then trigger an update
		if(!resetTagsToLocal())
			return;
		else if(sqlite3_open(TAG_DB , &coldDB) != SQLITE_OK)
		{
			logR("We have significant issues setting up our environment, this may be caused by permission issues, please contact us at contact@rakshata.com");
			exit(0);
		}
	}
	
	//Create the tables in the main cache
	createTagsTable(mainCache);
	
	sqlite3_stmt * requestRead, *requestWrite;
	
	//Build the tag base
	if(createRequest(coldDB, "SELECT "DBNAMETOID(RDB_tagID)", "DBNAMETOID(RDB_tagName)" FROM "TABLE_TAGS, &requestRead) == SQLITE_OK)
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
#ifdef DEV_VERSION
					uint ID = sqlite3_column_int(requestRead, 0);
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
	if(createRequest(coldDB, "SELECT "DBNAMETOID(RDB_CAT_ID)", "DBNAMETOID(RDB_CAT_rootID)", "DBNAMETOID(RDB_CAT_name)", "DBNAMETOID(RDB_CAT_tag1)", "DBNAMETOID(RDB_CAT_tag2)", "DBNAMETOID(RDB_CAT_tag3)", "DBNAMETOID(RDB_CAT_tag4)", "DBNAMETOID(RDB_CAT_tag5)", "DBNAMETOID(RDB_CAT_tag6)", "DBNAMETOID(RDB_CAT_tag7)", "DBNAMETOID(RDB_CAT_tag8)", "DBNAMETOID(RDB_CAT_tag9)", "DBNAMETOID(RDB_CAT_tag10)", "DBNAMETOID(RDB_CAT_tag11)", "DBNAMETOID(RDB_CAT_tag12)", "DBNAMETOID(RDB_CAT_tag13)", "DBNAMETOID(RDB_CAT_tag14)", "DBNAMETOID(RDB_CAT_tag15)", "DBNAMETOID(RDB_CAT_tag16)", "DBNAMETOID(RDB_CAT_tag17)", "DBNAMETOID(RDB_CAT_tag18)", "DBNAMETOID(RDB_CAT_tag19)", "DBNAMETOID(RDB_CAT_tag20)", "DBNAMETOID(RDB_CAT_tag21)", "DBNAMETOID(RDB_CAT_tag22)", "DBNAMETOID(RDB_CAT_tag23)", "DBNAMETOID(RDB_CAT_tag24)", "DBNAMETOID(RDB_CAT_tag25)", "DBNAMETOID(RDB_CAT_tag26)", "DBNAMETOID(RDB_CAT_tag27)", "DBNAMETOID(RDB_CAT_tag28)", "DBNAMETOID(RDB_CAT_tag29)", "DBNAMETOID(RDB_CAT_tag30)", "DBNAMETOID(RDB_CAT_tag31)", "DBNAMETOID(RDB_CAT_tag32)" FROM "TABLE_CATEGORY, &requestRead) == SQLITE_OK)
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
#ifdef DEV_VERSION
					uint ID = sqlite3_column_int(requestRead, 0);
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

#pragma mark - Query API

CATEGORY getCategoryForID(uint32_t categoryID)
{
	CATEGORY retValue;
	sqlite3_stmt * request = NULL;

	if(cache == NULL && immatureCache == NULL)
	{
		logR("Incomplete initialization");
		
		retValue.haveData = false;
	}
	
	else if(createRequest(cache == NULL ? immatureCache : cache, "SELECT "DBNAMETOID(RDB_CAT_tag1)", "DBNAMETOID(RDB_CAT_tag2)", "DBNAMETOID(RDB_CAT_tag3)", "DBNAMETOID(RDB_CAT_tag4)", "DBNAMETOID(RDB_CAT_tag5)", "DBNAMETOID(RDB_CAT_tag6)", "DBNAMETOID(RDB_CAT_tag7)", "DBNAMETOID(RDB_CAT_tag8)", "DBNAMETOID(RDB_CAT_tag9)", "DBNAMETOID(RDB_CAT_tag10)", "DBNAMETOID(RDB_CAT_tag11)", "DBNAMETOID(RDB_CAT_tag12)", "DBNAMETOID(RDB_CAT_tag13)", "DBNAMETOID(RDB_CAT_tag14)", "DBNAMETOID(RDB_CAT_tag15)", "DBNAMETOID(RDB_CAT_tag16)", "DBNAMETOID(RDB_CAT_tag17)", "DBNAMETOID(RDB_CAT_tag18)", "DBNAMETOID(RDB_CAT_tag19)", "DBNAMETOID(RDB_CAT_tag20)", "DBNAMETOID(RDB_CAT_tag21)", "DBNAMETOID(RDB_CAT_tag22)", "DBNAMETOID(RDB_CAT_tag23)", "DBNAMETOID(RDB_CAT_tag24)", "DBNAMETOID(RDB_CAT_tag25)", "DBNAMETOID(RDB_CAT_tag26)", "DBNAMETOID(RDB_CAT_tag27)", "DBNAMETOID(RDB_CAT_tag28)", "DBNAMETOID(RDB_CAT_tag29)", "DBNAMETOID(RDB_CAT_tag30)", "DBNAMETOID(RDB_CAT_tag31)", "DBNAMETOID(RDB_CAT_tag32)" FROM "TABLE_CATEGORY" WHERE "DBNAMETOID(RDB_CAT_ID)" = ?1", &request) == SQLITE_OK && sqlite3_step(request) == SQLITE_ROW)
	{
		
		retValue.haveData = true;
		retValue.ID = categoryID;
		
		for(byte i = 0; i < 32; i++)
			retValue.tags[i].ID = sqlite3_column_int(request, i);
		
		destroyRequest(request);
	}
	else
	{
		logR("Invalid request, no such ID");
		
		retValue.haveData = false;

		destroyRequest(request);
	}
	
	return retValue;
}

uint getRootCategoryIDForID(uint32_t categoryID)
{
	sqlite3_stmt * request = NULL;
	uint output;

	if(categoryID == TAG_NO_VALUE)
		return TAG_NO_VALUE;

	else if(createRequest(cache != NULL ? cache : immatureCache, "SELECT "DBNAMETOID(RDB_CAT_rootID)" FROM "TABLE_CATEGORY" WHERE "DBNAMETOID(RDB_CAT_ID)" = ?1;", &request) != SQLITE_OK || sqlite3_step(request) != SQLITE_ROW)
		output = sqlite3_column_int(request, 0);
	
	else
		output = TAG_NO_VALUE;
	
	destroyRequest(request);

	return output;
}

charType * getNameForRequestAndCode(sqlite3_stmt * request, uint32_t code)
{
	sqlite3_bind_int(request, 1, code);
	
	if(sqlite3_step(request) != SQLITE_ROW)
		return L"Unknown category";
	
	const unsigned char * rawString = sqlite3_column_text(request, 0);
	charType * output = NULL;
	
	if(rawString != NULL)
	{
		size_t length = ustrlen(rawString);
		output = malloc((length + 1) * sizeof(charType));
		
		if(output != NULL)
		{
			length = utf8_to_wchar((const char *) rawString, length, output, length + 1, 0);
			output[length] = 0;
		}
	}
	
	destroyRequest(request);
	
	return output;
}

charType * getCatNameForCode(uint32_t catID)
{
	if(cache == NULL)
		return NULL;
	
	sqlite3_stmt * request;
	
	if(createRequest(cache, "SELECT "DBNAMETOID(RDB_CAT_name)" FROM "TABLE_CATEGORY" WHERE "DBNAMETOID(RDB_CAT_ID)" = ?1", &request) != SQLITE_OK)
		return NULL;

	return getNameForRequestAndCode(request, catID);
}

charType * getTagNameForCode(uint32_t tagID)
{
	if(cache == NULL)
		return NULL;
	
	sqlite3_stmt * request;
	
	if(createRequest(cache, "SELECT "DBNAMETOID(RDB_tagName)" FROM "TABLE_TAGS" WHERE "DBNAMETOID(RDB_tagID)" = ?1", &request) != SQLITE_OK)
		return NULL;
	
	return getNameForRequestAndCode(request, tagID);
}
