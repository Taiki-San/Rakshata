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
**				  Series				**
**										**
*****************************************/

void updateRecentSeries()
{
	[[NSNotificationCenter defaultCenter] postNotificationName: @"RakSeriesNeedUpdateContent" object:nil userInfo: @{@"request": [NSNumber numberWithInt:RELOAD_RECENT]}];
}

/*****************************************
 **										**
 **				    MDL					**
 **										**
 *****************************************/

bool checkIfElementAlreadyInMDL(PROJECT_DATA data, bool isTome, int element)
{
	if(sharedTabMDL == nil)
		return false;
	return [(id) sharedTabMDL proxyCheckForCollision:data :isTome :element];
}

void addElementToMDL(PROJECT_DATA data, bool isTome, int element, bool partOfBatch)
{
	if(sharedTabMDL != nil)
		[(id) sharedTabMDL proxyAddElement:data :isTome :element :partOfBatch];
}