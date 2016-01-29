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

@class Prefs;

@interface RakPrefsDeepData : NSObject
{
	Prefs* mammouth;
	uint numberElem;
	uint sizeInputBuffer;
}

- (instancetype) init : (Prefs*) creator : (char *) inputData;
- (void) initJumpTable : (SEL *) jumpTable;
- (void) setAtIndex: (uint8_t) index : (CGFloat) data;
- (void) reinitAtIndex : (uint8_t) index;

- (void) setNumberElem;
- (void) setExpectedBufferSize;

- (CGFloat) triggerJumpTable : (SEL) selector;

- (uint) getNbElem;

@end

#include "RakPrefsTabDeepData.h"