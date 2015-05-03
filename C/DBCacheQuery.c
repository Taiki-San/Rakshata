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

bool copyOutputDBToStruct(sqlite3_stmt *state, PROJECT_DATA* output)
{
	void* buffer;

	MUTEX_LOCK(cacheParseMutex);

	//Repo
	buffer = getRepoForID(sqlite3_column_int64(state, RDB_repo-1));
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
	output->cacheDBID = sqlite3_column_int(state, RDB_ID-1);
	
	//Project ID
	output->projectID = sqlite3_column_int(state, RDB_projectID-1);
	
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
		size_t length = strlen(buffer);
		wchar_t converted[length + 1];
		
		length = utf8_to_wchar(buffer, length, converted, length + 1, 0);	converted[length] = 0;
		wstrncpy(output->projectName, LENGTH_PROJECT_NAME, converted);
	}
	
	//Description
	buffer = (void*) sqlite3_column_text(state, RDB_description-1);
	if(buffer == NULL)
		memset(output->description, 0, sizeof(output->description));
	else
	{
		size_t length = strlen(buffer);
		wchar_t converted[length + 1];
		
		length = utf8_to_wchar(buffer, length, converted, length + 1, 0);	converted[length] = 0;
		wstrncpy(output->description, LENGTH_DESCRIPTION, converted);
	}
	
	//Nom de l'auteur
	buffer = (void*) sqlite3_column_text(state, RDB_authors-1);
	if(buffer == NULL)
		memset(output->authorName, 0, sizeof(output->authorName));
	else
	{
		size_t length = strlen(buffer);
		wchar_t converted[length + 1];
		
		length = utf8_to_wchar(buffer, length, converted, length + 1, 0);	converted[length] = 0;
		wstrncpy(output->authorName, LENGTH_AUTHORS, converted);
	}
	
	//Divers données
	output->status = sqlite3_column_int(state, RDB_status-1);	//On pourrait vérifier que c'est une valeur tolérable mais je ne vois pas de raison pour laquelle quelqu'un irait patcher la BDD
	output->type = sqlite3_column_int(state, RDB_type-1);
	output->japaneseOrder = sqlite3_column_int(state, RDB_asianOrder-1);
	output->isPaid = sqlite3_column_int(state, RDB_isPaid-1);
	output->tag = sqlite3_column_int(state, RDB_tag-1);
	output->nombreChapitre = sqlite3_column_int(state, RDB_nombreChapitre-1);
	
	buffer = (void*) sqlite3_column_int64(state, RDB_chapitresPrice - 1);
	if(buffer != NULL)
	{
		output->chapitresPrix = malloc((output->nombreChapitre+2) * sizeof(uint));
		if(output->chapitresPrix != NULL)
			memcpy(output->chapitresPrix, buffer, (output->nombreChapitre + 1) * sizeof(int));
		else
			output->chapitresPrix = NULL;
	}
	else
		output->chapitresPrix = NULL;
	
	buffer = (void*) sqlite3_column_int64(state, RDB_chapitres-1);
	if(buffer != NULL)
	{
		output->chapitresFull = malloc((output->nombreChapitre+2) * sizeof(int));
		if(output->chapitresFull != NULL)
		{
			memcpy(output->chapitresFull, buffer, (output->nombreChapitre + 1) * sizeof(int));
			output->chapitresInstalled = NULL;
			checkChapitreValable(output, NULL);
		}
		else
			output->chapitresInstalled = NULL;
	}
	else
	{
		output->chapitresFull = NULL;
		output->chapitresInstalled = NULL;
		output->nombreChapitreInstalled = 0;
		
		free(output->chapitresPrix);
		output->chapitresPrix = NULL;
	}
	
	output->nombreTomes = sqlite3_column_int(state, RDB_nombreTomes-1);
	output->haveDRM = sqlite3_column_int(state, RDB_DRM-1);
	
	buffer = (void*) sqlite3_column_int64(state, RDB_tomes-1);
	if(buffer != NULL)
	{
		output->tomesFull = malloc((output->nombreTomes + 2) * sizeof(META_TOME));
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
	
	output->favoris = sqlite3_column_int(state, RDB_favoris-1);
	output->isInitialized = true;
	
	MUTEX_UNLOCK(cacheParseMutex);
	
	return true;
}

PROJECT_DATA * getCopyCache(uint maskRequest, uint* nbElemCopied)
{
	uint pos = 0;
	PROJECT_DATA * output = NULL;
	
	if (nbElemCopied != NULL)
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
		else if((maskRequest & RDB_SORTMASK) == SORT_ID)
			strncpy(sortRequest, DBNAMETOID(RDB_repo), 50);
		else
			strncpy(sortRequest, DBNAMETOID(RDB_ID), 50);
		
		if((maskRequest & RDB_LOADMASK) == RDB_LOADINSTALLED)
			snprintf(requestString, 200, "SELECT * FROM rakSQLite WHERE "DBNAMETOID(RDB_isInstalled)" = 1 ORDER BY %s ASC", sortRequest);
		else
			snprintf(requestString, 200, "SELECT * FROM rakSQLite ORDER BY %s ASC", sortRequest);
		
		
		sqlite3_stmt* request = NULL;
		createRequest(cache, requestString, &request);
		
		while(pos < nbElemInCache && sqlite3_step(request) == SQLITE_ROW)
		{
			if(!copyOutputDBToStruct(request, &output[pos]))
				continue;
			
			if(output[pos].isInitialized)
				pos++;
		}
		
		output[pos] = getEmptyProject();
		destroyRequest(request);
		
		if(nbElemCopied != NULL)
			*nbElemCopied = pos;
	}
	
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

uint64_t getRepoIndexFromURL(char * URL)
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
						ROOT_REPO_DATA * currentElem = output[i], * currentOld = (ROOT_REPO_DATA *) originalData[i];
						
						if(currentElem->nombreSubrepo == 0)
						{
#ifdef FLUSH_UNUSED_REPO
							free(output[index]);
							output[index] = NULL;
							discardedElement++;
							continue;
#else
							currentElem->subRepo = NULL;
#endif
						}
						else
						{
							currentElem->subRepo = calloc(currentElem->nombreSubrepo, sizeof(REPO_DATA));
							if(currentElem->subRepo == NULL)
							{
								free(currentElem);
								currentElem = NULL;
								output[index] = NULL;
							}
						}
						
						if(currentElem != NULL)
						{
							//We need to alloc those ourselves
							currentElem->descriptions = NULL;
							currentElem->langueDescriptions = NULL;
							
							if(currentElem->subRepo != NULL)
							{
								memcpy(currentElem->subRepo, currentOld->subRepo, currentElem->nombreSubrepo * sizeof(REPO_DATA));
							}
							
							//Yep, descriptions are a pain in the ass
							if(currentElem->nombreDescriptions > 0)
							{
								currentElem->descriptions = calloc(currentElem->nombreDescriptions, sizeof(charType*));
								currentElem->langueDescriptions = calloc(currentElem->nombreDescriptions, sizeof(char*));
								
								if(currentElem->descriptions != NULL && currentElem->langueDescriptions != NULL)
								{
									for(uint posDesc = 0; posDesc < currentElem->nombreDescriptions; posDesc++)
									{
										uint lengthDesc = wstrlen(currentOld->descriptions[posDesc]), lengthLanguage = strlen(currentOld->langueDescriptions[posDesc]);
										currentElem->descriptions[posDesc] = malloc((lengthDesc + 1) * sizeof(charType));
										currentElem->langueDescriptions[posDesc] = malloc((lengthLanguage + 1) * sizeof(char));
										
										if(currentElem->descriptions[posDesc] == NULL || currentElem->langueDescriptions[posDesc] == NULL)
										{
											do
											{
												free(currentElem->descriptions[posDesc]);
												free(currentElem->langueDescriptions[posDesc]);
											} while(posDesc-- > 0);
											
											free(currentElem->descriptions);
											free(currentElem->langueDescriptions);
											free(currentElem->subRepo);
											free(currentElem);
											output[index] = NULL;
											break;
										}
										else
										{
											memcpy(currentElem->descriptions[posDesc], currentOld->descriptions[posDesc], lengthDesc * sizeof(charType));
											memcpy(currentElem->langueDescriptions[posDesc], currentOld->langueDescriptions[posDesc], lengthLanguage * sizeof(char));
											currentElem->descriptions[posDesc][lengthDesc] = 0;
											currentElem->langueDescriptions[posDesc][lengthLanguage] = 0;
										}
									}
								}
								else
								{
									free(currentElem->descriptions);
									free(currentElem->langueDescriptions);
									free(currentElem->subRepo);
									free(currentElem);
									output[index] = NULL;
									i--;
								}
							}
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

void freeRootRepo(ROOT_REPO_DATA ** root)
{
	if(root == NULL)
		return;
	
	for(uint i = 0; root[i] != NULL; i++)
	{
		freeSingleRootRepo(root[i]);
	}
	
	free(root);
}

void freeSingleRootRepo(ROOT_REPO_DATA * root)
{
	free(root->subRepo);
	
	if(root->descriptions != NULL)
	{
		for(uint j = 0, length = root->nombreDescriptions; j < length; j++)
			free(root->descriptions[j]);
	}
	
	if(root->langueDescriptions != NULL)
	{
		for(uint j = 0, length = root->nombreDescriptions; j < length; j++)
			free(root->langueDescriptions[j]);
	}
	
	free(root->descriptions);
	free(root->langueDescriptions);
	free(root);
}

void freeRepo(REPO_DATA ** repos)
{
	if(repos == NULL)
		return;
	
	for(uint i = 0; repos[i] != NULL; free(repos[i++]));
	free(repos);
}

