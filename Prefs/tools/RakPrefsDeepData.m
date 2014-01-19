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

uint8_t hex2intPrefs(char hex[2], uint8_t maximum);

@implementation RakPrefsDeepData

- (id) init : (Prefs*) creator : (char *) inputData
{
	self = [super init];
	if(self != nil)
	{
		[self setNumberElem];
		
		uint8_t dataBuf, i;
		SEL jumpTable[numberElem];
		
		mammouth = creator;
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

- (void) initJumpTable : (SEL *) jumpTable
{
	
}

- (void) setNumberElem
{
	numberElem = 0;
}

- (uint8_t) getFlagFocus
{
	return STATE_READER_TAB_MASK;
}

//Getters

- (uint8_t) getAtIndex: (uint8_t) index
{
	return 0xff;
}

- (uint8_t) getIndexFromInput: (int) mainThread : (int) backgroundTabsWhenMDLActive : (int) stateTabsReader
{
	return 0xff;
}

- (void) setAtIndex: (uint8_t) index : (uint8_t) data
{

}

- (void) reinitAtIndex : (uint8_t) index
{
	if(index < numberElem)
	{
		SEL jumpTable[numberElem];
		[self initJumpTable:jumpTable];
		
		[self setAtIndex:index : (uint8_t) [self performSelector: jumpTable[index]] ];
	}
#ifdef DEV_VERSION
	else
		NSLog(@"[%s] : Unknown index: %d", __PRETTY_FUNCTION__, index);
#endif
}

- (void) performSelfCheck
{

}

- (int) getNbElem
{
	return numberElem;
}

@end
