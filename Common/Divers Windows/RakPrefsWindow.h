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

@class RakPrefsWindow;

#import "RakPrefsTools.h"
#import "RakPrefsRepoView.h"
#import "RakPrefsGeneralView.h"
#import "RakPrefsFavoriteView.h"

enum
{
	PREF_WINDOW_HEIGHT = 400,
	PREF_WINDOW_WIDTH = 600,
	PREF_WINDOW_REAL_WIDTH = 596,
	
	PREF_BUTTON_BAR_HEIGHT = 65
};

@interface RakPrefsWindow : RakCustomWindow
{
	RakPrefsButtons * header;
	
	byte activeView;
	
	RakPrefsGeneralView * generalView;
	RakPrefsRepoView * repoView;
	RakView * favoriteView;
	RakView * customView;
}

- (RakColor *) textColor;

- (void) focusChanged : (byte) newTab;

@end
