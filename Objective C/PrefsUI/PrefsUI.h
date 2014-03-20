/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                          **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite   **
 **                                                                                          **
 *********************************************************************************************/

#import <Cocoa/Cocoa.h>
#import "utils.h"

//Allow to create NSViewController without NIB
@interface RakPrefsWindow : NSViewController
{
	NSView * mainView;
}

- (id)initWithFrame:(NSRect)frame;

@end

@interface PrefsUI : NSView <NSPopoverDelegate>
{
	NSPanel * HUDWindow;
    RakPrefsWindow *viewControllerHUD;
    NSPopover *popover;

	NSButton *anchor;    
}
- (void)showPopover;
- (void) setAnchor : (NSButton *) newAnchor;

@end
