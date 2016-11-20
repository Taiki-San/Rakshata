//
//  RakPrefsCustom.h
//  Rakshata
//
//  Created by Taiki on 20/11/2016.
//  Copyright © 2016 Taiki. All rights reserved.
//

#include "RakPrefsTools.h"
#include "RakPrefsDeepData.h"
#import "RakContextRestoration.h"

@interface RakPrefsCustom : NSObject <RakPrefsCustomized>
{
	Prefs* _proxy;
	RakContentView* firstResponder;

	// Prefs taille/pos elements (pourcentages)
	RakSizeSeries	*	tabSerieSize;
	RakSizeCT		*	tabCTSize;
	RakSizeReader	*	tabReaderSize;
	
	RakMDLSize * prefsPosMDL;

	NSArray * _darkColor, * _lightColor, * _customColor;
}

@property uint mainThread;
@property uint stateTabsReader;
@property byte activePrefsPanel;
@property BOOL saveMagnification;
@property BOOL havePDFBackground;
@property BOOL overrideDirection;
@property BOOL favoriteAutoDL;
@property BOOL suggestFromLastRead;

- (void) updateContext : (NSString *) context withProxy : (Prefs *) proxy;
- (void) refreshFirstResponder;

- (NSArray <RakColor *> *) darkColorArray;
- (NSArray <RakColor *> *) lightColorArray;
- (NSArray <RakColor *> *) customColorArray;

@end

#include "prefsControl.h"
#include "prefsMagic.h"

#if !TARGET_OS_IPHONE
	#include "PrefsUI.h"
	#include "RakPrefsRemindPopover.h"
#endif
