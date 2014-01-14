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
 *********************************************************************************************/

#include "superHeader.h"

#define DEFAULT_NUMBER_ELEMS_IN_RakPrefsDeepData 7

uint8_t hex2intPrefs(char hex[2], uint8_t maximum);

@implementation RakPrefsDeepData

- (id) init : (char *) inputData
{
	self = [super init];
	[self setNumberElem];
	if(self != nil)
	{
		uint8_t dataBuf, i;
		SEL jumpTable[numberElem];
		[self initJumpTable:jumpTable];
		
		for(i = 0; i < numberElem; i++)
		{
			dataBuf = hex2intPrefs(&inputData[2*i], 100);
			if(dataBuf == 0xFF)
				dataBuf = (uint8_t) [self performSelector:jumpTable[i]];
			[self setAtIndex:i :dataBuf];
		}
	}
	return self;
}

- (void) setNumberElem
{
	numberElem = DEFAULT_NUMBER_ELEMS_IN_RakPrefsDeepData;
}

- (uint8_t) getData: (int) mainThread : (int) backgroundTabsWhenMDLActive : (int) stateTabsReader
{
	return [self getAtIndex: [self getIndexFromInput:mainThread :backgroundTabsWhenMDLActive :stateTabsReader]];
}

- (uint8_t) getIndexFromInput: (int) mainThread : (int) backgroundTabsWhenMDLActive : (int) stateTabsReader
{
	uint8_t ret_value = 0xff;
	
	if(mainThread & GUI_THREAD_SERIES)
		ret_value = 0;
	else if(mainThread & GUI_THREAD_CT)
		ret_value = 1;
	else if(mainThread & GUI_THREAD_READER)
	{
		if(stateTabsReader & STATE_READER_TAB_DISTRACTION_FREE)
			ret_value = 3;
		else
			ret_value = 2;
		
	}
	else if(mainThread & GUI_THREAD_MDL)
	{
		if(backgroundTabsWhenMDLActive & GUI_THREAD_SERIES)
			ret_value = 4;
		else if(backgroundTabsWhenMDLActive & GUI_THREAD_CT)
			ret_value = 5;
		else if(backgroundTabsWhenMDLActive & GUI_THREAD_READER)
			ret_value = 6;
		else
#ifdef DEV_VERSION
			NSLog(@"%s: couldn't identify request for MDL: %8x %8x %8x", __PRETTY_FUNCTION__, mainThread, backgroundTabsWhenMDLActive, stateTabsReader);
#endif
	}
	
	return ret_value;
}

- (void) initJumpTable : (SEL *) jumpTable
{
	jumpTable[0] = @selector(getDefaultFocusSerie);
	jumpTable[1] = @selector(getDefaultFocusCT);
	jumpTable[2] = @selector(getDefaultFocusReader);
	jumpTable[3] = @selector(getDefaultFocusReaderDFMode);
	jumpTable[4] = @selector(getDefaultFocusMDLInSerie);
	jumpTable[5] = @selector(getDefaultFocusMDLInCT);
	jumpTable[6] = @selector(getDefaultFocusMDLInReader);
}

- (void) setAtIndex: (uint8_t) index : (uint8_t) data
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
			focusReaderDFMode = data;
			break;
		}
		case 4:
		{
			focusMDLInSerie = data;
			break;
		}
		case 5:
		{
			focusMDLInCT = data;
			break;
		}
		case 6:
		{
			focusMDLInReader = data;
			break;
		}
		default:
		{
#ifdef DEV_VERSION
			NSLog(@"%s : Couldn't identify the index", __PRETTY_FUNCTION__);
#endif
		}
	}
}

- (uint8_t) getAtIndex: (uint8_t) index
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
			return focusReaderDFMode;

		case 4:
			return focusMDLInSerie;

		case 5:
			return focusMDLInCT;

		case 6:
			return focusMDLInReader;

		default:
		{
#ifdef DEV_VERSION
			NSLog(@"%s : Couldn't identify the index", __PRETTY_FUNCTION__);
#endif
		}
	}
	return 0xff;
}

- (uint8_t) getDefaultFocusSerie
{
	return 0;
}

- (uint8_t) getDefaultFocusCT
{
	return 0;
}

- (uint8_t) getDefaultFocusReader
{
	return 0;
}

- (uint8_t) getDefaultFocusReaderDFMode
{
	return 0;
}

- (uint8_t) getDefaultFocusMDLInSerie
{
	return 0;
}

- (uint8_t) getDefaultFocusMDLInCT
{
	return 0;
}

- (uint8_t) getDefaultFocusMDLInReader
{
	return 0;
}

@end

@implementation RakWidthSeries

- (uint8_t) getDefaultFocusSerie
{
	return TAB_SERIE_ACTIVE;
}

- (uint8_t) getDefaultFocusCT
{
	return TAB_SERIE_INACTIVE_CT;
}

- (uint8_t) getDefaultFocusReader
{
	return TAB_SERIE_INACTIVE_LECTEUR;
}

- (uint8_t) getDefaultFocusReaderDFMode
{
	return TAB_SERIE_INACTIVE_DISTRACTION_FREE;
}

- (uint8_t) getDefaultFocusMDLInSerie
{
	return [self getDefaultFocusSerie];
}

- (uint8_t) getDefaultFocusMDLInCT
{
	return [self getDefaultFocusCT];
}

- (uint8_t) getDefaultFocusMDLInReader
{
	return [self getDefaultFocusReader];
}

@end

@implementation RakWidthCT

- (uint8_t) getDefaultFocusSerie
{
	return TAB_CT_INACTIVE_SERIE;
}

- (uint8_t) getDefaultFocusCT
{
	return TAB_CT_ACTIVE;
}

- (uint8_t) getDefaultFocusReader
{
	return TAB_CT_INACTIVE_LECTEUR;
}

- (uint8_t) getDefaultFocusReaderDFMode
{
	return TAB_CT_INACTIVE_DISTRACTION_FREE;
}

- (uint8_t) getDefaultFocusMDLInSerie
{
	return [self getDefaultFocusSerie];
}

- (uint8_t) getDefaultFocusMDLInCT
{
	return [self getDefaultFocusCT];
}

- (uint8_t) getDefaultFocusMDLInReader
{
	return [self getDefaultFocusReader];
}

@end

@implementation RakWidthReader

- (void) setNumberElem
{
	numberElem = 8;
}

- (void) setAtIndex: (uint8_t) index : (uint8_t) data
{
	if(index == 7)
	{
		focusReaderPartial = data;
	}
	else
	{
		[super setAtIndex:index :data];
	}
}

- (uint8_t) getIndexFromInput: (int) mainThread : (int) backgroundTabsWhenMDLActive : (int) stateTabsReader
{
	if(mainThread & GUI_THREAD_READER && stateTabsReader & (STATE_READER_TAB_SERIE_FOCUS | STATE_READER_TAB_CT_FOCUS))
		return 7;
	else
		return [super getIndexFromInput:mainThread :backgroundTabsWhenMDLActive :stateTabsReader];
}

- (uint8_t) getAtIndex: (uint8_t) index
{
	if(index == 7)
		return focusReaderPartial;
	else
		return [super getAtIndex:index];
}

- (void) initJumpTable : (SEL *) jumpTable
{
	[super initJumpTable:jumpTable];
	jumpTable[7] = @selector(getDefaultFocusReaderPartial);
}

- (uint8_t) getDefaultFocusSerie
{
	return TAB_READER_INACTIVE_SERIE;
}

- (uint8_t) getDefaultFocusCT
{
	return TAB_READER_INACTIVE_CT;
}

- (uint8_t) getDefaultFocusReader
{
	return TAB_READER_ACTIVE;		//All Collapsed
}

- (uint8_t) getDefaultFocusReaderDFMode
{
	return TAB_READER_ACTIVE_DISTRACTION_FREE;
}

- (uint8_t) getDefaultFocusMDLInSerie
{
	return [self getDefaultFocusSerie];
}

- (uint8_t) getDefaultFocusMDLInCT
{
	return [self getDefaultFocusCT];
}

- (uint8_t) getDefaultFocusMDLInReader
{
	return [self getDefaultFocusReader];
}

- (uint8_t) getDefaultFocusReaderPartial
{
	return TAB_READER_ACTIVE_PARTIAL;
}

@end
