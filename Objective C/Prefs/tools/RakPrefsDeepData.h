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

@class Prefs;

@interface RakPrefsDeepData : NSObject
{
	Prefs* mammouth;
	int numberElem;
	int sizeInputBuffer;
}

- (id) init : (Prefs*) creator : (char *) inputData;
- (void) initJumpTable : (SEL *) jumpTable;
- (void) setAtIndex: (uint8_t) index : (CGFloat) data;
- (void) reinitAtIndex : (uint8_t) index;

- (void) setNumberElem;
- (void) setExpectedBufferSize;

- (CGFloat) triggerJumpTable : (SEL) selector;

- (int) getNbElem;
- (void) performSelfCheck;

@end

#include "RakPrefsTabDeepData.h"