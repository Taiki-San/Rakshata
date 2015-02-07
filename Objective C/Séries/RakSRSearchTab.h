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

#define SR_NOTIF_NAME_SEARCH_TRIGGERED @"RakSearchFieldWasTriggered"
#define SR_NOTIF_NEW_STATE @"newState"

#define SRSEARCHTAB_DEFAULT_HEIGHT 1
#define SR_SEARCH_TAB_INITIAL_HEIGHT 40
#define SR_SEARCH_TAB_EXPANDED_HEIGHT 150

#import "RakSRSearchList.h"
#import "RakSRSearchTabGroup.h"

@interface RakSRSearchTab : NSView
{
	BOOL _isVisible;
	BOOL _collapsed;
		
	RakText * placeholder;
	
	RakSRSearchTabGroup * author, *source, *tag, * type, *extra;
}

@property CGFloat height;

- (void) resizeAnimation : (NSRect) frameRect;

@end
