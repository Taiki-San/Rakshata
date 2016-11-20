/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at https://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

#define DEFAULT_NUMBER_ELEMS_IN_RakPrefsDeepData 8

@implementation RakPrefsTabDeepData

- (instancetype) initWithData : (char *) inputData
{
	self = [super init];
	
	if(self != nil)
	{
		[self setNumberElem];
		[self setExpectedBufferSize];
		
		NSRect dataBuf, dataDefault;
		SEL jumpTable[numberElem];
		
		[self initJumpTable:jumpTable];
		
		for(uint i = 0; i < numberElem - 1; i++)	//Le dernier cas est particulier
		{
			dataBuf.origin.x	= hex2intPrefs(&inputData[16*i], 1000);
			dataBuf.origin.y	= hex2intPrefs(&inputData[16*i + 4], 1000);
			dataBuf.size.height = hex2intPrefs(&inputData[16*i + 8], 1000);
			dataBuf.size.width	= hex2intPrefs(&inputData[16*i + 12], 1000);
			
			//On regarde si il y a des données invalides
			if(dataBuf.origin.x == -1 || dataBuf.origin.y == -1 || dataBuf.size.height == -1 || dataBuf.size.width == -1)
			{
				dataDefault = [self triggerJumpTableLocal:jumpTable[i]];
				
				if(dataBuf.origin.x == -1)
					dataBuf.origin.x = dataDefault.origin.x;
				
				if(dataBuf.origin.y == -1)
					dataBuf.origin.y = dataDefault.origin.y;
				
				if(dataBuf.size.height == -1)
					dataBuf.size.height = dataDefault.size.height;
				
				if(dataBuf.size.width == -1)
					dataBuf.size.width = dataDefault.size.width;
			}
			else
			{
				dataBuf.origin.x	/= 10.0f;
				dataBuf.origin.y	/= 10.0f;
				dataBuf.size.width	/= 10.0f;
				dataBuf.size.height /= 10.0f;
			}
			[self setAtIndex:i :dataBuf];
		}
		
		//cas du footer
		CGFloat footer = hex2intPrefs(&inputData[16 * (numberElem - 1)], 1000);
		
		if(footer == -1)
			footer = [self getDefaultFooterHeight];
		else
			footer /= 10;
		
		[self setFooterHeight:footer];
		
	}
	return self;
}

- (void) setNumberElem
{
	numberElem = DEFAULT_NUMBER_ELEMS_IN_RakPrefsDeepData;
}

- (void) setExpectedBufferSize
{
	sizeInputBuffer = (numberElem-1) * 16 + 4;
}

- (uint) getExpectedBufferSize
{
	return sizeInputBuffer;
}

+ (uint) getExpectedBufferSizeVirtual
{
	return (DEFAULT_NUMBER_ELEMS_IN_RakPrefsDeepData - 1) * 16 + 4;
}


- (void) initJumpTable : (SEL *) jumpTable
{
	jumpTable[0] = @selector(getDefaultFocusSerie);
	jumpTable[1] = @selector(getDefaultFocusCT);
	jumpTable[2] = @selector(getDefaultFocusReader);
	jumpTable[3] = @selector(getDefaultFocusReaderOneCollapsed);
	jumpTable[4] = @selector(getDefaultFocusReaderMainTab);
	jumpTable[5] = @selector(getDefaultFocusReaderAllCollapsed);
	jumpTable[6] = @selector(getDefaultFocusReaderDFMode);
	jumpTable[7] = @selector(getDefaultFooterHeight);
}

- (NSRect) triggerJumpTableLocal : (SEL) selector
{
	NSRect output = NSZeroRect;
	
	if(selector != NULL && [self respondsToSelector:selector])
	{
		NSMethodSignature * signature = [[self class] instanceMethodSignatureForSelector:selector];
		NSInvocation * invocation = [NSInvocation invocationWithMethodSignature:signature];
		
		[invocation setTarget:self];
		[invocation setSelector:selector];
		[invocation invoke];
		[invocation getReturnValue:&output];
	}
	
	return output;
}

//Save state when quit

- (void) dumpData : (char *) output : (uint) length
{
	if(length < sizeInputBuffer)	//Taille du buffer
	{
#ifdef EXTENSIVE_LOGGING
		NSLog(@"[%s]: Not enough room to save prefs: %d < %d", __PRETTY_FUNCTION__, length, sizeInputBuffer);
#endif
		return;
	}
	
	if(output == NULL)
	{
		return;
	}
	
	NSRect frame;
	for(uint i = 0; i < numberElem - 1; i++)
	{
		frame = [self getAtIndex:i];
		
		frame.origin.x =	floor(frame.origin.x * 10 + 0.5);
		frame.origin.y =	floor(frame.origin.y * 10 + 0.5);
		frame.size.width =	floor(frame.size.width * 10 + 0.5);
		frame.size.height = floor(frame.size.height * 10 + 0.5);
		
		snprintf(output, 17, "%04x%04x%04x%04x", (uint) frame.origin.x, (uint) frame.origin.y, (uint) frame.size.height, (uint) frame.size.width);
		output += 16;
	}
	
	//On a pas de \0 final donc on va faire la conversion dans un buffer intermédiaire puis le copie
	CGFloat footer = [self getFooterHeight];
	char buffer[5];
	snprintf(buffer, 5, "%04x", (uint) floor(footer * 10 + 0.5));
	for(uint8_t i = 0; i < 4; i++)
		output[i] = buffer[i];
	
	//AAANNNND, We're done :D
}


//Getters

- (uint8_t) getFlagFocus
{
	return STATE_READER_TAB_MASK;
}

- (NSRect) getDataTab: (uint) mainThread : (uint) stateTabsReader
{
	return [self getAtIndex: [self getIndexFromInput:mainThread :stateTabsReader]];
}

- (void) reinitAtIndex : (uint8_t) index
{
	if(index < numberElem)
	{
		SEL jumpTable[numberElem];
		[self initJumpTable:jumpTable];
		[self setAtIndex:index : [self triggerJumpTableLocal: jumpTable[index]]];
	}
#ifdef EXTENSIVE_LOGGING
	else
		NSLog(@"[%s] : Unknown index: %d", __PRETTY_FUNCTION__, index);
#endif
}

//Defaults

- (NSRect) getDefaultFocusSerie
{
	return NSZeroRect;
}

- (NSRect) getDefaultFocusCT
{
	return NSZeroRect;
}

- (NSRect) getDefaultFocusReader
{
	return NSZeroRect;
}

- (NSRect) getDefaultFocusReaderOneCollapsed
{
	return [self getDefaultFocusReader];
}

- (NSRect) getDefaultFocusReaderMainTab
{
	return [self getDefaultFocusReader];
}

- (NSRect) getDefaultFocusReaderAllCollapsed
{
	return [self getDefaultFocusReader];
}

- (NSRect) getDefaultFocusReaderDFMode
{
	return NSZeroRect;
}

- (CGFloat) getDefaultFooterHeight
{
	return 0;
}

//Getters

- (NSRect) getAtIndex: (uint8_t) index
{
	switch(index)
	{
		case 0:
			return focusSerie;
			
		case 1:
			return focusCT;
			
		case 2:
			return focusReader;
			
		case 3:
			return focusReaderOneCollapsed;
			
		case 4:
			return focusReaderMainTab;
			
		case 5:
			return focusReaderAllCollapsed;
			
		case 6:
			return focusReaderDFMode;
			
		default:
		{
#ifdef EXTENSIVE_LOGGING
			NSLog(@"%s : Couldn't identify the index", __PRETTY_FUNCTION__);
#endif
		}
	}
	return NSZeroRect;
}

- (void) setAtIndex: (uint8_t) index : (NSRect) data
{
	switch(index)
	{
		case 0:
		{
			focusSerie = data;
			break;
		}
		case 1:
		{
			focusCT = data;
			break;
		}
		case 2:
		{
			focusReader = data;
			break;
		}
		case 3:
		{
			focusReaderOneCollapsed = data;
			break;
		}
		case 4:
		{
			focusReaderMainTab = data;
			break;
		}
		case 5:
		{
			focusReaderAllCollapsed = data;
			break;
		}
		case 6:
		{
			focusReaderDFMode = data;
			break;
		}
		default:
		{
#ifdef EXTENSIVE_LOGGING
			NSLog(@"[%s] : Couldn't identify the index: %d", __PRETTY_FUNCTION__, index);
#endif
		}
	}
}

- (uint8_t) getIndexFromInput: (uint) mainThread : (uint) stateTabsReader
{
	uint8_t ret_value = 0xff;
	
	if(mainThread & TAB_SERIES)
		ret_value = 0;
	else if(mainThread & TAB_CT)
		ret_value = 1;
	else if(mainThread & TAB_READER)
	{
		if(stateTabsReader & STATE_READER_TAB_DISTRACTION_FREE)
			ret_value = 6;
		else if(stateTabsReader & STATE_READER_TAB_ALL_COLLAPSED)
			ret_value = 5;
		else if(stateTabsReader & [self getFlagFocus])	//Si on a le focus
			ret_value = 4;
		else
			ret_value = 3;
		
	}
	
	return ret_value;
}

- (CGFloat) getFooterHeight
{
	return footerHeight;
}

- (void) setFooterHeight : (CGFloat) data
{
	footerHeight = data;
}

@end
