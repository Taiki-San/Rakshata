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
	
	uint8_t focusSerie;
	uint8_t focusCT;
	uint8_t focusReader;
	uint8_t focusReaderOneCollapsed;
	uint8_t focusReaderMainTab;
	uint8_t focusReaderAllCollapsed;
	uint8_t focusReaderDFMode;
	uint8_t focusMDLInSerie;
	uint8_t focusMDLInCT;
	uint8_t focusMDLInReader;
}

- (id) init : (Prefs*) creator : (char *) inputData;
- (void) initJumpTable : (SEL *) jumpTable;
- (void) setAtIndex: (uint8_t) index : (uint8_t) data;
- (uint8_t) getIndexFromInput: (int) mainThread : (int) backgroundTabsWhenMDLActive : (int) stateTabsReader;
- (uint8_t) getAtIndex: (uint8_t) index;
- (uint8_t) getData: (int) mainThread : (int) backgroundTabsWhenMDLActive : (int) stateTabsReader;

- (int) getNbElem;
- (void) performSelfCheck;

- (uint8_t) getDefaultFocusSerie;
- (uint8_t) getDefaultFocusCT;
- (uint8_t) getDefaultFocusReader;
- (uint8_t) getDefaultFocusReaderOneCollapsed;
- (uint8_t) getDefaultFocusReaderMainTab;
- (uint8_t) getDefaultFocusReaderAllCollapsed;
- (uint8_t) getDefaultFocusReaderDFMode;
- (uint8_t) getDefaultFocusMDLInSerie;
- (uint8_t) getDefaultFocusMDLInCT;
- (uint8_t) getDefaultFocusMDLInReader;

@end

@interface checkConsistencyWidthPosXRakPrefsDeepData : NSObject

+ (BOOL) performTest: (Prefs*) mainInstance : (uint8_t) ID : (BOOL) reinitIfError;

@end

/**		Data	**/

@interface RakWidthSeries : RakPrefsDeepData

@end

@interface RakWidthCT : RakPrefsDeepData

@end

@interface RakWidthReader : RakPrefsDeepData

@end

@interface RakPosXSeries : RakPrefsDeepData

@end

@interface RakPosXCT : RakPrefsDeepData

@end

@interface RakPosXReader : RakPrefsDeepData

@end
