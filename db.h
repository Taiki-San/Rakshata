/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite  **
 **                                                                                         **
 *********************************************************************************************/

#include "sqlite3.h"

#define INITIAL_BUFFER_SIZE 1024
#define KEEP_UNUSED_TEAMS				//If droped, they won't be refreshed, nor their manga DB will be updated, so bad idea for now
//#define DELETE_UNLISTED_PROJECT

#define DB_CACHE_EXPIRENCY 5*60*1000	//5 minutes

unsigned long alreadyRefreshed;

/******		DBTools.c	  ******/
bool isRemoteRepoLineValid(char * data, int version);
bool parseRemoteRepoLine(char *data, TEAMS_DATA *previousData, int version, TEAMS_DATA *output);

uint defineBoundsTeamOnProjectDB(MANGAS_DATA * oldData, uint posBase, uint nbElem);
bool downloadedProjectListSeemsLegit(char *data, MANGAS_DATA reference);
uint getNumberLineReturn(char *input);
bool extractCurrentLine(char * input, uint * posInput, char * output, uint lengthOutput);
bool parseCurrentProjectLine(char * input, int version, MANGAS_DATA * output);
void parseDetailsBlock(char * input, MANGAS_DATA *data, char *teamName, uint lengthOfBlock);
bool isProjectListSorted(MANGAS_DATA* data, uint length);
void applyChangesProject(MANGAS_DATA * oldData, uint magnitudeOldData, MANGAS_DATA * newData, uint magnitudeNewData);

void resetUpdateDBCache();