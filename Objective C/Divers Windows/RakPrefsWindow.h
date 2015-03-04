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

@class RakPrefsWindow;

#import "RakPrefsTools.h"
#import "RakPrefsRepoView.h"

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
	
	NSView * generalView;
	RakPrefsRepoView * repoView;
	NSView * favoriteView;
	NSView * customView;
}

- (NSColor *) textColor;

- (void) focusChanged : (byte) newTab;

@end
