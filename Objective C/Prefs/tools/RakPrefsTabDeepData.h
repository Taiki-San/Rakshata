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

- (instancetype) init : (Prefs*) creator : (char *) inputData;
- (void) dumpData : (char *) output : (uint) length;

- (void) setExpectedBufferSize;
- (int) getExpectedBufferSize;
+ (uint) getExpectedBufferSizeVirtual;

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
- (NSRect) getDataTab: (int) mainThread : (int) stateTabsReader;

- (uint8_t) getIndexFromInput: (int) mainThread : (int) stateTabsReader;
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

//Structure trop différente pour qu'un héritage de RakPrefsTab soit rentable
@interface RakMDLSize : NSObject
{
	Prefs* mammouth;
	int sizeInputBuffer;
	
	//Prefs propres au MDL
	CGFloat widthMDLSerie;
	CGFloat heightMDLReaderFocus;
}

- (instancetype) init : (Prefs*) creator : (char*) inputData;
- (void) dumpData : (char *) output : (uint) length;

- (void) setExpectedBufferSize;
- (int) getExpectedBufferSize;
+ (uint) getExpectedBufferSizeVirtual;

- (uint8_t) getFlagFocus;
- (NSRect) getData:(int) mainThread : (int) stateTabsReader;
- (NSRect) getFocusSerie;
- (NSRect) getFocusCT;
- (NSRect) getFocusReader : (int) stateTabsReader;
- (NSRect) getDefaultFocusMDL;
- (CGFloat) getDefaultSerieWidth;
- (CGFloat) getDefaultFocusReaderHeight;

@end

