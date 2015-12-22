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

//Allow to create NSViewController without NIB
@interface RakPrefsPopover : NSViewController
{
	RakView * mainView;
	
	uint mainThread;
	
	NSMutableArray * textFields;
	
	//UI Elements
	//	Reader
	
	RakSwitchButton * forcePDFBackground, * saveMagnification, *overrideDirection;
}

@property (nonatomic, weak) NSPopover * popover;

- (instancetype) initWithFrame : (NSRect) frame;
- (void) popoverClosed;

@end

@interface PrefsUI : RakView <NSPopoverDelegate>
{
    RakPrefsPopover *viewControllerHUD;
    NSPopover *popover;

	NSButton *anchor;
}

- (void) showPopover;
- (void) setAnchor : (NSButton *) newAnchor;

@end
