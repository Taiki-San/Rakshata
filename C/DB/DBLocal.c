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

//Prevent reusing ID
static uint baseID = 1;

uint getEmptyLocalSlot(PROJECT_DATA project)
{
	char requestString[300];

	//Either we want the main local repo, or the local entries of a standard repo
	snprintf(requestString, sizeof(requestString), "SELECT "DBNAMETOID(RDB_projectID)" FROM "MAIN_CACHE" WHERE "DBNAMETOID(RDB_repo)" = %llu %s ORDER BY "DBNAMETOID(RDB_projectID)" ASC", getRepoID(project.repo), isLocalProject(project) ? "AND "DBNAMETOID(RDB_isLocal)" = 1" : "");

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

	//We ensure the directory is clean
	project.projectID = baseID;
	char * path = getPathForProject(project);
	if(path != NULL)
	{
		char pathFinal[strlen(path) + 100];
		snprintf(pathFinal, sizeof(pathFinal), PROJECT_ROOT"%s/", path);
		free(path);

		removeFolder(pathFinal);
	}
	
	removeRecentEntryInternal(LOCAL_REPO_ID, baseID, true);

	return baseID++;
}

void registerImportEntry(PROJECT_DATA_PARSED project, bool isTome)
{
	if(ACCESS_DATA(isTome, (void *) project.chaptersLocal, (void *) project.tomeLocal) == NULL)
		return;

	//Okay, we have quite some work to do
	//First, we need to check if we need to register the project

	char requestString[400];
	uint64_t repoID = getRepoID(project.project.repo);

	//Does the oroject exist?
	snprintf(requestString, sizeof(requestString), "SELECT "DBNAMETOID(RDB_ID)" FROM "MAIN_CACHE" WHERE "DBNAMETOID(RDB_projectID)" = %d AND "DBNAMETOID(RDB_repo)" = %llu %s ORDER BY "DBNAMETOID(RDB_ID)" ASC", project.project.projectID, repoID, isLocalProject(project.project) ? "AND "DBNAMETOID(RDB_isLocal)" = 1" : "");

	sqlite3_stmt * request = createRequest(cache, requestString);

	PROJECT_DATA_PARSED cachedProject;
	switch(sqlite3_step(request))
	{
		//We found something, we need to insert our new data in the base
		//If we can't load the data, we just insert the new entry
		case SQLITE_ROW:
		{
			cachedProject = getParsedProjectByID((uint) sqlite3_column_int(request, 0));

			//Successfully loaded the data
			if(cachedProject.project.isInitialized)
				break;
		}

		//Insert into the base, and quit
		case SQLITE_OK:
		default:
		{
			destroyRequest(request);

			generateCTUsable(&project);

			project.project.cacheDBID = addToCache(NULL, project, repoID, true, true);
			if(project.project.cacheDBID != 0)
			{
				insertInSearch(NULL, INSERT_PROJECT, project.project);
				addRecentEntry(project.project, true);
			}
			return;
		}
	}

	//We insert the new item
	if(isTome)
	{
		META_TOME * newField = realloc(cachedProject.tomeLocal, (project.nbVolumesLocal + cachedProject.nbVolumesLocal) * sizeof(META_TOME));
		if(newField != NULL)
		{
			memcpy(&newField[cachedProject.nbVolumesLocal], project.tomeLocal, project.nbVolumesLocal * sizeof(META_TOME));
			cachedProject.nbVolumesLocal += project.nbVolumesLocal;
			cachedProject.tomeLocal = newField;

			qsort(cachedProject.tomeLocal, cachedProject.nbVolumesLocal, sizeof(META_TOME), sortTomes);
		}
	}
	else
	{
		uint * newField = realloc(cachedProject.chaptersLocal, (project.nbChapterLocal + cachedProject.nbChapterLocal) * sizeof(uint));
		if(newField != NULL)
		{
			memcpy(&newField[cachedProject.nbChapterLocal], project.chaptersLocal, project.nbChapterLocal * sizeof(uint));
			cachedProject.nbChapterLocal += project.nbChapterLocal;
			cachedProject.chaptersLocal = newField;

			qsort(cachedProject.chaptersLocal, cachedProject.nbChapterLocal, sizeof(uint), sortNumbers);
		}
	}

	consolidateCTLocale(&cachedProject, isTome);
	generateCTUsable(&cachedProject);

	updateCache(cachedProject, RDB_UPDATE_ID, 0);
	addRecentEntry(cachedProject.project, true);
	releaseParsedData(cachedProject);
}

void migrateRemovedInstalledToLocal(PROJECT_DATA_PARSED oldProject, PROJECT_DATA_PARSED * newProject)
{
	uint nbOld, nbNew, lengthCollector;
	void * dataOld, *dataNew, * collector;
	uint16_t sizeOfType;
	bool shouldFreeCollector, isTome;

	//We have two passes, one for the chapters, one for the volume
	for(byte run = 0; run < 2; run++)
	{
		lengthCollector = 0;
		collector = NULL;
		shouldFreeCollector = false;
		isTome = run == 1;

		if(!isTome)	//chapters
		{
			oldProject.project.nbChapter = nbOld = oldProject.nbChapterRemote;
			oldProject.project.chaptersFull = dataOld = oldProject.chaptersRemote;
			nbNew = newProject->nbChapterRemote;
			dataNew = newProject->chaptersRemote;

			sizeOfType = sizeof(uint);
		}
		else
		{
			oldProject.project.nbVolumes = nbOld = oldProject.nbVolumesRemote;
			oldProject.project.volumesFull = dataOld = oldProject.tomeRemote;
			nbNew = newProject->nbVolumesRemote;
			dataNew = newProject->tomeRemote;

			sizeOfType = sizeof(META_TOME);
		}

		//Nothing that could even be installed, make things easier...
		if(nbOld == 0)
			continue;

		//Nothing remaining, so anything remaining is here to stay
		if(nbNew == 0)
		{
			if(isTome)
				oldProject.project.volumesInstalled = NULL;
			else
				oldProject.project.chaptersInstalled = NULL;

			getCTInstalled(&oldProject.project, isTome);

			uint length = ACCESS_DATA(isTome, oldProject.project.nbChapterInstalled, oldProject.project.nbVolumesInstalled);

			if(length != 0)
			{
				collector = ACCESS_DATA(isTome, (void*) oldProject.project.chaptersInstalled, (void*) oldProject.project.volumesInstalled);
				lengthCollector = length;
			}
		}
		else
		{
			//Okay, we have to perform a real diff
			collector = malloc(nbOld * sizeOfType);
			if(collector == NULL)
			{
				memoryError(nbOld * sizeOfType);
				continue;
			}

			shouldFreeCollector = true;

			//O(n^2) because meh
			for(uint posOld = 0, posNew; posOld < nbOld; ++posOld)
			{
				posNew = 0;
				uint oldDataForIndex = ACCESS_CT(isTome, dataOld, dataOld, posOld);

				for(; posNew < nbNew && ACCESS_CT(isTome, dataNew, dataNew, posNew) != oldDataForIndex; ++posNew);

				//The entry still exist, awesome
				if (posNew != nbNew)
					continue;

				//Oh, is it still installed?
				if(checkReadable(oldProject.project, isTome, oldDataForIndex))
				{
					//And shit... we need to copy it to the selector :X
					if(isTome)
						copyTomeList(&(((META_TOME *) dataOld)[posOld]), 1, &(((META_TOME *) collector)[lengthCollector++]));
					else
						((uint *) collector)[lengthCollector++] = oldDataForIndex;
				}
			}
		}

		//Copy the collected data to the local buffer
		if(lengthCollector != 0)
		{
			if(isTome)
			{
				void * tmp = realloc(newProject->tomeLocal, newProject->nbVolumesLocal + lengthCollector);
				if(tmp != NULL)
				{
					newProject->tomeLocal = tmp;
					memcpy(&(newProject->tomeLocal[newProject->nbVolumesLocal]), collector, lengthCollector * sizeOfType);
					newProject->nbVolumesLocal += lengthCollector;
				}
			}
			else
			{
				void * tmp = realloc(newProject->chaptersLocal, newProject->nbChapterLocal + lengthCollector);
				if(tmp != NULL)
				{
					newProject->chaptersLocal = tmp;
					memcpy(&(newProject->chaptersLocal[newProject->nbChapterLocal]), collector, lengthCollector * sizeOfType);
					newProject->nbChapterLocal += lengthCollector;
				}
			}
		}

		if(shouldFreeCollector)
			free(collector);
	}
}

static uint cachedVolumeID = MIN_LOCAL_VOLUME_ID - 1;
uint getVolumeIDForImport(PROJECT_DATA project)
{
	//Used in a scenario where isInitialized is not set (cf RakImportIO) so we can't check it
	
	char *encodedRepo = getPathForProject(project);
	if(encodedRepo == NULL)
		return INVALID_VALUE;
	
	char dirVol[2*LENGTH_PROJECT_NAME + 100];
	bool loopedOnce = false;

	//Check for an empty slot
	do
	{
		if(cachedVolumeID < MIN_LOCAL_VOLUME_ID - 1)
		{
			cachedVolumeID = MIN_LOCAL_VOLUME_ID - 1;
			
			if(loopedOnce)
			{
				free(encodedRepo);
				return INVALID_VALUE;
			}
			else
				loopedOnce = true;
		}
		
		snprintf(dirVol, sizeof(dirVol), PROJECT_ROOT"%s/"VOLUME_PREFIX"%u/", encodedRepo, ++cachedVolumeID);

	} while(checkDirExist(dirVol));
	
	free(encodedRepo);
	
	return cachedVolumeID;
}

bool isLocalVolumeID(uint ID)
{
	return ID >= 0x80000000;
}
