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

#define DEFAULT_NUMBER_ELEMS_IN_RakPrefsDeepData 10

@implementation RakPrefsTabDeepData

- (void) setNumberElem
{
	numberElem = DEFAULT_NUMBER_ELEMS_IN_RakPrefsDeepData;
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
	jumpTable[7] = @selector(getDefaultFocusMDLInSerie);
	jumpTable[8] = @selector(getDefaultFocusMDLInCT);
	jumpTable[9] = @selector(getDefaultFocusMDLInReader);
}

- (uint8_t) getFlagFocus
{
	return STATE_READER_TAB_MASK;
}

//Defaults

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

- (uint8_t) getDefaultFocusReaderOneCollapsed
{
	return [self getDefaultFocusReader];
}

- (uint8_t) getDefaultFocusReaderMainTab
{
	return [self getDefaultFocusReader];
}

- (uint8_t) getDefaultFocusReaderAllCollapsed
{
	return [self getDefaultFocusReader];
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

//Getters

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
			return focusReaderOneCollapsed;
			
		case 4:
			return focusReaderMainTab;
			
		case 5:
			return focusReaderAllCollapsed;
			
		case 6:
			return focusReaderDFMode;
			
		case 7:
			return focusMDLInSerie;
			
		case 8:
			return focusMDLInCT;
			
		case 9:
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
		case 7:
		{
			focusMDLInSerie = data;
			break;
		}
		case 8:
		{
			focusMDLInCT = data;
			break;
		}
		case 9:
		{
			focusMDLInReader = data;
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
			ret_value = 6;
		else if(stateTabsReader & STATE_READER_TAB_ALL_COLLAPSED)
			ret_value = 5;
		else if(stateTabsReader & [self getFlagFocus])	//Si on a le focus
			ret_value = 4;
		else
			ret_value = 3;
		
	}
	else if(mainThread & GUI_THREAD_MDL)
	{
		if(backgroundTabsWhenMDLActive & GUI_THREAD_SERIES)
			ret_value = 7;
		else if(backgroundTabsWhenMDLActive & GUI_THREAD_CT)
			ret_value = 8;
		else if(backgroundTabsWhenMDLActive & GUI_THREAD_READER)
			ret_value = 9;
		else
#ifdef DEV_VERSION
			NSLog(@"[%s]: couldn't identify request for MDL: %8x %8x %8x", __PRETTY_FUNCTION__, mainThread, backgroundTabsWhenMDLActive, stateTabsReader);
#endif
	}
	
	return ret_value;
}

@end

@implementation checkConsistencyWidthPosXRakPrefsTabDeepData

//Renvois si le check s'est bien passé, TRUE = OK | FALSE = KO
+ (BOOL) performTest: (Prefs*) mainInstance : (uint8_t) ID : (BOOL) reinitIfError
{
	BOOL ret_value = true;
	NSArray * array = [mainInstance executeConsistencyChecks: 1];
	
	uint i, nbElem = [[array objectAtIndex: 0] getNbElem], otherPan;
	
	for(i = 0; i < nbElem * 2; i++)
	{
		if(i % nbElem == 3 || i % nbElem == 4)	//Le cas particulier où on a un panneau ouvert et un autre replié
			otherPan = i % nbElem == 3 ? 4 : 3;
		else
			otherPan = i % nbElem;
		
		//On vérifie que la pos X du panneau A + la largeur est supérieure ou égale à la pos X du panneau 2
		if([[array objectAtIndex:(i / nbElem) + 3] getAtIndex: (i % nbElem) ] + [[array objectAtIndex: (i / nbElem)] getAtIndex: (i % nbElem) ] < [[array objectAtIndex:(i / nbElem) + 4] getAtIndex: (otherPan) ])
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
	return ret_value;
}

@end
