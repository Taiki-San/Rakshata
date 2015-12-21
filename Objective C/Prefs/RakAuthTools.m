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

#define DEFAULT_EMAIL @"exemple@email.com"

@implementation RakEmailField

- (instancetype) initWithFrame : (NSRect) frameRect
{
	self = [super initWithFrame:frameRect];
	
	if(self != nil)
	{
		self.currentStatus = AUTH_STATE_NONE;
		
		((RakTextCell*)self.cell).customizedInjectionPoint = YES;
		((RakTextCell*)self.cell).centered = YES;
		
		[self setBezeled:NO];
		[self setBordered:YES];
		self.wantCustomBorder = YES;
		
		[self setBackgroundColor:[Prefs getSystemColor:COLOR_TEXTFIELD_BACKGROUND]];
		[self setTextColor:[Prefs getSystemColor:COLOR_CLICKABLE_TEXT]];
		self.maxLength = 100;
		[self.cell setPlaceholderAttributedString : [[NSAttributedString alloc] initWithString:DEFAULT_EMAIL attributes:@{NSForegroundColorAttributeName : [RakColor grayColor]}]];
		
		[self setDelegate:self];
	}
	
	return self;
}

- (BOOL) becomeFirstResponder
{
	[self.cell setPlaceholderString:DEFAULT_EMAIL];
	return [super becomeFirstResponder];
}

- (void) addController : (RakAuthController *) controller
{
	authController = controller;
}

- (RakColor *) getBorderColor
{
	if(self.currentStatus != AUTH_STATE_INVALID && !checkNetworkState(CONNEXION_OK) && !checkNetworkState(CONNEXION_TEST_IN_PROGRESS))
	{
		self.currentStatus = AUTH_STATE_INVALID;
	}
	
	switch (self.currentStatus)
	{
		case AUTH_STATE_GOOD:
		{
			return [Prefs getSystemColor:COLOR_BUTTON_STATUS_OK];
		}
			
		case AUTH_STATE_INVALID:
		{
			return [Prefs getSystemColor:COLOR_BUTTON_STATUS_ERROR];
		}
			
		case AUTH_STATE_LOADING:
		{
			return [Prefs getSystemColor:COLOR_BUTTON_STATUS_WARN];
		}
			
		case AUTH_STATE_NONE:
		default:
		{
			return [super getBorderColor];
		}
	}
}

#pragma mark - Delegate

- (BOOL) textShouldBeginEditing:(NSText *)textObject
{
	return authController == NULL || !authController.postProcessing;
}

- (void)textDidBeginEditing:(NSNotification *)aNotification
{
	currentEditingSession++;
	self.currentStatus = AUTH_STATE_NONE;
}

- (void)textDidEndEditing:(NSNotification *)aNotification
{
	uint currentSession = currentEditingSession;
	
	self.currentStatus = AUTH_STATE_LOADING;
	[self setNeedsDisplay];
	
	[self performSelectorInBackground:@selector(checkEmail:) withObject:@(currentSession)];
	
	NSDictionary * dict = [aNotification userInfo];
	NSNumber * data;
	
	if(dict != nil && (data = [dict objectForKey:@"NSTextMovement"]) != nil)
		[authController focusLeft : self : [data unsignedIntegerValue]];
}

#pragma mark - Core

- (void) checkEmail : (NSNumber *) session
{
	if(session != nil && [session isKindOfClass:[NSNumber class]])
	{
		uint currentSession = [session unsignedIntValue];
		
		[CATransaction begin];
		
		[self checkEmailSub : currentSession];
		
		if(self.currentStatus != AUTH_STATE_GOOD)
			[CATransaction commit];
	}
	else
		self.currentStatus = AUTH_STATE_INVALID;
	
	[self performSelectorOnMainThread:@selector(setNeedsDisplay) withObject:nil waitUntilDone:NO];
}

- (void) checkEmailSub : (uint) currentSession
{
	if(checkNetworkState(CONNEXION_TEST_IN_PROGRESS))
	{
		while(currentSession == currentEditingSession && checkNetworkState(CONNEXION_TEST_IN_PROGRESS))
		{
			usleep(5000);
		}
	}
	
	if(currentSession == currentEditingSession && !checkNetworkState(CONNEXION_OK))
	{
		self.currentStatus = AUTH_STATE_INVALID;
		return;
	}
	
	if([self.stringValue length] > 100)
	{
		self.currentStatus = AUTH_STATE_INVALID;
		return;
	}
	
	NSString * string = [self.stringValue copy];
	const char * data = [string UTF8String];
	
	byte retValue = checkLogin(data);
	
	if(currentSession != currentEditingSession)
		return;
	
	if(retValue == 2)
	{
		self.currentStatus = AUTH_STATE_INVALID;
		return;
	}
	
	self.currentStatus = AUTH_STATE_GOOD;
	
	if(authController != nil)
	{
		[authController wakePassUp];
		[CATransaction commit];
		[authController validEmail : retValue == 0 : currentSession];
	}
}

@end

@implementation RakPassField

- (instancetype) initWithFrame : (NSRect) frameRect
{
	self = [super initWithFrame:frameRect];
	
	if(self != nil)
	{
		[self setBezeled:NO];
		[self setBordered:YES];
		self.wantCustomBorder = NO;
		[self setDelegate:self];
		
		[self setBackgroundColor:[Prefs getSystemColor:COLOR_TEXTFIELD_BACKGROUND]];
		[self setTextColor:[Prefs getSystemColor:COLOR_CLICKABLE_TEXT]];
	}
	
	return self;
}

- (RakColor *) getBorderColor
{
	switch (self.currentStatus)
	{
		case AUTH_STATE_GOOD:
		{
			return [Prefs getSystemColor:COLOR_BUTTON_STATUS_OK];
		}
			
		case AUTH_STATE_INVALID:
		{
			return [Prefs getSystemColor:COLOR_BUTTON_STATUS_ERROR];
		}
			
		case AUTH_STATE_LOADING:
		{
			return [Prefs getSystemColor:COLOR_BUTTON_STATUS_WARN];
		}
			
		case AUTH_STATE_NONE:
		default:
		{
			return [super getBorderColor];
		}
	}
}

- (void) addController : (RakAuthController *) controller
{
	authController = controller;
}

- (BOOL) textShouldBeginEditing:(NSText *)textObject
{
	return authController == nil || !authController.postProcessing;
}

- (void)textDidEndEditing:(NSNotification *)aNotification
{
	self.currentStatus = AUTH_STATE_LOADING;
	[self setNeedsDisplay];
	
	NSDictionary * dict = [aNotification userInfo];
	NSNumber * data;
	
	if(dict != nil && (data = [dict objectForKey:@"NSTextMovement"]) != nil)
		[authController focusLeft : self : [data unsignedIntegerValue]];
}

+ (Class) cellClass
{
	return [RakPassFieldCell class];
}

@end

#define RADIUS 2

@implementation RakAuthText

- (instancetype) initWithText : (NSRect)frame : (NSString *) text : (RakColor *) color;
{
	self = [self initWithFrame:frame];
	
	if(self != nil)
	{
		RakText * content = [[RakText alloc] initWithText : frame : text : color];
		[content sizeToFit];
		
		[self setFrameSize:NSMakeSize(content.bounds.size.width + 2 * RADIUS, content.bounds.size.height + 2 * RADIUS)];
		[content setFrameOrigin:NSMakePoint(RADIUS, RADIUS)];
		
		[self addSubview:content];
	}
	
	return self;
}

- (void) setFrame:(NSRect)frameRect
{
	[super setFrame:frameRect];
	[self.subviews[0] setFrame:NSInsetRect(frameRect, RADIUS, RADIUS)];
}

- (void) drawRect:(NSRect)dirtyRect
{
	if(backgroundColor == nil)
	{
		[Prefs registerForChange:self forType:KVO_THEME];
		backgroundColor = [Prefs getSystemColor:COLOR_BUTTON_BACKGROUND_UNSELECTED];
	}
	
	[backgroundColor setFill];
	[[NSBezierPath bezierPathWithRoundedRect:self.bounds xRadius:RADIUS yRadius:RADIUS] fill];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];

	backgroundColor = [Prefs getSystemColor:COLOR_BUTTON_BACKGROUND_UNSELECTED];
	[self setNeedsDisplay:YES];
}

- (void) mouseUp:(NSEvent *)theEvent
{
	if(self.URL != nil)
		[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:self.URL]];
}

@end

@implementation RakAuthTermsButton

- (void) keyDown:(NSEvent *)theEvent
{
	unsigned short key = [theEvent keyCode];
	NSUInteger flag;
	
	if(key == 48)		//Tab
	{
		if([theEvent modifierFlags] & NSShiftKeyMask)	//Shift is also pressed
			flag = NSBacktabTextMovement;
		else
			flag = NSTabTextMovement;
	}
	else if(key == 36)	//Return
		flag = NSReturnTextMovement;
	
	else if(key == 49)	//Space bar
	{
		[self performClick:self];
		//self.state = (self.state == NSOnState) ? NSOffState : NSOnState;
		return;
	}
	
	else
		return;
	
	if(self.controller != nil && [self.controller respondsToSelector : @selector(focusLeft::)])
	{
		[self.controller focusLeft : self : flag];
	}
}

@end