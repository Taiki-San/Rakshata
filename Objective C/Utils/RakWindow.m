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

- (void) configure
{
	((RakContentViewBack *) self.contentView).isMainWindow = YES;
	((RakContentViewBack *) self.contentView).title = [self getProjectName];

	[self.contentView setupBorders];

	self.movable = YES;
	self.movableByWindowBackground = YES;
	self.titlebarAppearsTransparent = YES;
}

- (BOOL) canBecomeKeyWindow		{ return YES; }
- (BOOL) canBecomeMainWindow	{ return YES; }
- (BOOL) acceptsFirstResponder	{ return YES; }
- (BOOL) becomeFirstResponder	{ return YES; }
- (BOOL) resignFirstResponder	{ return YES; }

- (BOOL) isFullscreen
{
	return (self.styleMask & NSFullScreenWindowMask) == NSFullScreenWindowMask;
}

- (void) sendEvent:(NSEvent *)event
{
	//In some cases, the window would end up becoming the first responder, which is not an expected scenario
	if(self.firstResponder == self || [self.firstResponder class] == [RakContentView class])
	{
		uint thread;
		[Prefs getPref:PREFS_GET_MAIN_THREAD :&thread];
		
		if(thread == TAB_SERIES)
			[self makeFirstResponder:[[NSApp delegate] serie]];
		else if(thread == TAB_CT)
			[self makeFirstResponder:[[NSApp delegate] CT]];
		else if(thread == TAB_READER)
			[self makeFirstResponder:[[NSApp delegate] reader]];
	}
	
	if(!self.fullscreen)
	{
		if([event type] == NSLeftMouseDown)
			[self mouseDown:event];
	}
	
	if([event type] == NSKeyDown)
	{
		if(self.shiftPressed && self.commandPressed)
		{
			NSString * character = [event charactersIgnoringModifiers];
			
			if(_isMainWindow)
			{
				if([character isEqualToString:@"f"])
					[self toggleFullScreen:self];
			}
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

- (void) updateConstraintsIfNeeded
{
	
}

#pragma mark - Title management

- (NSString *) getProjectName
{
#ifdef DEV_VERSION
	return [NSString stringWithFormat:@"[Confidential α (#%lX)] %s", [[[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleVersion"] integerValue], PROJECT_NAME];
#else
	return [NSString stringWithUTF8String:PROJECT_NAME];
#endif
}

- (void) resetTitle
{
	((RakContentViewBack *) self.contentView).title = [self getProjectName];
}

- (void) setProjectTitle : (PROJECT_DATA) project
{
	((RakContentViewBack *) self.contentView).title = [NSString stringWithFormat:@"%@ - %@", [self getProjectName], getStringForWchar(project.projectName)];
}

- (void) setCTTitle : (PROJECT_DATA) project : (NSString *) element
{
	((RakContentViewBack *) self.contentView).title = [NSString stringWithFormat:@"%@ - %@ - %@", [self getProjectName], getStringForWchar(project.projectName), element];
}

#pragma mark - Delegate

- (BOOL) makeFirstResponder:(NSResponder *)aResponder
{
	id old = _imatureFirstResponder;
	
	_imatureFirstResponder = aResponder;
	BOOL retValue = [super makeFirstResponder:aResponder];
	_imatureFirstResponder = old;
	
	return retValue;
}

@end
