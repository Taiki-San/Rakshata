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

bool copyOutputDBToStruct(sqlite3_stmt *state, PROJECT_DATA* output, bool copyDynamic, bool evenWantTags)
{
	void* buffer;

	*output = getEmptyProject();

	//Repo
	uint64_t repoID = (uint64_t) sqlite3_column_int64(state, RDB_repo-1);
	buffer = getRepoForID(repoID);
	if(buffer != NULL)				//Si la team est pas valable, on drop complètement le projet
	{
		output->repo = buffer;
	}
	else if(repoID != LOCAL_REPO_ID)
	{
		MUTEX_UNLOCK(cacheParseMutex);
		return false;
	}
	
	//ID d'accès rapide
	output->cacheDBID = (uint32_t) sqlite3_column_int(state, RDB_ID-1);
	
	//Project ID
	output->projectID = (uint32_t) sqlite3_column_int(state, RDB_projectID-1);
	
	//isInstalled est ici, on saute donc son index
	
	//Nom du projet
	buffer = (void*) sqlite3_column_text(state, RDB_projectName-1);
	if(buffer == NULL)
	{
		MUTEX_UNLOCK(cacheParseMutex);
		return false;
	}
	else
	{
		wchar_t converted[LENGTH_PROJECT_NAME] = {0};
		
		utf8_to_wchar(buffer, strlen(buffer), converted, LENGTH_PROJECT_NAME - 1, 0);
		memcpy(&(output->projectName), converted, sizeof(converted));
	}
	
	//Description
	buffer = (void*) sqlite3_column_text(state, RDB_description-1);
	if(buffer == NULL)
		memset(output->description, 0, sizeof(output->description));
	else
	{
		wchar_t converted[LENGTH_DESCRIPTION] = {0};
		
		utf8_to_wchar(buffer, strlen(buffer), converted, LENGTH_DESCRIPTION - 1, 0);
		memcpy(&(output->description), converted, sizeof(converted));
	}
	
	//Nom de l'auteur
	buffer = (void*) sqlite3_column_text(state, RDB_authors-1);
	if(buffer == NULL)
		memset(output->authorName, 0, sizeof(output->authorName));
	else
	{
		wchar_t converted[LENGTH_AUTHORS] = {0};
		
		utf8_to_wchar(buffer, strlen(buffer), converted, LENGTH_AUTHORS - 1, 0);
		memcpy(&(output->authorName), converted, sizeof(converted));
	}
	
	//Divers données
	output->status = sqlite3_column_int(state, RDB_status-1);	//On pourrait vérifier que c'est une valeur tolérable mais je ne vois pas de raison pour laquelle quelqu'un irait patcher la BDD
	output->category = (uint32_t) sqlite3_column_int(state, RDB_category-1);
	output->rightToLeft = sqlite3_column_int(state, RDB_asianOrder-1);
	output->isPaid = sqlite3_column_int(state, RDB_isPaid-1);
	output->mainTag = (uint32_t) sqlite3_column_int(state, RDB_mainTagID-1);

	if(evenWantTags)
	{
		output->nbTags = (uint32_t) sqlite3_column_int(state, RDB_nbTagData-1);
		output->tags = duplicateTag((void *) sqlite3_column_int64(state, RDB_tagData-1), output->nbTags);
	}
	else
	{
		output->nbTags = 0;
		output->tags = NULL;
	}

	output->nombreChapitre = (uint32_t) sqlite3_column_int(state, RDB_nombreChapitre-1);
	output->locale = (bool) sqlite3_column_int(state, RDB_isLocal-1);
	
	if(copyDynamic)
	{
		buffer = (void*) sqlite3_column_int64(state, RDB_chapitresPrice - 1);
		if(buffer != NULL)
		{
			output->chapitresPrix = malloc(output->nombreChapitre * sizeof(uint));
			if(output->chapitresPrix != NULL)
				memcpy(output->chapitresPrix, buffer, output->nombreChapitre * sizeof(int));
			else
				output->chapitresPrix = NULL;
		}
		else
			output->chapitresPrix = NULL;
		
		buffer = (void*) sqlite3_column_int64(state, RDB_chapitres-1);
		if(buffer != NULL)
		{
			output->chapitresFull = malloc(output->nombreChapitre * sizeof(int));
			if(output->chapitresFull != NULL)
			{
				memcpy(output->chapitresFull, buffer, output->nombreChapitre * sizeof(int));
				output->chapitresInstalled = NULL;
				getChapterInstalled(output, NULL);
			}
			else
				output->chapitresInstalled = NULL;
		}
		else
		{
			output->chapitresFull = output->chapitresInstalled = NULL;
			output->nombreChapitreInstalled = 0;
			
			free(output->chapitresPrix);	output->chapitresPrix = NULL;
		}
	}
	else
	{
		output->chapitresFull = NULL;
		output->chapitresInstalled = NULL;
		output->chapitresPrix = NULL;
	}
	
	output->nombreTomes = (uint32_t) sqlite3_column_int(state, RDB_nombreTomes-1);
	output->haveDRM = sqlite3_column_int(state, RDB_DRM-1);
	
	if(copyDynamic)
	{
		buffer = (void*) sqlite3_column_int64(state, RDB_tomes-1);
		if(buffer != NULL)
		{
			output->tomesFull = malloc(output->nombreTomes * sizeof(META_TOME));
			if(output->tomesFull != NULL)
			{
				copyTomeList(buffer, output->nombreTomes, output->tomesFull);
				output->tomesInstalled = NULL;
				getTomeInstalled(output, NULL);
			}
			else
			{
				output->tomesInstalled = NULL;
				output->nombreTomesInstalled = 0;
			}
		}
		else
		{
			output->tomesFull = NULL;
			output->tomesInstalled = NULL;
			output->nombreTomesInstalled = 0;
		}
	}
	else
	{
		output->tomesFull = NULL;
		output->tomesInstalled = NULL;
	}
	
	output->favoris = sqlite3_column_int(state, RDB_favoris-1);
	output->isInitialized = true;
	
	return true;
}

bool copyParsedDBToStruct(sqlite3_stmt * state, PROJECT_DATA_PARSED * output, bool copyDynamic)
{
	if(state == NULL || output == NULL)
		return false;

	//Copy chapitreLocal
	output->nombreChapitreLocal = (uint32_t) sqlite3_column_int(state, RDB_chapitreLocalLength-1);

	void * buffer = (void*) sqlite3_column_int64(state, RDB_chapitreLocal-1);
	if(copyDynamic && buffer != NULL)
	{
		output->chapitresLocal = malloc(output->nombreChapitreLocal * sizeof(int));
		if(output->chapitresLocal != NULL)
			memcpy(output->chapitresLocal, buffer, output->nombreChapitreLocal * sizeof(int));
	}
	else
	{
		output->chapitresLocal = NULL;

		if(copyDynamic)
			output->nombreChapitreLocal = 0;
	}

	//Copy chapitreRemote
	output->nombreChapitreRemote = (uint32_t) sqlite3_column_int(state, RDB_chapitreRemoteLength-1);
	buffer = (void*) sqlite3_column_int64(state, RDB_chapitreRemote-1);
	if(copyDynamic && buffer != NULL)
	{
		output->chapitresRemote = malloc(output->nombreChapitreRemote * sizeof(int));
		if(output->chapitresRemote != NULL)
			memcpy(output->chapitresRemote, buffer, output->nombreChapitreRemote * sizeof(int));
	}
	else
	{
		output->chapitresRemote = NULL;

		if(copyDynamic)
			output->nombreChapitreRemote = 0;
	}

	//Copy tomeLocal
	output->nombreTomeLocal = (uint32_t) sqlite3_column_int(state, RDB_tomeLocalLength-1);
	buffer = (void*) sqlite3_column_int64(state, RDB_tomeLocal-1);
	if(copyDynamic && buffer != NULL)
	{
		output->tomeLocal = malloc(output->nombreTomeLocal * sizeof(META_TOME));
		if(output->tomeLocal != NULL)
			copyTomeList(buffer, output->nombreTomeLocal, output->tomeLocal);
		else
			output->nombreTomeLocal = 0;
	}
	else
	{
		output->tomeLocal = NULL;

		if(copyDynamic)
			output->nombreTomeLocal = 0;
	}

	//And, finally, tomeRemote
	output->nombreTomeRemote = (uint32_t) sqlite3_column_int(state, RDB_tomeRemoteLength-1);
	buffer = (void*) sqlite3_column_int64(state, RDB_tomeRemote-1);
	if(copyDynamic && buffer != NULL)
	{
		output->tomeRemote = malloc(output->nombreTomeRemote * sizeof(META_TOME));
		if(output->tomeRemote != NULL)
			copyTomeList(buffer, output->nombreTomeRemote, output->tomeRemote);
		else
			output->nombreTomeRemote = 0;
	}
	else
	{
		output->tomeRemote = NULL;

		if(copyDynamic)
			output->nombreTomeRemote = 0;
	}

	return true;
}

void * getCopyCache(uint maskRequest, uint* nbElemCopied)
{
	uint pos = 0;
	void * output = NULL;
	bool wantParsedOutput = maskRequest & RDB_PARSED_OUTPUT, copyDynamic = !(maskRequest & RDB_EXCLUDE_DYNAMIC), wantTags = maskRequest & RDB_INCLUDE_TAGS;
	
	if(nbElemCopied != NULL)
		*nbElemCopied = 0;
	
	if(cache == NULL && !setupBDDCache())	//Échec du chargement
		return NULL;
	
	output = malloc((nbElemInCache + 1) * (wantParsedOutput ? sizeof(PROJECT_DATA_PARSED) : sizeof(PROJECT_DATA)));	//Unused memory seems to stay on the pool, so we can ask for more than really needed in the case we only want installed stuffs
	if(output != NULL)
	{
		char sortRequest[50], requestString[300], searchCond[200];
		int lengthWritten = 0;

		//Sort requirement
		if(maskRequest & SORT_ID)
			strncpy(sortRequest, DBNAMETOID(RDB_ID), 50);
		else if(maskRequest & SORT_REPO)
			strncpy(sortRequest, DBNAMETOID(RDB_repo), 50);
		else
			strncpy(sortRequest, DBNAMETOID(RDB_projectName)" COLLATE "SORT_FUNC, 50);

		//Type of data we look for
		if(maskRequest & RDB_LOADINSTALLED)
			lengthWritten += snprintf(searchCond, 200, DBNAMETOID(RDB_isInstalled)" = 1");
		else if(maskRequest & RDB_LOAD_FAVORITE)
			lengthWritten += snprintf(searchCond, 200, DBNAMETOID(RDB_favoris)" = 1");

		//Don't want local data
		if(maskRequest & RDB_REMOTE_ONLY)
		{
			if(lengthWritten)
				lengthWritten += snprintf(&(searchCond[lengthWritten]), 200 - (uint) lengthWritten, " AND "DBNAMETOID(RDB_isLocal)" = 0");
			else
				lengthWritten = snprintf(searchCond, 200, DBNAMETOID(RDB_isLocal)" = 0");
		}

		//Craft the final request
		if(lengthWritten)
			snprintf(requestString, sizeof(requestString), "SELECT * FROM "MAIN_CACHE" WHERE %s ORDER BY %s ASC", searchCond, sortRequest);
		else
			snprintf(requestString, sizeof(requestString), "SELECT * FROM "MAIN_CACHE" ORDER BY %s ASC", sortRequest);
		
		sqlite3_stmt* request = createRequest(cache, requestString);
		
		MUTEX_LOCK(cacheParseMutex);
		
		while(pos < nbElemInCache && sqlite3_step(request) == SQLITE_ROW)
		{
			if(wantParsedOutput)
			{
				if(!copyOutputDBToStruct(request, &(((PROJECT_DATA_PARSED *) output)[pos].project), copyDynamic, wantTags)
				   || !copyParsedDBToStruct(request, &(((PROJECT_DATA_PARSED *) output)[pos]), copyDynamic))
					continue;

				if(((PROJECT_DATA_PARSED *) output)[pos].project.isInitialized)
					pos++;
			}
			else
			{
				if(!copyOutputDBToStruct(request, &((PROJECT_DATA *) output)[pos], copyDynamic, wantTags))
					continue;

				if(((PROJECT_DATA *) output)[pos].isInitialized)
					pos++;
			}
		}

		MUTEX_UNLOCK(cacheParseMutex);

		if (wantParsedOutput)
			((PROJECT_DATA_PARSED *) output)[pos] = getEmptyParsedProject();
		else
			((PROJECT_DATA *) output)[pos] = getEmptyProject();

		destroyRequest(request);
		
		if(nbElemCopied != NULL)
			*nbElemCopied = pos;
	}
	
	return output;
}

void * _getProjectFromSearch (uint64_t IDRepo, uint projectID, bool locale, bool installed, bool copyDynamic, bool wantParsed, bool wantTags)
{
	void * output = calloc(1, wantParsed ? sizeof(PROJECT_DATA_PARSED) : sizeof(PROJECT_DATA));
	if(output == NULL)
		return NULL;

	sqlite3_stmt* request = NULL;

	if(installed)
		request = createRequest(cache, "SELECT * FROM "MAIN_CACHE" WHERE "DBNAMETOID(RDB_repo)" = ?1 AND "DBNAMETOID(RDB_projectID)" = ?2 AND "DBNAMETOID(RDB_isLocal)" = ?3 AND "DBNAMETOID(RDB_isInstalled)" = 1");
	else
		request = createRequest(cache, "SELECT * FROM "MAIN_CACHE" WHERE "DBNAMETOID(RDB_repo)" = ?1 AND "DBNAMETOID(RDB_projectID)" = ?2 AND "DBNAMETOID(RDB_isLocal)" = ?3");

	sqlite3_bind_int64(request, 1, (int64_t) IDRepo);
	sqlite3_bind_int(request, 2, (int32_t) projectID);
	sqlite3_bind_int(request, 3, (int32_t) locale);

	if(sqlite3_step(request) == SQLITE_ROW)
	{
		if(wantParsed)
		{
			if(!copyOutputDBToStruct(request, &((PROJECT_DATA_PARSED *) output)->project, copyDynamic, wantTags)
			   || (copyDynamic && !copyParsedDBToStruct(request, (PROJECT_DATA_PARSED *) output, copyDynamic)))
			{
				free(output);
				output = NULL;
			}
		}
		else if(!copyOutputDBToStruct(request, output, copyDynamic, wantTags))
		{
			free(output);
			output = NULL;
		}

		if(sqlite3_step(request) == SQLITE_ROW)
		{
#ifdef DEV_VERSION
			printf("Project was %ls\n", wantParsed ? ((PROJECT_DATA_PARSED *) output)->project.projectName : ((PROJECT_DATA *) output)->projectName);
#endif

			if(wantParsed)
				releaseParsedData(* (PROJECT_DATA_PARSED *) output);
			else
				releaseCTData(* (PROJECT_DATA *) output);

			free(output);
			output = NULL;
			logR("[Error]: Too much results to request, it was supposed to be unique, someone isn't respecting the standard ><");
		}
	}
	else
	{
		free(output);
		output = NULL;

		if(!installed)
			logR("[Error]: Request not found, something went wrong when parsing the data :/");
	}

	destroyRequest(request);

	return output;
}

PROJECT_DATA * getProjectFromSearch (uint64_t IDRepo, uint projectID, bool locale, bool installed)
{
	return _getProjectFromSearch(IDRepo, projectID, locale, installed, true, false, false);
}

PROJECT_DATA_PARSED getProjectByIDHelper(uint cacheID, bool copyDynamic, bool wantParsed, bool wantTags)
{
	PROJECT_DATA_PARSED output = getEmptyParsedProject();

	if(cache != NULL && cacheID != UINT_MAX)
	{
		sqlite3_stmt* request = createRequest(cache, "SELECT * FROM "MAIN_CACHE" WHERE "DBNAMETOID(RDB_ID)" = ?1");
		sqlite3_bind_int(request, 1, (int32_t) cacheID);

		MUTEX_LOCK(cacheParseMutex);

		if(sqlite3_step(request) == SQLITE_ROW && copyOutputDBToStruct(request, &output.project, copyDynamic, wantTags) && wantParsed)
		{
			copyParsedDBToStruct(request, &output, copyDynamic);
		}

		MUTEX_UNLOCK(cacheParseMutex);

		destroyRequest(request);
	}

	return output;
}

PROJECT_DATA_PARSED getParsedProjectByID(uint cacheID)
{
	return getProjectByIDHelper(cacheID, true, true, false);
}

PROJECT_DATA getProjectByID(uint cacheID)
{
	return getProjectByIDHelper(cacheID, true, false, false).project;
}

uint * getFavoritesID(uint * nbFavorites)
{
	if(nbFavorites == NULL || cache == NULL)
		return NULL;

	uint * output = malloc(nbElemInCache * sizeof(uint));	//nbMax of entries
	if(output == NULL)
		return NULL;

	sqlite3_stmt * request = createRequest(cache, "SELECT "DBNAMETOID(RDB_ID)" FROM "MAIN_CACHE" WHERE "DBNAMETOID(RDB_favoris)" = 1 ORDER BY "DBNAMETOID(RDB_ID)" ASC");

	if(request == NULL)
	{
		free(output);
		return NULL;
	}

	*nbFavorites = 0;
	while(sqlite3_step(request) == SQLITE_ROW && *nbFavorites < nbElemInCache)
		output[(*nbFavorites)++] = (uint32_t) sqlite3_column_int(request, 0);

	destroyRequest(request);

	if(*nbFavorites == 0)
	{
		free(output);	output = NULL;
	}
	else if(*nbFavorites != nbElemInCache)
	{
		void * tmp = realloc(output, *nbFavorites * sizeof(uint));
		if(tmp != NULL)
			output = tmp;
	}

	return output;
}

void setFavoriteForID(uint cacheID, bool isFavorite)
{
	sqlite3_stmt * request = createRequest(cache, "UPDATE "MAIN_CACHE" SET "DBNAMETOID(RDB_favoris)" = ?1 WHERE "DBNAMETOID(RDB_ID)" = ?2");
	if(request == NULL)
		return;

	sqlite3_bind_int(request, 1, isFavorite);
	sqlite3_bind_int(request, 2, (int32_t) cacheID);

	sqlite3_step(request);
	destroyRequest(request);
}

void * getUpdatedCTForID(uint cacheID, bool wantTome, size_t * nbElemUpdated, uint ** price)
{
	sqlite3_stmt* request = NULL;

	if(wantTome)
		request = createRequest(cache, "SELECT "DBNAMETOID(RDB_nombreTomes)", "DBNAMETOID(RDB_tomes)" FROM "MAIN_CACHE" WHERE "DBNAMETOID(RDB_ID)" = ?1");
	else
		request = createRequest(cache, "SELECT "DBNAMETOID(RDB_nombreChapitre)", "DBNAMETOID(RDB_chapitres)", "DBNAMETOID(RDB_chapitresPrice)" FROM "MAIN_CACHE" WHERE "DBNAMETOID(RDB_ID)" = ?1");

	sqlite3_bind_int(request, 1, (int32_t) cacheID);

	if(sqlite3_step(request) != SQLITE_ROW)
		return NULL;

	uint nbElemOut = (uint32_t) sqlite3_column_int(request, 0);
	void * output = NULL, * input = (void *) sqlite3_column_int64(request, 1);

	if(nbElemOut != 0 && input != NULL)
	{
		if(wantTome)
		{
			output = calloc(nbElemOut + 1, sizeof(META_TOME));

			if(output != NULL)
			{
				((META_TOME*)output)[nbElemOut].ID = INVALID_SIGNED_VALUE;		//Whatever copyTomeList may do, the array is valid by now
				copyTomeList((META_TOME*) input, nbElemOut, output);
			}
		}
		else
		{
			output = malloc((nbElemOut + 1) * sizeof(int));
			if(output != NULL)
			{
				memcpy(output, (int*) input, nbElemOut * sizeof(int));
				((int*) output)[nbElemOut] = INVALID_SIGNED_VALUE;				//In the case it was missing (kinda like a canary)
			}

			if(price != NULL)
			{
				void * data = (int*) sqlite3_column_int64(request, 2);
				if(data != NULL)
				{
					*price = malloc(nbElemOut * sizeof(uint));
					if(*price != NULL)
					{
						memcpy(*price, data, nbElemOut * sizeof(int));
					}
				}
				else
					*price = NULL;
			}
		}

		if(output != NULL && nbElemUpdated != NULL)
			*nbElemUpdated = nbElemOut;
	}
	else if(nbElemUpdated != NULL)
		*nbElemUpdated = 0;

	return output;
}

/*************		REPOSITORIES DATA		*****************/

uint64_t getRepoID(REPO_DATA * repo)
{
	if(isLocalRepo(repo))
		return LOCAL_REPO_ID;

	uint64_t output = repo->parentRepoID;;
	return (output << 32) | repo->repoID;
}

uint getRootFromRepoID(uint64_t repoID)
{
	return repoID >> 32;
}

uint getSubrepoFromRepoID(uint64_t repoID)
{
	return repoID & UINT_MAX;
}

uint64_t getRepoIndexFromURL(const char * URL)
{
	for(uint i = 0; i < lengthRepo; i++)
	{
		if(repoList[i] != NULL && !strcmp(repoList[i]->URL, URL))
			return getRepoID(repoList[i]);
	}

	return UINT64_MAX;
}

REPO_DATA * getRepoForID(uint64_t repoID)
{
	REPO_DATA * output = malloc(sizeof(REPO_DATA));
	
	if(output != NULL)
	{
		if(repoID == LOCAL_REPO_ID)
			return NULL;

		for(uint i = 0; i < lengthRepo; i++)
		{
			if(repoList[i] != NULL && getRepoID(repoList[i]) == repoID)
			{
				*output = *(repoList[i]);
				return output;
			}
		}
		
		free(output);
	}
	
	return NULL;
}

ROOT_REPO_DATA * getRootRepoForID(uint repoID)
{
	ROOT_REPO_DATA * output = malloc(sizeof(ROOT_REPO_DATA));
	
	if(output != NULL)
	{
		for(uint i = 0; i < lengthRootRepo; i++)
		{
			if(rootRepoList[i] != NULL && rootRepoList[i]->repoID == repoID)
			{
				if(copyRootRepo(*(rootRepoList[i]), output))
					return output;
				
				return NULL;
			}
		}
		
		free(output);
	}
	
	return NULL;
}

bool copyRootRepo(const ROOT_REPO_DATA original, ROOT_REPO_DATA * copy)
{
	if(copy != NULL)
	{
		*copy = original;

		//We need to alloc those ourselves
		copy->descriptions = NULL;
		copy->langueDescriptions = NULL;
		copy->subRepo = NULL;
	}
	else
		return false;
	
#ifdef FLUSH_UNUSED_REPO
	if(copy->nombreSubrepo == 0)
	{
		free(copy);
		return false;
	}
	else
	{
#endif
		copy->subRepo = calloc(copy->nombreSubrepo, sizeof(REPO_DATA));
		if(copy->subRepo == NULL)
		{
			free(copy);
			return false;
		}
#ifdef FLUSH_UNUSED_REPO
	}
#endif
	
	if(copy->subRepo != NULL)
		memcpy(copy->subRepo, original.subRepo, copy->nombreSubrepo * sizeof(REPO_DATA));
	
	//Yep, descriptions are a pain in the ass
	if(copy->nombreDescriptions > 0)
	{
		copy->descriptions = calloc(copy->nombreDescriptions, sizeof(charType*));
		copy->langueDescriptions = calloc(copy->nombreDescriptions, sizeof(char*));
		
		if(copy->descriptions != NULL && copy->langueDescriptions != NULL)
		{
			for(uint posDesc = 0; posDesc < copy->nombreDescriptions; posDesc++)
			{
				uint lengthDesc = wstrlen(original.descriptions[posDesc]), lengthLanguage = strlen(original.langueDescriptions[posDesc]);
				copy->descriptions[posDesc] = malloc((lengthDesc + 1) * sizeof(charType));
				copy->langueDescriptions[posDesc] = malloc((lengthLanguage + 1) * sizeof(char));
				
				if(copy->descriptions[posDesc] == NULL || copy->langueDescriptions[posDesc] == NULL)
				{
					do
					{
						free(copy->descriptions[posDesc]);
						free(copy->langueDescriptions[posDesc]);
					} while(posDesc-- > 0);
					
					free(copy->descriptions);
					free(copy->langueDescriptions);
					free(copy->subRepo);
					free(copy);
					
					return false;
				}
				else
				{
					memcpy(copy->descriptions[posDesc], original.descriptions[posDesc], lengthDesc * sizeof(charType));
					memcpy(copy->langueDescriptions[posDesc], original.langueDescriptions[posDesc], lengthLanguage * sizeof(char));
					copy->descriptions[posDesc][lengthDesc] = 0;
					copy->langueDescriptions[posDesc][lengthLanguage] = 0;
				}
			}
		}
		else
		{
			free(copy->descriptions);
			free(copy->langueDescriptions);
			free(copy->subRepo);
			free(copy);

			return false;
		}
	}
	
	return true;
}

void ** getCopyKnownRepo(uint * nbRepo, bool wantRoot)
{
	MUTEX_LOCK(cacheMutex);
	
	//+1 used to free everything
	uint sizeElem = wantRoot ? sizeof(ROOT_REPO_DATA) : sizeof(REPO_DATA), length = wantRoot ? lengthRootRepo : lengthRepo;
	void ** originalData = wantRoot ? ((void**)rootRepoList) : ((void**)repoList), ** output = calloc(length + 1, sizeof(void*));
	if(output != NULL)
	{
		uint discardedElement = 0;
		
		for(uint i = 0, index; i < length; i++)
		{
			index = i - discardedElement;
			
			if(originalData[i] == NULL)
				output[index] = NULL;
			else
			{
				output[index] = malloc(sizeElem);
				
				if(output[index] != NULL)
				{
					memcpy(output[index], originalData[i], sizeElem);
					
					if(wantRoot)
					{
						if(!copyRootRepo(*((ROOT_REPO_DATA *) originalData[i]), output[i]))
						{
							output[index] = NULL;
							discardedElement++;

							continue;
						}
					}
				}
				
				if(output[index] == NULL)	//Memory error, let's get the fuck out of here
				{
					if(wantRoot)
						freeRootRepo((ROOT_REPO_DATA**) output);
					else
					{
						for (; i > 0; free(output[--i]));
						free(output);
					}
					*nbRepo = 0;
					return NULL;
				}
			}
		}
		*nbRepo = length - discardedElement;
	}
	else
		*nbRepo = 0;
	
	MUTEX_UNLOCK(cacheMutex);
	
	return output;
}
