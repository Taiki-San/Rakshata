/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

#include "dbCache.h"

SUGGESTIONS_FAVS * getIDOfInterestingFavorites(uint forbiddenID, uint nbMax, uint * nbOutputData)
{
	sqlite3 * recentDB;	
	if(cache == NULL || (recentDB = getPtrRecentDB()) == NULL)
	{
		*nbOutputData = 0;
		return NULL;
	}

	sqlite3_stmt * request = createRequest(cache, "SELECT "DBNAMETOID(RDB_ID)", "DBNAMETOID(RDB_projectID)", "DBNAMETOID(RDB_repo)", "DBNAMETOID(RDB_nbChapter)", "DBNAMETOID(RDB_chapitres)", "DBNAMETOID(RDB_nbVolumes)", "DBNAMETOID(RDB_tomes)" FROM "MAIN_CACHE" WHERE "DBNAMETOID(RDB_favoris)" = 1 ORDER BY RANDOM()");
	if(request == NULL)
	{
		closeRecentDB(recentDB);
		*nbOutputData = 0;
		return NULL;
	}
	
	uint nbOutput = 0;
	SUGGESTIONS_FAVS * output = NULL;

	REPO_DATA repo;
	PROJECT_DATA project = getEmptyProject();
	project.repo = &repo;
	
	//We are looking for favorites with new, unread content.
	//Ideally, the insertion point has to be as close from where the reading was suspended as possible.
	while(nbOutput < nbMax && sqlite3_step(request) == SQLITE_ROW)
	{
		//We pull everything from the database
		project.cacheDBID = (uint) sqlite3_column_int(request, 0);
		project.projectID = (uint) sqlite3_column_int(request, 1);
		repo = getEmptyRepoWithID((uint64_t) sqlite3_column_int64(request, 2));
		
		project.nbChapter = (uint) sqlite3_column_int(request, 3);
		project.chaptersFull = (uint *) sqlite3_column_int64(request, 4);

		project.nbVolumes = (uint) sqlite3_column_int(request, 5);
		project.volumesFull = (META_TOME *) sqlite3_column_int64(request, 6);

		uint posLastChapterID, posLastVolumeID;
		bool lastReadWasTome = lastReadAsTome(recentDB, project), realLastReadWasTome = lastReadWasTome, haveChapter = true, haveVolume = true;
		
		//We load the previously read ID, they lookup it position in our array
		if(project.nbVolumes != 0 && project.volumesFull != NULL)
		{
			uint lastVolumeID = getSavedIDForProject(recentDB, project, true);
			if(lastVolumeID != INVALID_VALUE)
			{
				posLastVolumeID = project.nbVolumes;
				while(posLastVolumeID != 0 && project.volumesFull[--posLastVolumeID].ID != lastVolumeID);
				
				if(project.volumesFull[posLastVolumeID].ID != lastVolumeID)
					posLastVolumeID = INVALID_VALUE;
			}
			else
				posLastVolumeID = INVALID_VALUE;
		}
		else
		{
			if(lastReadWasTome)
				lastReadWasTome = false;
			
			posLastVolumeID = INVALID_VALUE;
			haveVolume = false;
		}
		
		if(project.nbChapter != 0 && project.chaptersFull != NULL)
		{
			uint lastChapterID = getSavedIDForProject(recentDB, project, false);
			if(lastChapterID != INVALID_VALUE)
			{
				posLastChapterID = project.nbChapter;
				while(posLastChapterID != 0 && project.chaptersFull[--posLastChapterID] != lastChapterID);
				
				if(project.chaptersFull[posLastChapterID] != lastChapterID)
					posLastChapterID = INVALID_VALUE;
			}
			else
				posLastChapterID = INVALID_VALUE;
		}
		else
		{
			if(!lastReadWasTome)
				lastReadWasTome = true;
			
			posLastChapterID = INVALID_VALUE;
			haveChapter = false;
		}
		
		//Nothing available :(
		if(!haveChapter && !haveVolume)
			continue;
		
		//We check if the last CT (depending of what was read last) is already read
		//We reverse lastReadWasTome first as the loop reverse it again (to check both C and T)
		uint indexLastCT;
		lastReadWasTome = !lastReadWasTome;
		for(byte i = 0; i < 2; ++i)
		{
			lastReadWasTome = !lastReadWasTome;
			if(!ACCESS_DATA(lastReadWasTome, haveChapter, haveVolume))
				continue;
			
			indexLastCT = ACCESS_DATA(lastReadWasTome, project.nbChapter, project.nbVolumes) - 1;
			if(checkSoonToBeReadable(project, lastReadWasTome, indexLastCT) && !checkAlreadyRead(project, lastReadWasTome, indexLastCT))
			{
				uint bound = ACCESS_DATA(lastReadWasTome, posLastChapterID, posLastVolumeID);
				if(!shouldSuggestFromLastRead())
				{
					do
					{
						indexLastCT -= 1;
					} while(indexLastCT > bound && checkSoonToBeReadable(project, lastReadWasTome, indexLastCT) && !checkAlreadyRead(project, lastReadWasTome, indexLastCT));
				}
				else
					indexLastCT = bound;
				
				break;
			}
			else
				indexLastCT = INVALID_VALUE;
		}

		//We add our new entry to the array
		if(indexLastCT != INVALID_VALUE)
		{
			output = realloc(output, ++nbOutput * sizeof(SUGGESTIONS_FAVS));
			if(output != NULL)
			{
				//If we resume the reading from the previously read point, we return a special value
				
				output[nbOutput - 1].ID = project.cacheDBID;
				output[nbOutput - 1].indexInsertionID = indexLastCT == ACCESS_DATA(lastReadWasTome, posLastChapterID, posLastVolumeID) ? INVALID_VALUE : indexLastCT;
				output[nbOutput - 1].isTome = lastReadWasTome;
				output[nbOutput - 1].priority = lastReadWasTome == realLastReadWasTome ? SUGG_PRIORITY_FAVS_BEST : SUGG_PRIORITY_FAVS_LOWER;
			}
			else
				nbOutput -= 1;
		}
		
		//FIXME: Should cross check volumes/chapter, so if the last volume is read, but new chapters, following the volume are out, detect them
	}
	
	closeRecentDB(recentDB);
	
	*nbOutputData = nbOutput;
	return output;
}