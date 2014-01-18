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

Prefs* prefsCache;

// Contexte
uint mainThread = GUI_THREAD_READER;				//Default : GUI_THREAD_SERIES
uint stateTabsReader = STATE_READER_TAB_DEFAULT;	//Default : STATE_READER_TAB_DEFAULT
uint backgroundTabsState = GUI_THREAD_SERIES;

@implementation Prefs

+ (void) initCache
{
	prefsCache = [Prefs alloc];
	if(prefsCache != NULL)
	{
		[prefsCache init];
	}
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
			*output = [prefsCache->tabSerieWidth getData: mainThread : backgroundTabsState: stateTabsReader];
			break;
		}
			
		case PREFS_GET_TAB_CT_WIDTH:
		{
			int * output = outputContainer;
			*output = [prefsCache->tabCTWidth getData: mainThread : backgroundTabsState: stateTabsReader];
			break;
		}
			
		case PREFS_GET_TAB_READER_WIDTH:
		{
			int * output = outputContainer;
			*output = [prefsCache->tabReaderWidth getData: mainThread : backgroundTabsState: stateTabsReader];
			break;
		}
			
		case PREFS_GET_TAB_SERIE_POSX:
		{
			int * output = outputContainer;
			*output = [prefsCache->tabSeriePosX getData: mainThread : backgroundTabsState: stateTabsReader];
			break;
		}
			
		case PREFS_GET_TAB_CT_POSX:
		{
			int * output = outputContainer;
			*output = [prefsCache->tabCTPosX getData: mainThread : backgroundTabsState: stateTabsReader];
			break;
		}
			
		case PREFS_GET_TAB_READER_POSX:
		{
			int * output = outputContainer;
			*output = [prefsCache->tabReaderPosX getData: mainThread : backgroundTabsState: stateTabsReader];
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
			ret_value = stateTabsReader != (uint) value;
			stateTabsReader = value & STATE_READER_TAB_MASK;
			break;
		}
			
		case PREFS_SET_READER_TABS_STATE_FROM_CALLER:
		{
			int newValue = -1;
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
				case GUI_THREAD_READER:
				{
					newValue = STATE_READER_TAB_ALL_COLLAPSED;
					break;
				}
			}
			if(newValue == -1)
			{
				ret_value = false;
				NSLog(@"Couldn't identify thread in PREFS_SET_READER_TABS_STATE_FROM_CALLER");
			}
			else
			{
				ret_value = stateTabsReader != (uint) newValue;
				stateTabsReader = newValue;
			}
			
			break;
		}

		default:
			break;
	}
	return ret_value;
}

/************		Private sections		************/

- (id) init
{
	self = [super init];
	if(self != nil)
	{
		char staticTest[] = "ffffffffffffffffffffffffff";
		
		//Width
		tabSerieWidth = [RakWidthSeries alloc];
		tabCTWidth = [RakWidthCT alloc];
		tabReaderWidth = [RakWidthReader alloc];
		
		if(tabSerieWidth == NULL || tabCTWidth == NULL || tabReaderWidth == NULL)
			[self flushMemory:YES];
		
		[tabSerieWidth init: prefsCache: staticTest];
		[tabCTWidth init: prefsCache: staticTest];
		[tabReaderWidth init: prefsCache: staticTest];
		
		//Pos X
		tabSeriePosX = [RakPosXSeries alloc];
		tabCTPosX = [RakPosXCT alloc];
		tabReaderPosX = [RakPosXReader alloc];
		
		if(tabSeriePosX == NULL || tabCTPosX == NULL || tabReaderPosX == NULL)
			[self flushMemory:YES];

		[tabSeriePosX init: prefsCache: staticTest];
		[tabCTPosX init: prefsCache: staticTest];
		[tabReaderPosX init: prefsCache: staticTest];
		
		[checkConsistencyWidthPosXRakPrefsDeepData performTest:prefsCache :1 :true];
	}
	return self;
}

- (void) flushMemory : (bool) memoryError
{
	if(tabSerieWidth != NULL)
		[tabSerieWidth release];
	
	if(tabCTWidth != NULL)
		[tabCTWidth release];
	
	if(tabReaderWidth != NULL)
		[tabReaderWidth release];
	
	if(tabSeriePosX != NULL)
		[tabSeriePosX release];
	
	if(tabCTPosX != NULL)
		[tabCTPosX release];
	
	if(tabReaderPosX != NULL)
		[tabReaderPosX release];
	
	if(memoryError)
		[[NSException exceptionWithName:@"NotEnoughMemory"
							 reason:@"We didn't had enough memory to do the job, sorry =/" userInfo:nil] raise];
}

- (NSArray *) executeConsistencyChecks : (uint8) request
{
	NSArray *array = [NSArray alloc];
	if(array == NULL)
		return NULL;
	
	switch(request)
	{
		case 1:
		{
			array = [array initWithObjects:tabSerieWidth, tabCTWidth, tabReaderWidth, tabSeriePosX, tabCTPosX, tabReaderPosX, nil];
			break;
		}
		default:
		{
#ifdef DEV_VERSION
			NSLog(@"WTF! %s couldn't identify request: %d", __PRETTY_FUNCTION__, request);
#endif
			[array release];
			array = NULL;
			break;
		}
	}
	return array;
}

@end
