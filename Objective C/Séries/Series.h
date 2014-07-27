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

@class PrefsUI;

#import "RakSRRessources.h"

@interface Series : RakTabView
{
	RakButton *preferenceButton;
	RakBackButton * backButton;
	
	RakSerieView * coreView;
	
	PrefsUI * winController;
	BOOL prefsUIIsOpen;
}

- (id)init : (NSView*)contentView : (NSString *) state;

- (void) gogoWindow;

- (void) setupBackButton;
- (void) backButtonClicked;

- (NSRect) getCoreviewFrame : (CGFloat) backButtonY;

@end
