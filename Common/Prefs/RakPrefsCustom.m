//
//  RakPrefsCustom.m
//  Rakshata
//
//  Created by Taiki on 20/11/2016.
//  Copyright © 2016 Taiki. All rights reserved.
//

enum
{
	SERIESMODE_MAX_WIDTH_WHEN_INACTIVE = 225,
	READERMODE_MAX_WIDTH_WHEN_INACTIVE = 320
};

@implementation RakPrefsCustom

#pragma mark Initialization and context restoration

- (void) initializeContextWithProxy : (Prefs *) proxy
{
	//Default
	_mainThread = TAB_DEFAULT;
	_stateTabsReader = STATE_READER_TAB_DEFAULT;
	_favoriteAutoDL = YES;
	_overrideDirection = YES;
	_suggestFromLastRead = YES;
#if !TARGET_OS_IPHONE
	_activePrefsPanel = PREFS_BUTTON_CODE_DEFAULT;
#endif
	
	NSString * context = RakRealApp.savedContext[0];
	if(context == nil)
		proxy.themeCode = 1;
	else
		[self updateContext:context withProxy:proxy];
	
	//The system support the sizes of the various elements to be feed at init
	//We're not using this feature but the code might be usefull in the future
	uint expectedSize[] = { [RakSizeSeries getExpectedBufferSizeVirtual], [RakSizeCT getExpectedBufferSizeVirtual], [RakSizeReader getExpectedBufferSizeVirtual], [RakMDLSize getExpectedBufferSizeVirtual] };
	uint bufferSize = expectedSize[0] + expectedSize[1] + expectedSize[2] + expectedSize[3];
	
	char sizingConfigured[bufferSize];
	for(uint i = 0; i < bufferSize; sizingConfigured[i++] = 'f');

	tabSerieSize = [[RakSizeSeries alloc] initWithData:sizingConfigured];
	tabCTSize = [[RakSizeCT alloc] initWithData: &sizingConfigured[expectedSize[0]]];
	tabReaderSize = [[RakSizeReader alloc] initWithData: &sizingConfigured[expectedSize[0] + expectedSize[1]]];
	prefsPosMDL = [[RakMDLSize alloc] initWithData: &sizingConfigured[expectedSize[0] + expectedSize[1] + expectedSize[2]]];	//Must come last
	
	if(tabSerieSize == nil || tabCTSize == nil || tabReaderSize == nil || prefsPosMDL == nil)
		[proxy flushMemory:YES];
	
	_proxy = proxy;
}

- (void) updateContext : (NSString *) context withProxy : (Prefs *) proxy
{
	NSArray *componentsWithSpaces = [context componentsSeparatedByCharactersInSet:[NSCharacterSet newlineCharacterSet]];
	NSArray *dataState = [componentsWithSpaces filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"length > 0"]];
	
	uint pos = 0;
	
	for(NSString * element in dataState)
	{
		int64_t value = [element longLongValue];
		
		switch (pos++)
		{
			case 0:
			{
				if(value == TAB_SERIES || value == TAB_CT || value == TAB_READER)
					_mainThread = value;
				break;
			}
				
			case 1:
			{
				if(value >= THEME_CODE_DARK && value <= MAX_THEME_ID)
					proxy.themeCode = value;
				
				break;
			}
				
			case 2:
			{
				_favoriteAutoDL = value != 0;
				break;
			}
				
#if !TARGET_OS_IPHONE
			case 3:
			{
				if(value >= PREFS_BUTTON_CODE_GENERAL && value <= PREFS_BUTTON_CODE_CUSTOM)
					_activePrefsPanel = value;
				break;
			}
#endif
				
			case 4:
			{
				_saveMagnification = value != 0;
				break;
			}
				
			case 5:
			{
				_havePDFBackground = value != 0;
				break;
			}
				
			case 6:
			{
				_overrideDirection = value != 0;
				break;
			}
				
			case 7:
			{
				_suggestFromLastRead = value != 0;
				break;
			}
		}
	}
}

- (NSString *) dumpPrefs
{
	return [NSString stringWithFormat:@"%d%@\n%d\n%d\n%d\n%d\n%d", _mainThread, [_proxy dumpProxyPrefs], _favoriteAutoDL, _activePrefsPanel, _saveMagnification, _havePDFBackground, _overrideDirection];
}

- (void) refreshFirstResponder
{
	[firstResponder updateContext:_mainThread :_stateTabsReader];
}

#pragma mark - Color management

- (NSArray <RakColor *> *) getColorThemeWithID : (uint) ID
{
	switch (ID)
	{
		case THEME_CODE_DARK:
			return [self darkColorArray];
			
		case THEME_CODE_LIGHT:
			return [self lightColorArray];
			
		case THEME_CODE_CUSTOM:
			return [self customColorArray];
	}
	
	return nil;
}

- (NSArray <RakColor *> *) darkColorArray
{
	if(_darkColor == nil)
		_darkColor = loadCustomColor([[[NSBundle mainBundle] pathForResource:@CUSTOM_COLOR_FILE ofType:@"json" inDirectory:@"theme 1"] UTF8String]);
	
	return _darkColor;
}

- (NSArray <RakColor *> *) lightColorArray
{
	if(_lightColor == nil)
		_lightColor = loadCustomColor([[[NSBundle mainBundle] pathForResource:@CUSTOM_COLOR_FILE ofType:@"json" inDirectory:@"theme 2"] UTF8String]);
	
	return _lightColor;
}

- (NSArray <RakColor *> *) customColorArray
{
	if(_customColor == nil)
		_customColor = loadCustomColor("theme/"CUSTOM_COLOR_FILE);
	
	return _customColor;
}

#pragma mark - Font

- (NSFont *) getFont : (byte) context ofSize : (CGFloat) size
{
	NSFont * output;
	
	switch (context)
	{
		case FONT_TITLE:
		{
			output = [NSFont fontWithName:@"Futura" size:size];
			break;
		}
		case FONT_STANDARD:
		{
			output = [NSFont systemFontOfSize:size];
			break;
		}
			
		case FONT_AUTHOR_ITALIC:
		case FONT_TAGS:
		case FONT_PLACEHOLDER:
		{
#if !TARGET_OS_IPHONE
			output = [[NSFontManager sharedFontManager] fontWithFamily:[NSFont systemFontOfSize:size].familyName traits:NSItalicFontMask weight:0 size:size];
#else
			output = [UIFont italicSystemFontOfSize:size];
#endif
			break;
		}
		case FONT_RD_BUTTONS:
		case FONT_SR_TITLE:
		case FONT_ABOUT:
		case FONT_PREFS_TITLE:
		{
			output = [NSFont boldSystemFontOfSize:size];
			break;
		}
			
		default:
			output = nil;
	}
	
	return output;
}

#pragma mark - Custom preference code

- (void) getPrefInternal : (uint) requestID : (void*) outputContainer : (void*) additionalData
{
	if(outputContainer == NULL)
		return;
	
	switch(requestID)
	{
		case PREFS_GET_MAIN_THREAD:
		{
			uint* output = outputContainer;
			*output = _mainThread;
			break;
		}
			
		case PREFS_GET_TAB_SERIE_WIDTH:
		{
			CGFloat * output = outputContainer;
			*output = [tabSerieSize getDataTab: _mainThread : _stateTabsReader].size.width;
			
			if(additionalData != NULL)
			{
				*output = percToSize(*output, (*(NSSize *) additionalData).width, _mainThread != TAB_SERIES ? READERMODE_MAX_WIDTH_WHEN_INACTIVE : -1);
				
				//The CT tab may be reduced, so we need to keep that in mind when defining our width
				if(_mainThread == TAB_SERIES)
				{
					CGFloat width = 0;
					[self getPrefInternal:PREFS_GET_TAB_CT_WIDTH :&width :NULL];
					
					width = percToSize(width, (*(NSSize *) additionalData).width, -1);
					if(width > SERIESMODE_MAX_WIDTH_WHEN_INACTIVE)
						*output += width - SERIESMODE_MAX_WIDTH_WHEN_INACTIVE;
				}
			}
			
			break;
		}
			
		case PREFS_GET_TAB_CT_WIDTH:
		{
			CGFloat * output = outputContainer;
			*output = [tabCTSize getDataTab: _mainThread : _stateTabsReader].size.width;
			
			if(additionalData != NULL)
				*output = percToSize(*output, (*(NSSize *) additionalData).width, _mainThread == TAB_READER ? READERMODE_MAX_WIDTH_WHEN_INACTIVE : (_mainThread == TAB_SERIES ? SERIESMODE_MAX_WIDTH_WHEN_INACTIVE : -1));
			
			break;
		}
			
		case PREFS_GET_TAB_CT_POSX:
		{
			if(_mainThread == TAB_READER && _stateTabsReader & (STATE_READER_TAB_SERIE_FOCUS | STATE_READER_TAB_MDL_FOCUS))
			{
				[self getPrefInternal:PREFS_GET_TAB_SERIE_WIDTH :outputContainer :additionalData];
			}
			else
			{
				NSRect frame = [tabCTSize getDataTab: _mainThread : _stateTabsReader];
				CGFloat * output = outputContainer;
				*output = frame.origin.x;
				
				if(additionalData != NULL)
				{
					*output = percToSize(*output, (*(NSSize *) additionalData).width, -1);
					
					if(_mainThread == TAB_SERIES)
					{
						frame.size.width = percToSize(frame.size.width, (*(NSSize *) additionalData).width, -1);
						if(frame.size.width > SERIESMODE_MAX_WIDTH_WHEN_INACTIVE)
							*output += frame.size.width - SERIESMODE_MAX_WIDTH_WHEN_INACTIVE;
					}
				}
			}
			break;
		}
			
		case PREFS_GET_TAB_READER_POSX:
		{
			//Classical code
			CGFloat * output = outputContainer;
			*output = [tabReaderSize getDataTab: _mainThread : _stateTabsReader].origin.x;
			
			if(additionalData == NULL)
				break;
			
			*output = percToSize(*output, (*(NSSize *) additionalData).width, -1);
			
			//Reader position is highly dependant of the width of either SER/CT tabs, we need to check we're not impacted if they were maximized
			if(_mainThread == TAB_READER && _stateTabsReader & STATE_READER_NONE_COLLAPSED)
			{
				CGFloat data, percentage;
				
				if(_stateTabsReader & (STATE_READER_TAB_SERIE_FOCUS | STATE_READER_TAB_MDL_FOCUS))
				{
					[self getPrefInternal:PREFS_GET_TAB_SERIE_WIDTH : &data : additionalData];
					if(data == READERMODE_MAX_WIDTH_WHEN_INACTIVE)	//Maximized
					{
						[self getPrefInternal : PREFS_GET_TAB_SERIE_WIDTH : &percentage : NULL];	//We get the real percentage
						*output -= percToSize(percentage, (*(NSSize *) additionalData).width, -1) - data;	//We reduced our base position by the diff between the real width, and the minimized
					}
				}
				
				//The same code for CT tab
				if(_stateTabsReader & STATE_READER_TAB_CT_FOCUS)
				{
					[self getPrefInternal:PREFS_GET_TAB_CT_WIDTH : &data : additionalData];
					if(data == READERMODE_MAX_WIDTH_WHEN_INACTIVE)	//Maximized
					{
						[self getPrefInternal : PREFS_GET_TAB_CT_WIDTH : &percentage : NULL];
						*output -= percToSize(percentage, (*(NSSize *) additionalData).width, -1) - data;
					}
				}
			}
			
			break;
		}
			
		case PREFS_GET_CT_FOOTER_HEIGHT:
		{
			CGFloat *output = outputContainer;
			*output = [tabCTSize getFooterHeight];
			
			if(additionalData != NULL)
				*output = percToSize(*output, (*(NSSize *) additionalData).height, -1);
			
			break;
		}
			
		case PREFS_GET_READER_FOOTER_HEIGHT:
		{
			CGFloat *output = outputContainer;
			*output = [tabReaderSize getFooterHeight];
			
			if(additionalData != NULL)
				*output = percToSize(*output, (*(NSSize *) additionalData).height, -1);
			
			break;
		}
			
		case PREFS_GET_TAB_SERIE_FRAME:
		{
			NSRect data = [tabSerieSize getDataTab: _mainThread : _stateTabsReader];
			*(NSRect *) outputContainer = additionalData == NULL ? data : prefsPercToFrame(data, *(NSSize*) additionalData);
			
			if(_mainThread == TAB_READER && additionalData != NULL && ((NSRect*) outputContainer)->size.width > READERMODE_MAX_WIDTH_WHEN_INACTIVE)
				((NSRect*) outputContainer)->size.width = READERMODE_MAX_WIDTH_WHEN_INACTIVE;
			
			else if(_mainThread == TAB_SERIES && additionalData != NULL)
				[self getPrefInternal : PREFS_GET_TAB_SERIE_WIDTH : &(((NSRect *)outputContainer)->size.width) : additionalData];
			
			break;
		}
			
		case PREFS_GET_TAB_CT_FRAME:
		{
			NSRect data = [tabCTSize getDataTab: _mainThread : _stateTabsReader];
			*(NSRect *) outputContainer = additionalData == NULL ? data : prefsPercToFrame(data, *(NSSize*) additionalData);
			
			if(_mainThread == TAB_READER && additionalData != NULL && ((NSRect*) outputContainer)->size.width > READERMODE_MAX_WIDTH_WHEN_INACTIVE)
			{
				((NSRect*) outputContainer)->size.width = READERMODE_MAX_WIDTH_WHEN_INACTIVE;
				[self getPrefInternal : PREFS_GET_TAB_CT_POSX : &(((NSRect *)outputContainer)->origin.x) : additionalData];
			}
			else if(_mainThread == TAB_SERIES && additionalData != NULL && ((NSRect*) outputContainer)->size.width > SERIESMODE_MAX_WIDTH_WHEN_INACTIVE)
			{
				((NSRect*) outputContainer)->size.width = SERIESMODE_MAX_WIDTH_WHEN_INACTIVE;
				[self getPrefInternal : PREFS_GET_TAB_CT_POSX : &(((NSRect *)outputContainer)->origin.x) : additionalData];
			}
			
			break;
		}
			
		case PREFS_GET_TAB_READER_FRAME:
		{
			NSRect data = [tabReaderSize getDataTab: _mainThread : _stateTabsReader];
			*(NSRect *) outputContainer = additionalData == NULL ? data : prefsPercToFrame(data, *(NSSize*) additionalData);
			
			[self getPrefInternal: PREFS_GET_TAB_READER_POSX : &(((NSRect *) outputContainer)->origin.x) : additionalData];
			
			break;
		}
			
		case PREFS_GET_MDL_FRAME:
		{
			NSRect data = [prefsPosMDL getData: _mainThread : _stateTabsReader];
			*(NSRect *) outputContainer = additionalData == NULL ? data : prefsPercToFrame(data, *(NSSize*) additionalData);
			
			if(_mainThread == TAB_READER && additionalData != NULL)
			{
				CGFloat maxWidth = 0;
				[self getPrefInternal: PREFS_GET_TAB_READER_POSX : &maxWidth : additionalData];
				maxWidth = MAX(READERMODE_MAX_WIDTH_WHEN_INACTIVE, maxWidth);
				
				if(((NSRect*) outputContainer)->size.width > maxWidth)
					((NSRect*) outputContainer)->size.width = maxWidth;
			}
			
			break;
		}
			
		case PREFS_GET_IS_READER_MT:
		{
			BOOL * data = outputContainer;
			*data = (_mainThread & TAB_READER) != 0;
			break;
		}
			
		case PREFS_GET_READER_TABS_STATE:
		{
			uint * output = outputContainer;
			*output = _stateTabsReader;
			break;
		}
			
		case PREFS_GET_SCROLLER_STYLE:
		{
			*(byte*) outputContainer = SCROLLER_STYLE_LARGE;
			break;
		}
			
		case PREFS_GET_FAVORITE_AUTODL:
		{
			* (bool *) outputContainer = _favoriteAutoDL;
			break;
		}
			
		case PREFS_GET_ACTIVE_PREFS_PANEL:
		{
			* (byte *) outputContainer = _activePrefsPanel;
			break;
		}
			
		case PREFS_GET_SAVE_MAGNIFICATION:
		{
			* (BOOL *) outputContainer = _saveMagnification;
			break;
		}
			
		case PREFS_GET_HAVE_PDF_BACKGROUND:
		{
			* (BOOL *) outputContainer = _havePDFBackground;
			break;
		}
			
		case PREFS_GET_DIROVERRIDE:
		{
			* (BOOL *) outputContainer = _overrideDirection;
			break;
		}
			
		case PREFS_GET_SUGGEST_FROM_LAST_READ:
		{
			* (BOOL *) outputContainer = _suggestFromLastRead;
			break;
		}
			
		default:
		{
			NSLog(@"Couldn't identify request: %d", requestID);
		}
	}
}

- (BOOL) setPref : (uint) requestID atValue: (uint64_t) value
{
	BOOL ret_value = NO;
	
	switch (requestID)
	{
		case PREFS_SET_OWNMAINTAB:
		{
			ret_value = _mainThread != (uint) value;
			if(ret_value)
			{
				_mainThread = value & TAB_MASK;
				[self refreshFirstResponder];
				
				[RakApp setHaveDistractionFree : _mainThread == TAB_READER];
			}
			
			break;
		}
			
		case PREFS_SET_FAVORITE_AUTODL:
		{
			ret_value = _favoriteAutoDL != (bool) value;
			
			if(ret_value)
				_favoriteAutoDL = value;
			
			break;
		}
			
		case PREFS_SET_READER_TABS_STATE:
		{
			ret_value = _stateTabsReader != (uint) value;
			if(ret_value)
			{
				_stateTabsReader = value & STATE_READER_TAB_MASK;
				[self refreshFirstResponder];
			}
			
			break;
		}
			
		case PREFS_SET_READER_DISTRACTION_FREE:
		{
			if(value && _stateTabsReader != STATE_READER_TAB_DISTRACTION_FREE)
			{
				_stateTabsReader = STATE_READER_TAB_DISTRACTION_FREE;
				ret_value = YES;
			}
			else if(!value && _stateTabsReader == STATE_READER_TAB_DISTRACTION_FREE)
			{
				_stateTabsReader = STATE_READER_TAB_ALL_COLLAPSED;
				ret_value = YES;
			}
			break;
		}
			
		case PREFS_SET_READER_TABS_STATE_FROM_CALLER:
		{
			//Only relevant in the reader
			if(_mainThread != TAB_READER)
				break;
			
			uint newValue = INVALID_VALUE;
			switch(value)
			{
				case TAB_SERIES:
				{
					newValue = STATE_READER_TAB_SERIE_FOCUS;
					break;
				}
				case TAB_CT:
				{
					newValue = STATE_READER_TAB_CT_FOCUS;
					break;
				}
				case TAB_MDL:
				{
					newValue = STATE_READER_TAB_MDL_FOCUS;
					break;
				}
				case TAB_READER:
				{
					newValue = STATE_READER_TAB_ALL_COLLAPSED;
					break;
				}
					
				case TAB_READER_DF:
				{
					newValue = STATE_READER_TAB_DISTRACTION_FREE;
					break;
				}
			}
			if(newValue != INVALID_VALUE)
			{
				if(_stateTabsReader != STATE_READER_TAB_DISTRACTION_FREE)
				{
					ret_value = _stateTabsReader != newValue;
					_stateTabsReader = newValue;
				}
			}
#ifdef EXTENSIVE_LOGGING
			else
			{
				NSLog(@"[%s]: Couldn't identify thread :%llu", __PRETTY_FUNCTION__, value);
			}
#endif
			break;
		}
			
		case PREFS_SET_ACTIVE_PREFS_PANEL:
		{
			ret_value = _activePrefsPanel != value;
			_activePrefsPanel = value;
			break;
		}
			
		case PREFS_SET_SAVE_MAGNIFICATION:
		{
			value = value != 0;
			
			ret_value = _saveMagnification != (BOOL) value;
			
			if(ret_value)
				_saveMagnification = value;
			
			break;
		}
			
		case PREFS_SET_HAVE_PDF_BACKGROUND:
		{
			value = value != 0;
			
			ret_value = _havePDFBackground != (BOOL) value;
			
			if(ret_value)
				_havePDFBackground = value;
			
			break;
		}
			
		case PREFS_SET_DIROVERRIDE:
		{
			value = value != 0;
			
			ret_value = _overrideDirection != (BOOL) value;
			
			if(ret_value)
				_overrideDirection = value;
			
			break;
		}
			
		default:
			break;
	}
	return ret_value;
}

#pragma mark - Sizing code

- (void) directQueryInternal : (uint8_t) request : (uint8_t) subRequest : (uint) mainThreadLocal : (uint) stateTabsReaderLocal : (CGFloat*) output
{
	if(output == NULL)
		return;
	
	if(mainThreadLocal == INVALID_VALUE)
		mainThreadLocal = _mainThread;
	
	if(stateTabsReaderLocal == INVALID_VALUE)
		stateTabsReaderLocal = _stateTabsReader;
	
	NSRect frame;
	
	switch(request)
	{
		case QUERY_SERIE:
		{
			frame = [tabSerieSize getDataTab: mainThreadLocal : stateTabsReaderLocal];
			break;
		}
		case QUERY_CT:
		{
			frame = [tabCTSize getDataTab: mainThreadLocal : stateTabsReaderLocal];
			break;
		}
		case QUERY_READER:
		{
			frame = [tabReaderSize getDataTab: mainThreadLocal : stateTabsReaderLocal];
			break;
		}
		case QUERY_MDL:
		{
			frame = [prefsPosMDL getData: _mainThread : stateTabsReaderLocal];
			break;
		}
		default:
		{
#ifdef EXTENSIVE_LOGGING
			NSLog(@"%s: Received garbage", __PRETTY_FUNCTION__);
#endif
			*output = 0;
			return;
		}
	}
	
	if(subRequest == QUERY_GET_WIDTH)
		*output = frame.size.width;
	else if(subRequest == QUERY_GET_HEIGHT)
		*output = frame.size.height;
	else if(subRequest == QUERY_GET_POSX)
		*output = frame.origin.x;
	else if(subRequest == QUERY_GET_POSY)
		*output = frame.origin.y;
	else
		*output = -1;
}

#pragma mark - KVO

- (NSString *) getKeyPathForCode : (byte) code
{
	switch (code)
	{
		case KVO_MAIN_THREAD:
			return @"mainThread";
			
		case KVO_PDF_BACKGRND:
			return @"havePDFBackground";
			
		case KVO_MAGNIFICATION:
			return @"saveMagnification";
			
		case KVO_DIROVERRIDE:
			return @"overrideDirection";
	}
	
	return nil;
}


@end
