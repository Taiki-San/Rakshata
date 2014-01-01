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

#import "PrefsUI.h"

@interface Series : NSViewCustom
{
	NSView* tabSerie;
	
	NSButton *button;
	
	PrefsUI * winController;
}
- (id)init:(NSWindow*)window;
- (void) craftPrefButton : (NSRect) tabSerieFrame;

- (void) resizeView:(NSRect)frame;
- (void) drawView;

- (void) gogoWindow;

@end

#import "Footer Static/FooterSStatic.h"
#import "Footer Drag/FooterSDrag.h"
