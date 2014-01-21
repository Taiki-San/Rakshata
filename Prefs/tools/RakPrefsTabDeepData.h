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

CGFloat hex2intPrefs(char hex[2], int maximum);

@interface RakPrefsTabDeepData : RakPrefsDeepData
{
	
	NSRect focusSerie;
	NSRect focusCT;
	NSRect focusReader;
	NSRect focusReaderOneCollapsed;
	NSRect focusReaderMainTab;
	NSRect focusReaderAllCollapsed;
	NSRect focusReaderDFMode;
	
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

//De par son comportement très dépendant des données des autres tabs, RakMDLSize ne dépent pas de RakPrefsDeepData
@interface RakMDLSize : NSObject
{
	Prefs* mammouth;
	
	//Prefs propres au MDL
	CGFloat widthMDLSerie;
	CGFloat heightMDLReaderFocus;
	NSRect focusMDLSize;
}

- (id) init : (Prefs*) creator : (char*) inputData;

- (uint8_t) getFlagFocus;
- (NSRect) getData:(int) mainThread : (int) stateTabsReader;
- (NSRect) getFocusSerie;
- (NSRect) getFocusCT;
- (NSRect) getFocusReader : (int) stateTabsReader;
- (NSRect) getDefaultFocusMDL;
- (CGFloat) getDefaultSerieWidth;
- (CGFloat) getDefaultFocusReaderHeight;

@end

