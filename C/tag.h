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

#define TABLE_TAGS "rakTags"
#define TABLE_CATEGORY "rakCats"
#define TABLE_TAG_VERSION "HiRDJLcYua0"
#define VERSION_COLUMN_NAME "DeusExHRRocks"

enum
{
	DEFAULT_TAG_VERSION = 1
};

//Remote update parser
bool loadRemoteTagState(char * remoteDump, TAG_VERBOSE ** tags, uint * nbTags, CATEGORY_VERBOSE ** categories, uint * nbCategories, uint * newDBVersion);

//Internal high level API
void updateTagDBVersion(uint newDBVersion);
uint getTagDBVersion();

bool resetTagsToLocal();

void tagUpdateCachedEntry(TAG_VERBOSE * newData, uint nbData);
void catUpdateCachedEntry(CATEGORY_VERBOSE * newData, uint nbData);
void dumpTagCat(TAG_VERBOSE * tags, uint nbTags, CATEGORY_VERBOSE * category, uint nbCat);