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

@class Prefs;

@interface RakPrefsDeepData : NSObject
{
	int numberElem;
	Prefs* mammouth;
}

- (id) init : (Prefs*) creator : (char *) inputData;
- (void) initJumpTable : (SEL *) jumpTable;
- (uint8_t) getAtIndex: (uint8_t) index;
- (uint8_t) getData: (int) mainThread : (int) backgroundTabsWhenMDLActive : (int) stateTabsReader;
- (void) setAtIndex: (uint8_t) index : (uint8_t) data;
- (void) reinitAtIndex : (uint8_t) index;

- (int) getNbElem;
- (void) performSelfCheck;

@end

#import "RakPrefsTabDeepData.h"
#import "RakPrefsMDLDeepData.h"