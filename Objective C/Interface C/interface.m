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

/*****************************************
**										**
**				  PREFS					**
**										**
*****************************************/

uint32_t getMainThread()
{
	uint32_t output;
	
	[Prefs getPref:PREFS_GET_MAIN_THREAD :&output];
	
	return output;
}

bool shouldDownloadFavorite()
{
	bool output = true;
	
	[Prefs getPref:PREFS_GET_FAVORITE_AUTODL :&output];
	
	return output;
}

void sendToLog(char * string)
{
	NSLog(@"%s", string);
}

#ifdef DEV_VERSION
void logStack(void * address)
{
	[[NSString stringWithFormat:@"%@", [NSThread callStackSymbols]] writeToFile:[NSString stringWithFormat:@"log/%p.txt", address] atomically:NO encoding:NSASCIIStringEncoding error:nil];
}
#endif

void notifyEmailUpdate()
{
	[[NSNotificationCenter defaultCenter] postNotificationName:NOTIFICATION_MAIL object:nil];
}

void restorePrefsFile()
{
	NSString * settings = [[NSBundle mainBundle] pathForResource:@"settings" ofType:@""];
	
	if(settings != nil)
		[[NSData dataWithContentsOfFile:settings] writeToFile:@""SETTINGS_FILE"" options:NSDataWritingAtomic error:nil];
}

/*****************************************
 **										**
 **				 DB update				**
 **										**
 *****************************************/

void notifyFullUpdate()
{
	[RakDBUpdate postNotificationFullUpdate];
}

void notifyUpdateRepo(REPO_DATA repo)
{
	[RakDBUpdate postNotificationRepoUpdate:getRepoID(&repo)];
}

void notifyUpdateRootRepo(ROOT_REPO_DATA root)
{
	[RakDBUpdate postNotificationRepoUpdate:root.repoID];
}

void notifyUpdateProject(PROJECT_DATA project)
{
	[RakDBUpdate postNotificationProjectUpdate:project];
}

void notifyFullUpdateRepo()
{
	[RakDBUpdate postNotificationFullRepoUpdate];
}

/*****************************************
 **										**
 **			  Thumbnail update			**
 **										**
 *****************************************/

NSString * thumbNotificationName(byte updateType)
{
	switch (updateType)
	{
		case THUMBID_SRGRID:
			return NOTIFICATION_THUMBNAIL_UPDATE_SRGRID;
			
		case THUMBID_HEAD:
			return NOTIFICATION_THUMBNAIL_UPDATE_HEAD;
			
		case THUMBID_CT:
			return NOTIFICATION_THUMBNAIL_UPDATE_CT;
	}
	
	return nil;
}

void notifyThumbnailUpdate(ICONS_UPDATE * payload)
{
	if(payload == NULL)
		return;
	
	if(payload->next != NULL)
	{
		ICONS_UPDATE * next = payload->next;
		
		//The next entry is exactly the @2x version
		if(next->updateType == payload->updateType && next->projectID == payload->projectID && next->repoID == payload->repoID && strcmp(next->crc32, payload->crc32))
			return;
	}
	
	NSString * notificationName = thumbNotificationName(payload->updateType);
	
	if(notificationName != nil)
	{
		uint projectID = payload->projectID;
		uint64_t repoID = payload->repoID;
		
		dispatch_async(dispatch_get_main_queue(), ^{
			
			invalidateCacheForRepoID(repoID);
			[[NSNotificationCenter defaultCenter] postNotificationName:notificationName object:nil userInfo:@{@"project" : @(projectID), @"source" : @(repoID)}];
		});
	}
}

void registerThumbnailUpdate(id object, SEL selector, byte updateType)
{
	NSString * notificationName = thumbNotificationName(updateType);
	
	if(notificationName != nil)
		[[NSNotificationCenter defaultCenter] addObserver:object selector:selector name:notificationName object:nil];
}

/*****************************************
 **										**
 **			Restrictions update			**
 **										**
 *****************************************/

void notifyRestrictionChanged()
{
	[[NSNotificationCenter defaultCenter] postNotificationName: NOTIFICATION_SEARCH_UPDATED object:nil userInfo: nil];
}

/*****************************************
 **										**
 **				  Series				**
 **										**
 *****************************************/

void updateRecentSeries()
{
	[[NSNotificationCenter defaultCenter] postNotificationName: @"RakSeriesNeedUpdateRecent" object:nil userInfo: nil];
}

/*****************************************
 **										**
 **				    MDL					**
 **										**
 *****************************************/

bool checkIfElementAlreadyInMDL(PROJECT_DATA data, bool isTome, int element)
{
	MDL * tabMDL = [(RakAppDelegate*) [NSApp delegate] MDL];

	return tabMDL == nil ? [tabMDL proxyCheckForCollision:data :isTome :element] : false;
}

void addElementToMDL(PROJECT_DATA data, bool isTome, int element, bool partOfBatch)
{
	MDL * tabMDL = [(RakAppDelegate*) [NSApp delegate] MDL];
	
	if(tabMDL != nil)
		[tabMDL proxyAddElement:data isTome:isTome element:element partOfBatch:partOfBatch];
}