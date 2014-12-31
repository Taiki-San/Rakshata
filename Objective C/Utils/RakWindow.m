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
	if(!self.fullscreen)
	{
		if([event type] == NSLeftMouseDown)
			[self mouseDown:event];
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
	self.commandPressed		= (flags & NSCommandKeyMask) != 0;
	
	[super flagsChanged:theEvent];
}

- (void) keyDown:(NSEvent *)theEvent
{
	[self.contentView keyDown:theEvent];
}

- (void) layoutIfNeeded
{
	
}

#pragma mark - Delegate

- (void) windowWillExitFullScreen:(NSNotification *)notification
{
	[[(RakAppDelegate*) [NSApp delegate]reader] shouldLeaveDistractionFreeMode];
}

@end
