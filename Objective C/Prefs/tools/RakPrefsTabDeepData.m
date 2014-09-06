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

#define DEFAULT_NUMBER_ELEMS_IN_RakPrefsDeepData 8

@implementation RakPrefsTabDeepData

- (id) init : (Prefs*) creator : (char *) inputData
{
	self = [super init];
	if(self != nil)
	{
		[self setNumberElem];
		[self setExpectedBufferSize];
		
		NSRect dataBuf, dataDefault;
		int i;
		SEL jumpTable[numberElem];
		
		mammouth = creator;
		[self initJumpTable:jumpTable];
		
		for(i = 0; i < numberElem-1; i++)	//Le dernier cas est particulier
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
				dataBuf.origin.x	/= 10;
				dataBuf.origin.y	/= 10;
				dataBuf.size.width	/= 10;
				dataBuf.size.height /= 10;
			}
			[self setAtIndex:i :dataBuf];
		}
		
		//cas du footer
		CGFloat footer = hex2intPrefs(&inputData[16*i], 1000);
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

- (int) getExpectedBufferSize
{
	return sizeInputBuffer;
}

+ (int) getExpectedBufferSizeVirtual
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
	NSRect output = {{0,0},{0,0}};
	
	if (selector != NULL && [self respondsToSelector:selector])
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
#ifdef DEV_VERSION
		NSLog(@"[%s]: Not enough room to save prefs: %d < %d", __PRETTY_FUNCTION__, length, sizeInputBuffer);
#endif
		return;
	}
	
	if(output == NULL)
	{
		return;
	}
	
	NSRect frame;
	for(int i = 0; i < numberElem - 1; i++)
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

- (NSRect) getDataTab: (int) mainThread : (int) stateTabsReader
{
	return [self getAtIndex: [self getIndexFromInput:mainThread :stateTabsReader]];
}

- (void) reinitAtIndex : (uint8_t) index
{
	if(index < numberElem)
	{
		SEL jumpTable[numberElem];
		[self initJumpTable:jumpTable];
		[self setAtIndex:index : [self triggerJumpTableLocal: jumpTable[index]] ];
	}
#ifdef DEV_VERSION
	else
		NSLog(@"[%s] : Unknown index: %d", __PRETTY_FUNCTION__, index);
#endif
}

//Defaults

- (NSRect) getDefaultFocusSerie
{
	NSRect output;
	
	return output;
}

- (NSRect) getDefaultFocusCT
{
	NSRect output;
	
	return output;
}

- (NSRect) getDefaultFocusReader
{
	NSRect output;
	
	return output;
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
	NSRect output;
	
	return output;
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
#ifdef DEV_VERSION
			NSLog(@"%s : Couldn't identify the index", __PRETTY_FUNCTION__);
#endif
		}
	}
	NSRect outputFailure = {{0, 0}, {0, 0}};
	return outputFailure;
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
#ifdef DEV_VERSION
			NSLog(@"[%s] : Couldn't identify the index: %d", __PRETTY_FUNCTION__, index);
#endif
		}
	}
}

- (uint8_t) getIndexFromInput: (int) mainThread : (int) stateTabsReader
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

@implementation checkConsistencyWidthPosXRakPrefsTabDeepData

//Renvois si le check s'est bien passé, TRUE = OK | FALSE = KO
+ (BOOL) performTest: (Prefs*) mainInstance : (uint8_t) ID : (BOOL) reinitIfError
{
	BOOL ret_value = true;
	NSArray * array = [mainInstance initExecuteConsistencyChecks: 1];
	
	if(array == nil)
		return false;
	
	uint i, nbElem = [[array objectAtIndex: 0] getNbElem] - 1, otherPan;
	//nbElem est décrémenté car on ne test pas la hauteur du footer
	
	for(i = 0; i < nbElem * 2; i++)
	{
		if(i % nbElem == 3)			//Le cas particulier où on a un panneau ouvert et un autre replié
			otherPan = 4;
		else if(i % nbElem == 4)	//Same
			otherPan = 3;
		else
			otherPan = i % nbElem;
		
		//On vérifie que la pos X du panneau A + la largeur est supérieure ou égale à la pos X du panneau 2
		if([[array objectAtIndex:(i / nbElem)] getAtIndex: (i % nbElem) ].origin.x + [[array objectAtIndex: (i / nbElem)] getAtIndex: (i % nbElem) ].size.width < [[array objectAtIndex:(i / nbElem) + 1] getAtIndex: (otherPan) ].origin.x)
		{
			ret_value = false;
#ifdef DEV_VERSION
			NSLog(@"[%s] : Incoherency found at index %d", __PRETTY_FUNCTION__, i);
#endif
			if(reinitIfError)
			{
				[[array objectAtIndex: (i / nbElem)] reinitAtIndex: ( i % nbElem)];
				[[array objectAtIndex: (i / nbElem) + 4] reinitAtIndex: ( i % nbElem)];
			}
		}
	}
	[array release];
	return ret_value;
}

@end
