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

#import "superHeader.h"
#include "prefsTools.h"

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
			return (void*) getWidthSerie(mainThread);
			break;
		}
			
		case PREFS_GET_TAB_CT_WIDTH:
		{
			return (void*) getWidthCT(mainThread);
			break;
		}
			
		case PREFS_GET_TAB_READER_WIDTH:
		{
			return (void*) getWidthReader(mainThread);
			break;
		}
			
		case PREFS_GET_TAB_SERIE_POSX:
		{
			return (void*) 0;	//Le tab série est collé au bord gauche
			break;
		}
			
		case PREFS_GET_TAB_CT_POSX:
		{
			return (void*) getWidthSerie(mainThread);
			break;
		}
			
		case PREFS_GET_TAB_READER_POSX:
		{
			return (void*) (getWidthSerie(mainThread) + getWidthCT(mainThread));
			break;
		}
			
		default:
		{
			NSLog(@"Couldn't identify request: %d", request);
		}
	}
	return NULL;
}

@end
