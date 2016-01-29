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

@interface RakSRSearchTabGroup : RakView
{
	byte _ID;
	
	RakSRSearchList * list;
	RakSRSearchBar * searchBar;
	
	//Used by extra
	RakButton * close, *flush;
	RakSwitchButton * freeSwitch, * favsSwitch;
	RakText * freeText, * favsText;
	RakView * buttonContainer;
	
	charType ** listData, **_cachedListData;
	uint nbDataList, _cachedNbDataList;
	uint64_t * indexesData, * _cachedIndexesData, sessionID;
	
	BOOL manualUpdateNoNotif;
}

- (instancetype) initWithFrame:(NSRect)frameRect : (byte) ID;
- (void) resizeAnimation:(NSRect)frameRect;

- (RakSRSearchBar *) searchBar;

@end
