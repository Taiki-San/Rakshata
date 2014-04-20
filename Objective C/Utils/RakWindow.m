/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite  **
 **                                                                                         **
 *********************************************************************************************/

@implementation RakWindow

- (BOOL) canBecomeKeyWindow		{ return YES; }
- (BOOL) canBecomeMainWindow	{ return YES; }
- (BOOL) acceptsFirstResponder	{ return YES; }
- (BOOL) becomeFirstResponder	{ return YES; }
- (BOOL) resignFirstResponder	{ return YES; }

- (BOOL) isFullscreen
{
	return ([self styleMask] & NSFullScreenWindowMask) == NSFullScreenWindowMask;
}

- (void)sendEvent:(NSEvent *)theEvent
{
	if(![self isFullscreen])
	{
		if([theEvent type] == NSLeftMouseDown)
			[self mouseDown:theEvent];
		else if([theEvent type] == NSLeftMouseDragged && dragInProgress)
			[self mouseDragged:theEvent];
		else if([theEvent type] == NSLeftMouseUp)
			dragInProgress = false;
	}
	
	[super sendEvent:theEvent];
}

- (void) stopDrag
{
	dragInProgress = false;
}

-(void)mouseDown:(NSEvent *)theEvent
{
    NSRect  windowFrame = [self frame];
	
    initialLocation = [NSEvent mouseLocation];
	
    initialLocation.x -= windowFrame.origin.x;
    initialLocation.y -= windowFrame.origin.y;
	
	dragInProgress = true;
}

- (void)mouseDragged:(NSEvent *)theEvent
{
	if([self isFullscreen])
		return;
    
	NSPoint currentLocation;
    NSPoint newOrigin;
	
    NSRect  screenFrame = [[NSScreen mainScreen] frame];
    NSRect  windowFrame = [self frame];
	
    currentLocation = [NSEvent mouseLocation];
    newOrigin.x = currentLocation.x - initialLocation.x;
    newOrigin.y = currentLocation.y - initialLocation.y;
	
    // Don't let window get dragged up under the menu bar
    if( (newOrigin.y+windowFrame.size.height) > (screenFrame.origin.y+screenFrame.size.height) ){
        newOrigin.y=screenFrame.origin.y + (screenFrame.size.height-windowFrame.size.height);
    }
	
    //go ahead and move the window to the new location
    [self setFrameOrigin:newOrigin];
}

@end
