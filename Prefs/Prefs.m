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
uint mainThread = GUI_THREAD_SERIES;
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

+ (void) getPref : (int) requestID : (void*) outputContainer
{
	if(prefsCache == NULL)
		[self initCache];
	
	switch(requestID)
	{
		case PREFS_GET_MAIN_THREAD:
		{
			int* output = outputContainer;
			*output = mainThread;
			break;
		}
			
		case PREFS_GET_TAB_SERIE_WIDTH:
		{
			int * output = outputContainer;
			*output = getWidthSerie(mainThread, stateTabsReader, false);
			break;
		}
			
		case PREFS_GET_TAB_CT_WIDTH:
		{
			int * output = outputContainer;
			*output = getWidthCT(mainThread, stateTabsReader, false);
			break;
		}
			
		case PREFS_GET_TAB_READER_WIDTH:
		{
			int * output = outputContainer;
			*output = getWidthReader(mainThread, stateTabsReader);
			break;
		}
			
		case PREFS_GET_TAB_SERIE_POSX:
		{
			int * output = outputContainer;
			*output = 0;	//Le tab série est collé au bord gauche
			break;
		}
			
		case PREFS_GET_TAB_CT_POSX:
		{
			int * output = outputContainer;
			*output = getWidthSerie(mainThread, stateTabsReader, true);
			break;
		}
			
		case PREFS_GET_TAB_READER_POSX:
		{
			int * output = outputContainer;
			*output = (getWidthSerie(mainThread, stateTabsReader, true) + getWidthCT(mainThread, stateTabsReader, true));
			break;
		}
			
		case PREFS_GET_SERIE_FOOTER_HEIGHT:
		{
			int *output = outputContainer;
			*output = TAB_SERIE_FOOTER_HEIGHT;
			break;
		}
			
		case PREFS_GET_CT_FOOTER_HEIGHT:
		{
			int *output = outputContainer;
			*output = TAB_CT_FOOTER_HEIGHT;
			break;
		}
			
		case PREFS_GET_READER_FOOTER_HEIGHT:
		{
			int *output = outputContainer;
			*output = TAB_READER_FOOTER_HEIGHT;
			break;
		}
			
		case PREFS_GET_MDL_WIDTH:
		{
			getMDLWidth(outputContainer, mainThread, stateTabsReader);
			break;
		}
			
		case PREFS_GET_MDL_HEIGHT:
		{
			getMDLHeight(outputContainer, mainThread, stateTabsReader);
			break;
		}
			
		case PREFS_GET_MDL_POS_Y:
		{
			getMDLPosY(outputContainer, mainThread, stateTabsReader);
			break;
		}
			
		case PREFS_GET_MDL_POS_X:
		{
			getMDLPosX(outputContainer, mainThread, stateTabsReader);
			break;
		}
			
		case PREFS_GET_IS_READER_MT:
		{
			bool * data = outputContainer;
			*data = (mainThread & GUI_THREAD_READER) != 0;
			break;
		}
			
		case PREFS_GET_READER_TABS_STATE:
		{
			int * output = outputContainer;
			*output = stateTabsReader;
			break;
		}
			
		default:
		{
			NSLog(@"Couldn't identify request: %d", requestID);
		}
	}
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
			break;
		}
			
		case PREFS_SET_READER_TABS_STATE_FROM_CALLER:
		{
			int newValue = 0;
			switch(value)
			{
				case GUI_THREAD_SERIES:
				{
					newValue = STATE_READER_TAB_SERIE_FOCUS;
					break;
				}
				case GUI_THREAD_CT:
				{
					newValue = STATE_READER_TAB_CT_FOCUS;
					break;
				}
				case GUI_THREAD_MDL:
				{
					newValue = STATE_READER_TAB_MDL_FOCUS;
					break;
				}
			}
			if(!newValue)
				ret_value = false;
			else
			{
				ret_value = stateTabsReader == (uint) newValue;
				stateTabsReader = newValue;
			}
			
			break;
		}

		default:
			break;
	}
	return ret_value;
}
@end
