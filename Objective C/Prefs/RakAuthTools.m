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
		
		[self setBackgroundColor:[Prefs getSystemColor:GET_COLOR_BACKGROUND_TEXTFIELD :nil]];
		[self setTextColor:[Prefs getSystemColor:GET_COLOR_CLICKABLE_TEXT :nil]];
		[self setFormatter:[[[RakFormatterLength alloc] init : 100] autorelease]];
		[self.cell setPlaceholderAttributedString : [[[NSAttributedString alloc] initWithString:@"exemple@email.com" attributes:@{NSForegroundColorAttributeName : [NSColor grayColor]}] autorelease]];
		
		[self setDelegate:self];
	}
	
	return self;
}

- (BOOL) becomeFirstResponder
{
	[self.cell setPlaceholderString:@"exemple@email.com"];
	return [super becomeFirstResponder];
}

- (void) addController : (RakAuthController *) controller
{
	authController = controller;
}

- (NSColor *) getBorderColor
{
	if(self.currentStatus != AUTH_STATE_INVALID && !checkNetworkState(CONNEXION_OK) && !checkNetworkState(CONNEXION_TEST_IN_PROGRESS))
	{
		self.currentStatus = AUTH_STATE_INVALID;
	}
	
	switch (self.currentStatus)
	{
		case AUTH_STATE_GOOD:
		{
			return [NSColor colorWithSRGBRed:0 green:1 blue:0 alpha:1.0];;
		}
			
		case AUTH_STATE_INVALID:
		{
			return [NSColor colorWithSRGBRed:1 green:0 blue:0 alpha:1.0];;
		}
			
		case AUTH_STATE_LOADING:
		{
			return [NSColor colorWithSRGBRed:1 green:0.5f blue:0 alpha:1.0];;
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
	
	NSString * string = [[[self stringValue] copy] autorelease];
	const char * data = [string cStringUsingEncoding:NSASCIIStringEncoding];
	
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
		
		[self setBackgroundColor:[Prefs getSystemColor:GET_COLOR_BACKGROUND_TEXTFIELD :nil]];
		[self setTextColor:[Prefs getSystemColor:GET_COLOR_CLICKABLE_TEXT :nil]];
	}
	
	return self;
}

- (NSColor *) getBorderColor
{
	switch (self.currentStatus)
	{
		case AUTH_STATE_GOOD:
		{
			return [NSColor colorWithSRGBRed:0 green:1 blue:0 alpha:1.0];;
		}
			
		case AUTH_STATE_INVALID:
		{
			return [NSColor colorWithSRGBRed:1 green:0 blue:0 alpha:1.0];;
		}
			
		case AUTH_STATE_LOADING:
		{
			return [NSColor colorWithSRGBRed:1 green:0.5f blue:0 alpha:1.0];;
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
	
	[self.controller focusLeft : self : flag];
}

@end