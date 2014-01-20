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


@interface RakPrefsTabDeepData : RakPrefsDeepData
{
	
	NSRect focusSerie;
	NSRect focusCT;
	NSRect focusReader;
	NSRect focusReaderOneCollapsed;
	NSRect focusReaderMainTab;
	NSRect focusReaderAllCollapsed;
	NSRect focusReaderDFMode;
	NSRect focusMDLInSerie;
	NSRect focusMDLInCT;
	NSRect focusMDLInReader;
	
	CGFloat footerHeight;
}

- (id) init : (Prefs*) creator : (char *) inputData;

- (NSRect) getDefaultFocusSerie;
- (NSRect) getDefaultFocusCT;
- (NSRect) getDefaultFocusReader;
- (NSRect) getDefaultFocusReaderOneCollapsed;
- (NSRect) getDefaultFocusReaderMainTab;
- (NSRect) getDefaultFocusReaderAllCollapsed;
- (NSRect) getDefaultFocusReaderDFMode;
- (NSRect) getDefaultFocusMDLInSerie;
- (NSRect) getDefaultFocusMDLInCT;
- (NSRect) getDefaultFocusMDLInReader;
- (CGFloat) getDefaultFooterHeight;

- (NSRect) triggerJumpTableLocal : (SEL) selector;

- (NSRect) getAtIndex: (uint8_t) index;
- (void) setAtIndex: (uint8_t) index : (NSRect) data;
- (NSRect) getDataTab: (int) mainThread : (int) backgroundTabsWhenMDLActive : (int) stateTabsReader;

- (uint8_t) getIndexFromInput: (int) mainThread : (int) backgroundTabsWhenMDLActive : (int) stateTabsReader;
- (CGFloat) getFooterHeight;
- (void) setFooterHeight : (CGFloat) data;


@end

@interface checkConsistencyWidthPosXRakPrefsTabDeepData : NSObject

+ (BOOL) performTest: (Prefs*) mainInstance : (uint8_t) ID : (BOOL) reinitIfError;

@end

/**		Data	**/

@interface RakSizeSeries : RakPrefsTabDeepData

@end

@interface RakSizeCT : RakPrefsTabDeepData

@end

@interface RakSizeReader : RakPrefsTabDeepData

@end

@interface RakPosXReader : RakPrefsTabDeepData

@end