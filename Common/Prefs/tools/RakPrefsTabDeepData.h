/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
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
- (uint) getExpectedBufferSize;
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
- (NSRect) getDataTab: (uint) mainThread : (uint) stateTabsReader;

- (uint8_t) getIndexFromInput: (uint) mainThread : (uint) stateTabsReader;
- (CGFloat) getFooterHeight;
- (void) setFooterHeight : (CGFloat) data;


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
	uint sizeInputBuffer;
	
	//Prefs propres au MDL
	CGFloat widthMDLSerie;
	CGFloat heightMDLReaderFocus;
}

- (instancetype) init : (Prefs*) creator : (char*) inputData;
- (void) dumpData : (char *) output : (uint) length;

- (void) setExpectedBufferSize;
- (uint) getExpectedBufferSize;
+ (uint) getExpectedBufferSizeVirtual;

- (uint8_t) getFlagFocus;
- (NSRect) getData:(uint) mainThread : (uint) stateTabsReader;
- (NSRect) getFocusSerie;
- (NSRect) getFocusCT;
- (NSRect) getFocusReader : (uint) stateTabsReader;
- (NSRect) getDefaultFocusMDL;
- (CGFloat) getDefaultSerieWidth;
- (CGFloat) getDefaultFocusReaderHeight;

@end

