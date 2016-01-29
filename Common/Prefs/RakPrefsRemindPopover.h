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

//Don't forget to reflect any addition to +haveAnyRemindedValue
#define PREFS_REMIND_DELETE			@"DefaultPopoverDelete"
#define PREFS_REMIND_AUTODL			@"DefaultPopoverAutoDL"
#define PREFS_REMIND_FAVS			@"PopoverFavoriteDisplayed"
#define PREFS_REMIND_SUGGESTION		@"GetThoseDamnSuggestionsOutOfTheWay"

@interface RakPrefsRemindPopover : NSObject

+ (void) setValueReminded : (NSString *) element : (BOOL) value;
+ (BOOL) getValueReminded : (NSString *) element : (BOOL *) value;
+ (void) removeValueReminded : (NSString *) element;

+ (BOOL) haveAnyRemindedValue;
+ (void) flushRemindedValues;

@end
