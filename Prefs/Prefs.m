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
	
	[prefsCache getPrefInternal : requestID : outputContainer];
}

- (void) getPrefInternal : (int) requestID : (void*) outputContainer
{
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
			CGFloat * output = outputContainer;
			*output = [tabSerieSize getDataTab: mainThread : backgroundTabsState: stateTabsReader].size.width;
			break;
		}
			
		case PREFS_GET_TAB_CT_WIDTH:
		{
			CGFloat * output = outputContainer;
			*output = [tabCTSize getDataTab: mainThread : backgroundTabsState: stateTabsReader].size.width;
			break;
		}
			
		case PREFS_GET_TAB_READER_WIDTH:
		{
			CGFloat * output = outputContainer;
			*output = [tabReaderSize getDataTab: mainThread : backgroundTabsState: stateTabsReader].size.width;
			break;
		}
			
		case PREFS_GET_TAB_SERIE_POSX:
		{
			CGFloat * output = outputContainer;
			*output = [tabSerieSize getDataTab: mainThread : backgroundTabsState: stateTabsReader].origin.x;
			break;
		}
			
		case PREFS_GET_TAB_CT_POSX:
		{
			CGFloat * output = outputContainer;
			*output = [tabCTSize getDataTab: mainThread : backgroundTabsState: stateTabsReader].origin.x;
			break;
		}
			
		case PREFS_GET_TAB_READER_POSX:
		{
			CGFloat * output = outputContainer;
			*output = [tabReaderSize getDataTab: mainThread : backgroundTabsState: stateTabsReader].origin.x;
			break;
		}
			
		case PREFS_GET_SERIE_FOOTER_HEIGHT:
		{
			CGFloat *output = outputContainer;
			*output = [tabSerieSize getFooterHeight];
			break;
		}
			
		case PREFS_GET_CT_FOOTER_HEIGHT:
		{
			CGFloat *output = outputContainer;
			*output = [tabCTSize getFooterHeight];
			break;
		}
			
		case PREFS_GET_READER_FOOTER_HEIGHT:
		{
			CGFloat *output = outputContainer;
			*output = [tabReaderSize getFooterHeight];
			break;
		}
			
		case PREFS_GET_MDL_WIDTH:
		{
			CGFloat * output = outputContainer;
			*output = [prefsPosMDL getData: mainThread : stateTabsReader].size.width;
			break;
		}
			
		case PREFS_GET_MDL_HEIGHT:
		{
			CGFloat * output = outputContainer;
			*output = [prefsPosMDL getData: mainThread : stateTabsReader].size.height;
			break;
		}
			
		case PREFS_GET_MDL_POS_Y:
		{
			CGFloat * output = outputContainer;
			*output = [prefsPosMDL getData: mainThread : stateTabsReader].origin.y;
			break;
		}
			
		case PREFS_GET_MDL_POS_X:
		{
			CGFloat * output = outputContainer;
			*output = [prefsPosMDL getData: mainThread : stateTabsReader].origin.x;
			break;
		}
			
		case PREFS_GET_MDL_FRAME:
		{
			NSRect * output = outputContainer;
			*output = [prefsPosMDL getData: mainThread : stateTabsReader];
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
			
			if(value & GUI_THREAD_MDL && !(mainThread & GUI_THREAD_MDL))
				backgroundTabsState = mainThread;
			
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
				NSLog(@"[%s]: Couldn't identify thread :%llu", __PRETTY_FUNCTION__, value);
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

+ (void) directQuery : (uint8_t) request : (uint8_t) subRequest : (uint) mainThreadLocal : (uint) stateTabsReaderLocal : (uint) backgroundTabsStateLocal : (void*) outputContainer
{
	if(prefsCache == NULL)
		[self initCache];
	
	[prefsCache directQueryInternal:request :subRequest :mainThreadLocal :stateTabsReaderLocal :backgroundTabsStateLocal :outputContainer];
	
}

- (void) directQueryInternal : (uint8_t) request : (uint8_t) subRequest : (uint) mainThreadLocal : (uint) stateTabsReaderLocal : (uint) backgroundTabsStateLocal : (void*) outputContainer
{
	if(mainThreadLocal == -1)
		mainThreadLocal = mainThread;
	if(stateTabsReaderLocal == -1)
		stateTabsReaderLocal = stateTabsReader;
	if(backgroundTabsStateLocal == -1)
		backgroundTabsStateLocal = backgroundTabsState;
	
	CGFloat *output = outputContainer;
	*output = -1;
	
	NSRect frame;
	
	switch(request)
	{
		case QUERY_SERIE:
		{
			frame = [tabSerieSize getDataTab: mainThreadLocal : backgroundTabsStateLocal: stateTabsReaderLocal];
			break;
		}
		case QUERY_CT:
		{
			frame = [tabCTSize getDataTab: mainThreadLocal : backgroundTabsStateLocal: stateTabsReaderLocal];
			break;
		}
		case QUERY_READER:
		{
			frame = [tabReaderSize getDataTab: mainThreadLocal : backgroundTabsStateLocal: stateTabsReaderLocal];
			break;
		}
		case QUERY_MDL:
		{
			frame = [prefsPosMDL getData: mainThread : stateTabsReaderLocal];
			break;
		}
	}
	
	if(subRequest == QUERY_GET_HEIGHT)
		*output = frame.size.height;
	else if(subRequest == QUERY_GET_WIDTH)
		*output = frame.size.width;
	else if(subRequest == QUERY_GET_POSX)
		*output = frame.origin.x;
	else if(subRequest == QUERY_GET_POSY)
		*output = frame.origin.y;
}

/************		Private sections		************/

char * loadPref(char request[3], unsigned int length, char defaultChar);

- (id) init
{
	self = [super init];
	if(self != nil)
	{
		uint expectedSize[] = { [RakSizeSeries getExpectedBufferSizeVirtual], [RakSizeCT getExpectedBufferSizeVirtual], [RakSizeReader getExpectedBufferSizeVirtual], [RakMDLSize getExpectedBufferSizeVirtual] };
		int bufferSize = expectedSize[0] + expectedSize[1] + expectedSize[2] + expectedSize[3];
		char *input = loadPref("si", bufferSize, 'f'), recoveryBuffer[bufferSize];

		if(input == NULL)
		{
			input = recoveryBuffer;
			for(int i = 0; i < bufferSize; input[i++] = 'f');
		}
		
		tabSerieSize = [RakSizeSeries alloc];
		tabCTSize = [RakSizeCT alloc];
		tabReaderSize = [RakSizeReader alloc];
		
		if(tabSerieSize == NULL || tabCTSize == NULL || tabReaderSize == NULL)
			[self flushMemory:YES];
		
		[tabSerieSize init: prefsCache: input];
		[tabCTSize init: prefsCache: &input[expectedSize[0]]];
		[tabReaderSize init: prefsCache: &input[expectedSize[0] + expectedSize[1]]];
		
		[checkConsistencyWidthPosXRakPrefsTabDeepData performTest:prefsCache :1 :true];
		
		//Must come after tabs prefs initialization
		prefsPosMDL = [RakMDLSize alloc];
		if(prefsPosMDL == NULL)
			[self flushMemory:YES];
		
		[prefsPosMDL init: prefsCache: &input[expectedSize[0] + expectedSize[1] + expectedSize[2]]];
	
		if(input != recoveryBuffer)
			free(input);
	}
	return self;
}

- (char*) dumpPrefs
{
	char *output = NULL;
	uint expectedSize[] = { [tabSerieSize getExpectedBufferSize], [tabCTSize getExpectedBufferSize], [tabReaderSize getExpectedBufferSize], [prefsPosMDL getExpectedBufferSize] };
	
	output = calloc(expectedSize[0] + expectedSize[1] + expectedSize[2] + expectedSize[3], sizeof(char));
	
	if(output != NULL)
	{
		[tabSerieSize dumpData:		output	: expectedSize[0]];
		[tabCTSize dumpData:		&output[expectedSize[0]] :	expectedSize[1]];
		[tabReaderSize dumpData:	&output[expectedSize[0] + expectedSize[1]] :	expectedSize[2]];
		[prefsPosMDL dumpData:		&output[expectedSize[0] + expectedSize[1] + expectedSize[2]] :	expectedSize[3]];
	}
	
	return output;
}

- (void) flushMemory : (bool) memoryError
{
	if(tabSerieSize != NULL)
		[tabSerieSize release];
	
	if(tabCTSize != NULL)
		[tabCTSize release];
	
	if(tabReaderSize != NULL)
		[tabReaderSize release];
	
	if(prefsPosMDL != NULL)
		[prefsPosMDL release];
	
	[prefsCache release];
	prefsCache = NULL;
	
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
			array = [array initWithObjects: tabSerieSize, tabCTSize, tabReaderSize, nil];
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
