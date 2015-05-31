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

#ifndef NSAppKitVersionNumber10_10
#define NSAppKitVersionNumber10_10 1343
#endif

@implementation RakWindow

- (void) configure
{
	[self.contentView setupBorders];
	[self setMovable:YES];
	self.movableByWindowBackground = YES;
	
	self.title = [self getProjectName];
	self.showsTitle = YES;
	self.verticallyCenterTitle = YES;
	self.centerTrafficLightButtons = YES;
	self.showsBaselineSeparator = NO;
	self.hideTitleBarInFullScreen = YES;
	
	self.titleTextShadow = self.inactiveTitleTextShadow = [[NSShadow alloc] init];
	
	self.titleTextColor = [Prefs getSystemColor:GET_COLOR_SURVOL : self];
	self.inactiveTitleTextColor = [Prefs getSystemColor:GET_COLOR_ACTIVE : nil];
	
	self.titleBarDrawingBlock = ^(BOOL drawsAsMainWindow, CGRect drawingRect, CGRectEdge edge, CGPathRef clippingPath)
	{
		CGContextRef ctx = [[NSGraphicsContext currentContext] graphicsPort];
		CGContextSaveGState(ctx);
		if (clippingPath)
		{
			CGContextAddPath(ctx, clippingPath);
			CGContextClip(ctx);
		}
		
		if (drawsAsMainWindow || !self.isMainWindow)
		{
			if((NSInteger)NSAppKitVersionNumber < NSAppKitVersionNumber10_10)
			{
				NSGradient *gradient = [[NSGradient alloc] initWithStartingColor:[[Prefs getSystemColor:GET_COLOR_TITLEBAR_BACKGROUND_GRADIENT_START :nil] colorWithAlphaComponent:0.7]
																	 endingColor:[Prefs getSystemColor:GET_COLOR_TITLEBAR_BACKGROUND_GRADIENT_END :nil]];
				[gradient drawInRect:drawingRect angle:90];
			}
			else
			{
				[[Prefs getSystemColor:GET_COLOR_TITLEBAR_BACKGROUND_MAIN :nil] setFill];
				NSRectFill(drawingRect);
			}
		}
		else
		{
			[[Prefs getSystemColor:GET_COLOR_TITLEBAR_BACKGROUND_STANDBY :nil] setFill];
			NSRectFill(drawingRect);
		}
	};
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
	
	if ([event type] == NSKeyDown)
	{
		if (([event modifierFlags] & NSDeviceIndependentModifierFlagsMask) == NSCommandKeyMask)
		{
			NSString * character = [event charactersIgnoringModifiers];
			
			if(_isMainWindow)
			{
				if ([character isEqualToString:@"f"])
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
	return [NSString stringWithFormat:@"[Build #%lX] %s", [[[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleVersion"] integerValue], PROJECT_NAME];
#else
	return [NSString stringWithUTF8String:PROJECT_NAME];
#endif
}

- (void) resetTitle
{
	self.title = [self getProjectName];
}

- (void) setProjectTitle : (PROJECT_DATA) project
{
	self.title = [NSString stringWithFormat:@"%@ - %@", [self getProjectName], getStringForWchar(project.projectName)];
}

- (void) setCTTitle : (PROJECT_DATA) project : (NSString *) element
{
	self.title = [NSString stringWithFormat:@"%@ - %@ - %@", [self getProjectName], getStringForWchar(project.projectName), element];
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

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return;
	
	self.titleTextColor = [Prefs getSystemColor:GET_COLOR_ACTIVE : nil];
	self.inactiveTitleTextColor = [Prefs getSystemColor:GET_COLOR_ACTIVE : nil];
}

@end
