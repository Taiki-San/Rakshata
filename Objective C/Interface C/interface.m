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

void firstLaunch()
{
	
}

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

void sendToLog(char * string)
{
	NSLog(@"%s", string);
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

void notifyUpdateTeam(TEAMS_DATA team)
{
	[RakDBUpdate postNotificationTeamUpdate:team];
}

void notifyUpdateProject(PROJECT_DATA project)
{
	[RakDBUpdate postNotificationProjectUpdate:project];
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