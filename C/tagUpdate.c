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

#include "tag.h"

MUTEX_VAR concurentColdUpdate;

void checkIfRefreshTag()
{
	char * bufferDL = NULL;
	size_t downloadLength;
	
	char URL[250];
	
	snprintf(URL, sizeof(URL), "https://"SERVEUR_URL"/getUpdatedTags.php?v=%d", getTagDBVersion());
	
	//'0' => no update available
	if(download_mem(URL, NULL, &bufferDL, &downloadLength, SSL_ON) != CODE_RETOUR_OK || bufferDL == NULL || *bufferDL == '0')
	{
		free(bufferDL);
		return;
	}
	
	//We parse the updated DB
	TAG_VERBOSE * tagDB = NULL;
	CATEGORY_VERBOSE * catDB = NULL;
	uint tagLength, catLength, newDBVersion;
	
	//The parser rejected the file
	if(!loadRemoteTagState(bufferDL, &tagDB, &tagLength, &catDB, &catLength, &newDBVersion))
	{
		free(bufferDL);
		return;
	}
	
	//Okay, we got the data, we have to insert into our dynamic DB and update the static one
	tagUpdateCachedEntry(tagDB, tagLength);
	catUpdateCachedEntry(catDB, catLength);
	
	dumpTagCat(tagDB, tagLength, catDB, catLength);
	updateTagDBVersion(newDBVersion);
}
