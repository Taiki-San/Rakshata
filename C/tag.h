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

extern MUTEX_VAR concurentColdUpdate;

#define TAG_DB "tags.db"
#define OLD_TAG_DB "old_"TAG_DB
#define WIP_TAG_DB "WIP_"TAG_DB

enum
{
	DEFAULT_TAG_VERSION = 1
};

//Remote update parser
bool loadRemoteTagState(char * remoteDump, TAG_VERBOSE ** tags, uint * nbTags, CATEGORY ** categories, uint * nbCategories);

//Main API
void checkIfRefreshTag();

//Internal high level API
bool resetTagsToLocal();

void tagUpdateCachedEntry(TAG_VERBOSE * newData, uint nbData);
void catUpdateCachedEntry(CATEGORY * newData, uint nbData);
void dumpTagCat(TAG_VERBOSE * tags, uint nbTags, CATEGORY * category, uint nbCat);