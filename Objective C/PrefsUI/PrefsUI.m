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

@implementation RakPrefsWindow

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithNibName:nil bundle:nil];
    if (self)
	{
		mainView = [[NSView alloc] initWithFrame:frame];
		[self setView:mainView];
		
		RakButton * button = [[RakButton allocWithText:@"Change UI" :frame] retain];
		[button sizeToFit];
		[button setTarget:self];
		[button setAction:@selector(updateUITheme)];
		[button setFrameOrigin:NSMakePoint(frame.size.width / 2 - button.frame.size.width / 2, frame.size.height / 2 - button.frame.size.height / 2)];
		[mainView addSubview:button];
	}
    return self;
}

- (void) updateUITheme
{
	[Prefs setCurrentTheme:[Prefs getCurrentTheme:nil] == THEME_CODE_DARK ? THEME_CODE_LIGHT : THEME_CODE_DARK];
}

@end

@implementation PrefsUI

- (id) init
{
	self = [super init];
	if(self)
	{
		viewControllerHUD = [[RakPrefsWindow alloc] initWithFrame:NSMakeRect(0, 0, 150, 150)];
		[self setAnchor:nil];
	}
	return self;
}

- (void) setAnchor : (NSButton *) newAnchor
{
	anchor = newAnchor;
}

- (void)createPopover
{
    if (popover == nil)
    {
        popover = [[NSPopover alloc] init];
        
		popover.contentViewController = viewControllerHUD;
        popover.appearance = 1;
		
        popover.animates = YES;
        
        // AppKit will close the popover when the user interacts with a user interface element outside the popover.
        // note that interacting with menus or panels that become key only when needed will not cause a transient popover to close.
        popover.behavior = NSPopoverBehaviorTransient;
        
        // so we can be notified when the popover appears or closes
        popover.delegate = self;
    }
}

- (void)dealloc
{
	if(anchor == nil)
		[anchor release];
    [popover release];
    [super dealloc];
}

- (void)showPopover
{
    [self createPopover];
	[popover showRelativeToRect:[anchor bounds]
						   ofView:anchor
					preferredEdge:NSMaxYEdge];
}

#pragma mark - NSPopoverDelegate

- (void)popoverWillShow:(NSNotification *)notification
{
	//viewControllerHUD is loaded by now, so set its UI up
}

- (void)popoverDidShow:(NSNotification *)notification
{
    // add new code here after the popover has been shown
}

- (void)popoverWillClose:(NSNotification *)notification
{
    NSString *closeReason = [[notification userInfo] valueForKey:NSPopoverCloseReasonKey];
    if (closeReason)
    {
        // closeReason can be:
        //      NSPopoverCloseReasonStandard
        //      NSPopoverCloseReasonDetachToWindow
        //
        // add new code here if you want to respond "before" the popover closes
        //
    }
}

- (void)popoverDidClose:(NSNotification *)notification
{
	//Ajouter des codes à la fermeture

    [popover release];
    popover = nil;
}

@end