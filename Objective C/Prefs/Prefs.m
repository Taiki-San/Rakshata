/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite  **
 **                                                                                         **
 ********************************************************************************************/

Prefs* prefsCache;

// Contexte
uint mainThread = GUI_THREAD_READER;				//Default : GUI_THREAD_SERIES
uint stateTabsReader = STATE_READER_TAB_DEFAULT;	//Default : STATE_READER_TAB_DEFAULT
uint backgroundTabsState = GUI_THREAD_SERIES;		//Background tab when MDL have focus

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

+ (void) syncCacheToDisk
{
	if(prefsCache != nil)
	{
		[prefsCache dumpPrefs];
	}
}

+ (NSColor*) getSystemColor : (byte) context
{
	NSColor* output;
	switch (context)
	{
		case GET_COLOR_EXTERNALBORDER_FAREST:
		{
			output = [NSColor colorWithDeviceWhite:20/255.0f alpha:1.0];
			break;
		}
		case GET_COLOR_EXTERNALBORDER_MIDDLE:
		{
			output = [NSColor colorWithDeviceWhite:32/255.0f alpha:1.0];
			break;
		}
		case GET_COLOR_EXTERNALBORDER_CLOSEST:
		{
			output = [NSColor colorWithDeviceWhite:50/255.0f alpha:1.0];
			break;
		}
		case GET_COLOR_INACTIVE:
		{
			output = [NSColor colorWithSRGBRed:104/255.0f green:143/255.0f blue:71/255.0f alpha:1.0];
			break;
		}
		case GET_COLOR_SURVOL:
		{
			output = [NSColor colorWithSRGBRed:136/255.0f green:177/255.0f blue:102/255.0f alpha:1.0];
			break;
		}
		case GET_COLOR_ACTIVE:
		{
			output = [NSColor colorWithSRGBRed:191/255.0f green:228/255.0f blue:160/255.0f alpha:1.0];
			break;
		}
		case GET_COLOR_READER_BAR:
		{
			output = [NSColor colorWithDeviceWhite:20/255.0f alpha:0.8];
			break;
		}
		case GET_COLOR_READER_BAR_FRONT:
		{
			output = [NSColor colorWithDeviceWhite:75/255.0f alpha:0.8];
			break;
		}
			
		case GET_COLOR_READER_BAR_PAGE_COUNTER:
		{
			output = [NSColor colorWithDeviceWhite:25/255.0f alpha:1];
			break;
		}
			
		case GET_COLOR_BACKGROUND_READER_INTAB:
		{
			output = [NSColor colorWithDeviceWhite:42/255.0f alpha:1.0];
			break;
		}
		case GET_COLOR_BACKGROUND_TABS:
		{
			output = [NSColor colorWithDeviceWhite:47/255.0f alpha:1.0];
			break;
		}
		case GET_COLOR_BORDER_TABS:
		{
			output = [NSColor colorWithDeviceWhite:52/255.0f alpha:1.0];
			break;
		}
			
		case GET_COLOR_BACKGROUD_BACK_BUTTONS:
		{
			output = [NSColor colorWithDeviceWhite:39/255.0f alpha:1.0];
			break;
		}
			
			//To improve
		case GET_COLOR_BACKGROUD_BACK_BUTTONS_ANIMATING:
		{
			output = [NSColor colorWithDeviceWhite:100/255.0f alpha:1.0];
			break;
		}
			
		case GET_COLOR_BACKGROUD_CT_READERMODE:
		case GET_COLOR_BACKGROUD_SR_READERMODE:
		{
			output = [NSColor colorWithDeviceWhite:34/255.0f alpha:1.0];
			break;
		}

		case GET_COLOR_TEXT_CT_SELECTOR_UNAVAILABLE:
		{
			output = [NSColor colorWithSRGBRed:104/255.0f green:143/255.0f blue:71/255.0f alpha:1.0];
			break;
		}
		
		case GET_COLOR_TEXT_CT_SELECTOR_CLICKED:
		{
			output = [NSColor colorWithSRGBRed:191/255.0f green:228/255.0f blue:160/255.0f alpha:1.0];
			break;
		}
			
		case GET_COLOR_TEXT_CT_SELECTOR_NONCLICKED:
		{
			output = [NSColor colorWithSRGBRed:136/255.0f green:177/255.0f blue:102/255.0f alpha:1.0];
			break;
		}
		
			//To improve
		case GET_COLOR_BACKGROUND_CT_TVCELL:
		{
			output = [NSColor colorWithDeviceWhite:100/255.0f alpha:1.0];
			break;
		}
			
		default:
		{
			return nil;
		}
	}
	
	return output;
}

+ (NSString *) getFontName : (byte) context
{
	NSString * output = nil;
	
	switch (context)
	{
		case GET_FONT_TITLE:
		{
			output = @"Futura";
			break;
		}
		case GET_FONT_STANDARD:
		{
			output = @"Helvetica";
			break;
		}
		case GET_FONT_RD_BUTTONS:
		{
			output = @"Helvetica-Bold";
			break;
		}
	}
	
	return output;
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
			
		case PREFS_GET_MDL_WIDTH:
		{
			CGFloat * output = outputContainer;
			*output = [prefsPosMDL getData: mainThread : stateTabsReader].size.width;
			break;
		}
			
		case PREFS_GET_TAB_SERIE_HEIGHT:
		{
			CGFloat * output = outputContainer;
			*output = [tabSerieSize getDataTab: mainThread : backgroundTabsState: stateTabsReader].size.height;
			break;
		}
			
		case PREFS_GET_TAB_CT_HEIGHT:
		{
			CGFloat * output = outputContainer;
			*output = [tabCTSize getDataTab: mainThread : backgroundTabsState: stateTabsReader].size.height;
			break;
		}
			
		case PREFS_GET_TAB_READER_HEIGHT:
		{
			CGFloat * output = outputContainer;
			*output = [tabReaderSize getDataTab: mainThread : backgroundTabsState: stateTabsReader].size.height;
			break;
		}
		
		case PREFS_GET_MDL_HEIGHT:
		{
			CGFloat * output = outputContainer;
			*output = [prefsPosMDL getData: mainThread : stateTabsReader].size.height;
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
			
		case PREFS_GET_MDL_POS_X:
		{
			CGFloat * output = outputContainer;
			*output = [prefsPosMDL getData: mainThread : stateTabsReader].origin.x;
			break;
		}
			
		case PREFS_GET_TAB_SERIE_POSY:
		{
			CGFloat * output = outputContainer;
			*output = [tabSerieSize getDataTab: mainThread : backgroundTabsState: stateTabsReader].origin.y;
			break;
		}
			
		case PREFS_GET_TAB_CT_POSY:
		{
			CGFloat * output = outputContainer;
			*output = [tabCTSize getDataTab: mainThread : backgroundTabsState: stateTabsReader].origin.y;
			break;
		}
			
		case PREFS_GET_TAB_READER_POSY:
		{
			CGFloat * output = outputContainer;
			*output = [tabReaderSize getDataTab: mainThread : backgroundTabsState: stateTabsReader].origin.y;
			break;
		}
			
		case PREFS_GET_MDL_POSY:
		{
			CGFloat * output = outputContainer;
			*output = [prefsPosMDL getData: mainThread : stateTabsReader].origin.y;
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
			
		case PREFS_GET_TAB_SERIE_FRAME:
		{
			NSRect * output = outputContainer;
			*output = [tabSerieSize getDataTab: mainThread : backgroundTabsState: stateTabsReader];
			break;
		}
			
		case PREFS_GET_TAB_CT_FRAME:
		{
			NSRect * output = outputContainer;
			*output = [tabCTSize getDataTab: mainThread : backgroundTabsState: stateTabsReader];
			break;
		}
			
		case PREFS_GET_TAB_READER_FRAME:
		{
			NSRect * output = outputContainer;
			*output = [tabReaderSize getDataTab: mainThread : backgroundTabsState: stateTabsReader];
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
			[prefsCache refreshFirstResponder];
			
			break;
		}
			
		case PREFS_SET_READER_TABS_STATE:
		{
			ret_value = stateTabsReader != (uint) value;
			stateTabsReader = value & STATE_READER_TAB_MASK;

			[prefsCache refreshFirstResponder];
			
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

- (void) directQueryInternal : (uint8_t) request : (uint8_t) subRequest : (uint) mainThreadLocal : (uint) stateTabsReaderLocal : (uint) backgroundTabsStateLocal : (CGFloat*) output
{
	if(output == NULL)
		return;
	if(mainThreadLocal == -1)
		mainThreadLocal = mainThread;
	if(stateTabsReaderLocal == -1)
		stateTabsReaderLocal = stateTabsReader;
	if(backgroundTabsStateLocal == -1)
		backgroundTabsStateLocal = backgroundTabsState;
	
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
		default:
		{
#ifdef DEV_VERSION
			NSLog(@"%s: Received garbage", __PRETTY_FUNCTION__);
#else
			NSLog(@"Received garbage");
#endif
			memset(output, 0, sizeof(*output));
			return;
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
		
		firstResponder = [(RakContentViewBack *) mainWindowShouldNotBeAccessedWithoutReallyGoodReason.contentView getFirstResponder];
		[self refreshFirstResponder];
	
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

- (void) refreshFirstResponder
{
	[firstResponder updateContext:mainThread :stateTabsReader];
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

- (NSArray *) initExecuteConsistencyChecks : (uint8) request
{
	NSArray *array = [NSArray alloc];
	if(array == nil)
		return nil;
	
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
			array = nil;
			break;
		}
	}
	return array;
}

@end
