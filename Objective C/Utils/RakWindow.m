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

- (void)sendEvent:(NSEvent *)event
{
	if(![self isFullscreen])
	{
		if([event type] == NSLeftMouseDown)
			[self mouseDown:event];
		else if([event type] == NSLeftMouseDragged && dragInProgress)
			[self mouseDragged:event];
		else if([event type] == NSLeftMouseUp)
			dragInProgress = false;
	}
	
	if ([event type] == NSKeyDown)
	{
        if (([event modifierFlags] & NSDeviceIndependentModifierFlagsMask) == NSCommandKeyMask)
		{
			NSString * character = [event charactersIgnoringModifiers];

            if ([character isEqualToString:@"f"])
			    [self toggleFullScreen:self];
		}
	}
	
	[super sendEvent:event];
}

- (void) flagsChanged:(NSEvent *)theEvent
{
	uint flags = [theEvent modifierFlags];
	
	self.shiftPressed		= (flags & NSShiftKeyMask) != 0;
	self.optionPressed		= (flags & NSAlternateKeyMask) != 0;
	self.controlPressed		= (flags & NSControlKeyMask) != 0;
	self.functionPressed	= (flags & NSFunctionKeyMask) != 0;
	
	[super flagsChanged:theEvent];
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
	if( (newOrigin.y+windowFrame.size.height) > (screenFrame.origin.y+screenFrame.size.height) )
        newOrigin.y = screenFrame.origin.y + (screenFrame.size.height - windowFrame.size.height);
	
    //go ahead and move the window to the new location
    [self setFrameOrigin:newOrigin];
}

- (void)visualizeConstraints:(NSArray *)constraints
{
	NSLog(@"UI just blown up, sorry :/");
}

@end
