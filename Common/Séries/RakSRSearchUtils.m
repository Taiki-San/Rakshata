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
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

byte getRestrictionTypeForSBID(byte searchBarID)
{
	if(searchBarID == SEARCH_BAR_ID_AUTHOR)
		return RDBS_TYPE_AUTHOR;
	
	else if(searchBarID == SEARCH_BAR_ID_SOURCE)
		return RDBS_TYPE_SOURCE;
	
	else if(searchBarID == SEARCH_BAR_ID_TAG)
		return RDBS_TYPE_TAG;
	
	else if(searchBarID == SEARCH_BAR_ID_CAT)
		return RDBS_TYPE_CAT;
	
	return 0;
}

NSString * getNotificationNameForSBID(byte searchBarID)
{
	if(searchBarID == SEARCH_BAR_ID_AUTHOR)
		return SR_NOTIFICATION_AUTHOR;
	
	else if(searchBarID == SEARCH_BAR_ID_SOURCE)
		return SR_NOTIFICATION_SOURCE;
	
	else if(searchBarID == SEARCH_BAR_ID_TAG)
		return SR_NOTIFICATION_TAG;
	
	else if(searchBarID == SEARCH_BAR_ID_CAT)
		return SR_NOTIFICATION_TYPE;
	
	return nil;
}