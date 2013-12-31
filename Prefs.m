/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                          **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite   **
 **                                                                                          **
 *********************************************************************************************/

#import "Prefs.h"

void* prefsCache;
int mainThread = GUI_THREAD_SERIES;

@implementation Prefs

+ (void) initCache
{
	//We'll have to cache the old encrypted prefs /!\ prefs de crypto à protéger!!!
	//Also, need to get the open prefs including tabs size, theme and various stuffs
}

+ (void) rebuildCache
{
	
}

+ (void) clearCache
{
	
}

+ (void *) getPref : (int) request
{
	if(prefsCache == NULL)
		[self initCache];
	
	switch(request)
	{
		case PREFS_GET_TAB_SERIE_WIDTH:
		{
			if(mainThread & (GUI_THREAD_CT | GUI_THREAD_MDL))
				return (void*) TAB_SERIE_INACTIVE_CT;
			else if(mainThread & GUI_THREAD_READER)
				return (void*) TAB_SERIE_INACTIVE_LECTEUR;
			else
			{
				if((mainThread & GUI_THREAD_SERIES) == 0)
					NSLog(@"Couldn't identify mainThread in prefs");
				return (void *) 600;
			}
		}
			
		default:
		{
			NSLog(@"Couldn't identify request: %d", request);
		}
	}
	return NULL;
}

@end
