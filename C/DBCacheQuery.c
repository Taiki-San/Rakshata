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

bool copyOutputDBToStruct(sqlite3_stmt *state, PROJECT_DATA* output, bool copyDynamic)
{
	void* buffer;

	//Repo
	buffer = getRepoForID((uint64_t) sqlite3_column_int64(state, RDB_repo-1));
	if(buffer != NULL)				//Si la team est pas valable, on drop complètement le projet
		output->repo = buffer;
	else
	{
		output->isInitialized = false;
		output->repo = NULL;	//L'appelant est signalé d'ignorer l'élément
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
	output->tagMask = (uint64_t) sqlite3_column_int64(state, RDB_tagMask-1);
	output->nombreChapitre = (uint32_t) sqlite3_column_int(state, RDB_nombreChapitre-1);
	
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
				checkChapitreValable(output, NULL);
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
				checkTomeValable(output, NULL);
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

PROJECT_DATA * getCopyCache(uint maskRequest, uint* nbElemCopied)
{
	uint pos = 0;
	PROJECT_DATA * output = NULL;
	
	if(nbElemCopied != NULL)
		*nbElemCopied = 0;
	
	if(cache == NULL && !setupBDDCache())	//Échec du chargement
		return NULL;
	
	output = malloc((nbElemInCache + 1) * sizeof(PROJECT_DATA));	//Unused memory seems to stay on the pool, so we can ask for more than really needed in the case we only want installed stuffs
	if(output != NULL)
	{
		//On craft la requète en fonctions des arguments
		char sortRequest[50], requestString[200];
		if((maskRequest & RDB_SORTMASK) == SORT_NAME)
			strncpy(sortRequest, DBNAMETOID(RDB_projectName)" COLLATE "SORT_FUNC, 50);
		else if((maskRequest & RDB_SORTMASK) == SORT_REPO)
			strncpy(sortRequest, DBNAMETOID(RDB_repo), 50);
		else
			strncpy(sortRequest, DBNAMETOID(RDB_ID), 50);
		
		if((maskRequest & RDB_LOADMASK) == RDB_LOADINSTALLED)
			snprintf(requestString, 200, "SELECT * FROM "MAIN_CACHE" WHERE "DBNAMETOID(RDB_isInstalled)" = 1 ORDER BY %s ASC", sortRequest);
		else if((maskRequest & RDB_LOADMASK) == RDB_LOAD_FAVORITE)
			snprintf(requestString, 200, "SELECT * FROM "MAIN_CACHE" WHERE "DBNAMETOID(RDB_favoris)" = 1 ORDER BY %s ASC", sortRequest);
		else
			snprintf(requestString, 200, "SELECT * FROM "MAIN_CACHE" ORDER BY %s ASC", sortRequest);
		
		
		sqlite3_stmt* request = NULL;
		createRequest(cache, requestString, &request);
		
		MUTEX_LOCK(cacheParseMutex);
		
		while(pos < nbElemInCache && sqlite3_step(request) == SQLITE_ROW)
		{
			if(!copyOutputDBToStruct(request, &output[pos], (maskRequest & RDB_COPY_MASK) != RDB_EXCLUDE_DYNAMIC))
				continue;

			if(output[pos].isInitialized)
				pos++;
		}

		MUTEX_UNLOCK(cacheParseMutex);

		output[pos] = getEmptyProject();
		destroyRequest(request);
		
		if(nbElemCopied != NULL)
			*nbElemCopied = pos;
	}
	
	return output;
}

PROJECT_DATA * _getProjectFromSearch (uint64_t IDRepo, uint projectID, bool installed, bool copyDynamic)
{
	PROJECT_DATA * output = calloc(1, sizeof(PROJECT_DATA));
	if(output == NULL)
		return NULL;

	sqlite3_stmt* request = NULL;

	if(installed)
	{
		createRequest(cache, "SELECT * FROM "MAIN_CACHE" WHERE "DBNAMETOID(RDB_repo)" = ?1 AND "DBNAMETOID(RDB_projectID)" = ?2 AND "DBNAMETOID(RDB_isInstalled)" = 1", &request);
	}
	else
		createRequest(cache, "SELECT * FROM "MAIN_CACHE" WHERE "DBNAMETOID(RDB_repo)" = ?1 AND "DBNAMETOID(RDB_projectID)" = ?2", &request);

	sqlite3_bind_int64(request, 1, (int64_t) IDRepo);
	sqlite3_bind_int(request, 2, (int32_t) projectID);

	if(sqlite3_step(request) == SQLITE_ROW)
	{
		if(!copyOutputDBToStruct(request, output, copyDynamic))
		{
			free(output);
			output = NULL;
		}

		if(sqlite3_step(request) == SQLITE_ROW)
		{
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

PROJECT_DATA * getProjectFromSearch (uint64_t IDRepo, uint projectID, bool installed)
{
	return _getProjectFromSearch(IDRepo, projectID, installed, true);
}

PROJECT_DATA getProjectByIDHelper(uint cacheID, bool copyDynamic)
{
	sqlite3_stmt* request = NULL;
	PROJECT_DATA output = getEmptyProject();

	if(cache != NULL && cacheID != UINT_MAX)
	{
		createRequest(cache, "SELECT * FROM "MAIN_CACHE" WHERE "DBNAMETOID(RDB_ID)" = ?1", &request);
		sqlite3_bind_int(request, 1, (int32_t) cacheID);

		MUTEX_LOCK(cacheParseMutex);

		if(sqlite3_step(request) == SQLITE_ROW)
			copyOutputDBToStruct(request, &output, copyDynamic);

		MUTEX_UNLOCK(cacheParseMutex);

		destroyRequest(request);
	}

	return output;
}

PROJECT_DATA getProjectByID(uint cacheID)
{
	return getProjectByIDHelper(cacheID, true);
}

uint * getFavoritesID(uint * nbFavorites)
{
	if(nbFavorites == NULL || cache == NULL)
		return NULL;

	uint * output = malloc(nbElemInCache * sizeof(uint));	//nbMax of entries
	if(output == NULL)
		return NULL;

	sqlite3_stmt * request;
	if(createRequest(cache, "SELECT "DBNAMETOID(RDB_ID)" FROM "MAIN_CACHE" WHERE "DBNAMETOID(RDB_favoris)" = 1 ORDER BY "DBNAMETOID(RDB_ID)" ASC", &request) != SQLITE_OK)
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

void * getUpdatedCTForID(uint cacheID, bool wantTome, size_t * nbElemUpdated, uint ** price)
{
	sqlite3_stmt* request = NULL;

	if(wantTome)
		createRequest(cache, "SELECT "DBNAMETOID(RDB_nombreTomes)", "DBNAMETOID(RDB_tomes)" FROM "MAIN_CACHE" WHERE "DBNAMETOID(RDB_ID)" = ?1", &request);
	else
		createRequest(cache, "SELECT "DBNAMETOID(RDB_nombreChapitre)", "DBNAMETOID(RDB_chapitres)", "DBNAMETOID(RDB_chapitresPrice)" FROM "MAIN_CACHE" WHERE "DBNAMETOID(RDB_ID)" = ?1", &request);

	sqlite3_bind_int(request, 1, (int32_t) cacheID);

	if(sqlite3_step(request) != SQLITE_ROW)
		return NULL;

	uint nbElemOut = (uint32_t) sqlite3_column_int(request, 0);
	void * output = NULL;

	if(nbElemOut != 0)
	{
		if(wantTome)
		{
			output = calloc(nbElemOut + 1, sizeof(META_TOME));

			if(output != NULL)
			{
				((META_TOME*)output)[nbElemOut].ID = INVALID_SIGNED_VALUE;		//Whatever copyTomeList may do, the array is valid by now
				copyTomeList((META_TOME*) sqlite3_column_int64(request, 1), nbElemOut, output);
			}
		}
		else
		{
			output = malloc((nbElemOut + 1) * sizeof(int));
			if(output != NULL)
			{
				memcpy(output, (int*) sqlite3_column_int64(request, 1), nbElemOut * sizeof(int));
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
