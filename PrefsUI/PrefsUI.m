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

@implementation PrefsUI

- (id) init
{
	self = [super init];
	if(self)
	{
		HUDWindow = [[NSPanel alloc] initWithContentRect:NSMakeRect(50, 100, 150, 200)
								   styleMask:NSHUDWindowMask backing:NSBackingStoreBuffered defer:NO];
		
		viewControllerHUD = [[MainViewController alloc] initWithFrame:self.window.frame];
		[self setAnchor:nil];
	}
	return self;
}

- (void) setAnchor : (NSButton *) newAnchor
{
	anchor = newAnchor;
}

// -------------------------------------------------------------------------------
//  createPopover
// -------------------------------------------------------------------------------
- (void)createPopover
{
    if (popover == nil)
    {
        // create and setup our popover
        popover = [[NSPopover alloc] init];
        
		popover.contentViewController = viewControllerHUD;
        popover.appearance = 1;
		
		//Get if animation have to be set from Prefs /  animates = YES
        popover.animates = YES;
        
        // AppKit will close the popover when the user interacts with a user interface element outside the popover.
        // note that interacting with menus or panels that become key only when needed will not cause a transient popover to close.
        popover.behavior = NSPopoverBehaviorTransient;
        
        // so we can be notified when the popover appears or closes
        popover.delegate = self;
    }
}



// -------------------------------------------------------------------------------
//  dealloc
// -------------------------------------------------------------------------------
- (void)dealloc
{
	if(anchor == nil)
		[anchor release];
    [popover release];
    [super dealloc];
}

// -------------------------------------------------------------------------------
//  showPopoverAction:sender
// -------------------------------------------------------------------------------
- (void)showPopover
{
    [self createPopover];
	[popover showRelativeToRect:[anchor bounds]
						   ofView:anchor
					preferredEdge:NSMaxYEdge];
}

// -------------------------------------------------------------------------------
//  applicationShouldTerminateAfterLastWindowClosed:sender
//
//  NSApplication delegate method placed here so the sample conveniently quits
//  after we close the window.
// -------------------------------------------------------------------------------
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
    return YES;
}


#pragma mark -
#pragma mark NSPopoverDelegate

// -------------------------------------------------------------------------------
// Invoked on the delegate when the NSPopoverWillShowNotification notification is sent.
// This method will also be invoked on the popover. 
// -------------------------------------------------------------------------------
- (void)popoverWillShow:(NSNotification *)notification
{
    NSPopover *localPopover = [notification object];
    if (localPopover.appearance == NSPopoverAppearanceHUD)
    {
        //viewControllerHUD is loaded by now, so set its UI up
    }
}

// -------------------------------------------------------------------------------
// Invoked on the delegate when the NSPopoverDidShowNotification notification is sent.
// This method will also be invoked on the popover. 
// -------------------------------------------------------------------------------
- (void)popoverDidShow:(NSNotification *)notification
{
    // add new code here after the popover has been shown
}

// -------------------------------------------------------------------------------
// Invoked on the delegate when the NSPopoverWillCloseNotification notification is sent.
// This method will also be invoked on the popover. 
// -------------------------------------------------------------------------------
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

// -------------------------------------------------------------------------------
// Invoked on the delegate when the NSPopoverDidCloseNotification notification is sent.
// This method will also be invoked on the popover. 
// -------------------------------------------------------------------------------
- (void)popoverDidClose:(NSNotification *)notification
{
    NSString *closeReason = [[notification userInfo] valueForKey:NSPopoverCloseReasonKey];
    if (closeReason)
    {
        // closeReason can be:
        //      NSPopoverCloseReasonStandard
        //      NSPopoverCloseReasonDetachToWindow
        //
        // add new code here if you want to respond "after" the popover closes
        //
    }
    
    [popover release];
    popover = nil;
}

// -------------------------------------------------------------------------------
// Invoked on the delegate asked for the detachable window for the popover.
// -------------------------------------------------------------------------------
- (NSWindow *)detachableWindowForPopover:(NSPopover *)popover
{
	return HUDWindow;
}

@end