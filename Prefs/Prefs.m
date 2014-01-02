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
uint mainThread = GUI_THREAD_READER;
uint stateTabsReader = STATE_READER_TAB_DEFAULT;

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

+ (void *) getPref : (int) requestID
{
	if(prefsCache == NULL)
		[self initCache];
	
	switch(requestID)
	{
		case PREFS_GET_TAB_SERIE_WIDTH:
		{
			return (void*) getWidthSerie(mainThread, stateTabsReader);
			break;
		}
			
		case PREFS_GET_TAB_CT_WIDTH:
		{
			return (void*) getWidthCT(mainThread, stateTabsReader);
			break;
		}
			
		case PREFS_GET_TAB_READER_WIDTH:
		{
			return (void*) getWidthReader(mainThread, stateTabsReader);
			break;
		}
			
		case PREFS_GET_TAB_SERIE_POSX:
		{
			return (void*) 0;	//Le tab série est collé au bord gauche
			break;
		}
			
		case PREFS_GET_TAB_CT_POSX:
		{
			return (void*) getWidthSerie(mainThread, stateTabsReader);
			break;
		}
			
		case PREFS_GET_TAB_READER_POSX:
		{
			return (void*) (getWidthSerie(mainThread, stateTabsReader) + getWidthCT(mainThread, stateTabsReader));
			break;
		}
			
		default:
		{
			NSLog(@"Couldn't identify request: %d", requestID);
		}
	}
	return NULL;
}

+ (bool) setPref : (uint) requestID : (uint64) value
{
	bool ret_value = false;
	
	if(prefsCache == NULL)
		[self initCache];
	
	switch (requestID)
	{
		case PREFS_SET_OWNMAINTAB:
		{
			ret_value = mainThread != (uint) value;
			mainThread = value & GUI_MASK;
			break;
		}
			
		case PREFS_SET_READER_TABS_STATE:
		{
			ret_value = stateTabsReader == (uint) value;
			stateTabsReader = value & STATE_READER_TAB_MASK;
		}

		default:
			break;
	}
	return ret_value;
}
@end
