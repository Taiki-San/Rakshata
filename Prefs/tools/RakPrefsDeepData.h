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

@interface RakPrefsDeepData : NSObject
{
	int numberElem;
	
	uint8_t focusSerie;
	uint8_t focusCT;
	uint8_t focusReader;
	uint8_t focusReaderDFMode;
	uint8_t focusMDLInSerie;
	uint8_t focusMDLInCT;
	uint8_t focusMDLInReader;
}

- (id) init : (char *) inputData;
- (void) initJumpTable : (SEL *) jumpTable;
- (void) setAtIndex: (uint8_t) index : (uint8_t) data;
- (uint8_t) getIndexFromInput: (int) mainThread : (int) backgroundTabsWhenMDLActive : (int) stateTabsReader;
- (uint8_t) getAtIndex: (uint8_t) index;
- (uint8_t) getData: (int) mainThread : (int) backgroundTabsWhenMDLActive : (int) stateTabsReader;

- (uint8_t) getDefaultFocusSerie;
- (uint8_t) getDefaultFocusCT;
- (uint8_t) getDefaultFocusReader;
- (uint8_t) getDefaultFocusReaderDFMode;
- (uint8_t) getDefaultFocusMDLInSerie;
- (uint8_t) getDefaultFocusMDLInCT;
- (uint8_t) getDefaultFocusMDLInReader;

@end

@interface RakWidthSeries : RakPrefsDeepData

@end

@interface RakWidthCT : RakPrefsDeepData

@end

@interface RakWidthReader : RakPrefsDeepData
{
	uint8_t focusReaderPartial;
}

- (uint8_t) getDefaultFocusReaderPartial;
@end

