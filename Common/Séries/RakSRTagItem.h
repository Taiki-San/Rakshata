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

@class RakSRTagRail;

@interface RakSRTagItem : RakView
{
	RakText * label;
	RakButton * close;
}

@property byte _dataType;
@property uint64_t _dataID;

@property uint index;
@property (weak) RakSRTagRail * parent;

- (instancetype) initWithTag : (NSString *) tagName;
- (void) updateContent : (NSString *) newTagName;

@end

enum
{
	TAG_BUTTON_HEIGHT = 25,
	TAG_BUTTON_MIDDLE = 13,
	TAG_BUTTON_ARROW_DEPTH = 12,
	TAG_BORDER_LEFT_LABEL = 3,
	TAG_BORDER_RIGHT_LABEL = 5
};

