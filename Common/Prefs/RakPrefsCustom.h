/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at https://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

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
