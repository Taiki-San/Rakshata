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

#define TITLE_BAR_HEIGHT 22

@interface RakWindow : NSWindow <NSDraggingDestination>
{
	BOOL _sheetManipulation;
}

@property (getter=isMainWindow) BOOL isMainWindow;

//Event data
@property BOOL shiftPressed;
@property BOOL optionPressed;
@property BOOL controlPressed;
@property BOOL functionPressed;
@property BOOL commandPressed;

@property (readonly, getter=isFullscreen) BOOL fullscreen;
@property (weak) NSView* defaultDispatcher;
@property (strong) NSResponder* imatureFirstResponder;

- (void) configure;
- (void) registerForDrop;

- (void) resetTitle;
- (void) setProjectTitle : (PROJECT_DATA) project;
- (void) setCTTitle : (PROJECT_DATA) project : (NSString *) element;

@end
