/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at https://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

#include "tag.h"

MUTEX_VAR concurentColdUpdate;

void checkIfRefreshTag()
{
	char * bufferDL = NULL;
	size_t downloadLength;
	
	char URL[250];
	
	snprintf(URL, sizeof(URL), SERVEUR_URL"/getUpdatedTags.php?v=%d", getTagDBVersion());
	
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
	
	//The parser accepted the file
	if(loadRemoteTagState(bufferDL, &tagDB, &tagLength, &catDB, &catLength, &newDBVersion))
	{
		//Okay, we got the data, we have to insert into our dynamic DB and update the static one
		tagUpdateCachedEntry(tagDB, tagLength);
		catUpdateCachedEntry(catDB, catLength);
		
		dumpTagCat(tagDB, tagLength, catDB, catLength);
		updateTagDBVersion(newDBVersion);
		
		notifyFullUpdate();
	}
	
	free(bufferDL);
}
