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
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

#define SR_NOTIF_NEW_STATE @"newState"

#define SRSEARCHTAB_DEFAULT_HEIGHT 1
#define SR_SEARCH_TAB_INITIAL_HEIGHT 40
#define SR_SEARCH_TAB_EXPANDED_HEIGHT 150

#import "RakSRSearchList.h"
#import "RakSRSearchTabGroup.h"

@interface RakSRSearchTab : RakView
{
	BOOL _isVisible;
	BOOL _collapsed;
		
	RakText * placeholder;
	
	RakSRSearchTabGroup * author, *source, *tag, * type, *extra;
}

@property CGFloat height;

- (void) resizeAnimation : (NSRect) frameRect;

@end

byte getRestrictionTypeForSBID(byte searchBarID);
NSString * getNotificationNameForSBID(byte searchBarID);