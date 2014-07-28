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

@implementation RakAuthController

- (void) launch
{
	RakAppDelegate * core = [NSApp delegate];
	
	[self retain];
	
	if([core class] != [RakAppDelegate class])
	{
		[self release];
		return;
	}
	
	self.view.wantsLayer = YES;
	self.view.layer.backgroundColor = [Prefs getSystemColor:GET_COLOR_BACKGROUND_TABS :self].CGColor;
	self.view.layer.cornerRadius = 4;
	
	[self updateMainView];
	
	foreground = [[RakForegroundView alloc] init : [core getContentView] : self.view];
	foreground.delegate = self;
	
	RakButton * inactiveConfirm = [RakButton allocWithText:@"Valider" :NSZeroRect];
	[inactiveConfirm sizeToFit];
	[inactiveConfirm setEnabled:NO];
	
	[inactiveConfirm setFrameOrigin: NSMakePoint(container.frame.size.width / 2 - inactiveConfirm.frame.size.width / 2, 15)];
	[container addSubview:inactiveConfirm];
	
	[foreground switchState];
}

- (void) updateMainView
{
	[header setTextColor:[Prefs getSystemColor:GET_COLOR_SURVOL :nil]];
	[header setFont:[NSFont boldSystemFontOfSize:13]];
	[header sizeToFit];
	[header setFrameOrigin:NSMakePoint(self.view.frame.size.width / 2 - header.frame.size.width / 2, header.frame.origin.y)];
	
	[headerDetails setStringValue:@"String placeholder for the time being..."];
	[headerDetails setTextColor:[Prefs getSystemColor:GET_COLOR_SURVOL :nil]];
	[headerDetails sizeToFit];
	[headerDetails setFrameOrigin:NSMakePoint(self.view.frame.size.width / 2 - headerDetails.frame.size.width / 2, headerDetails.frame.origin.y)];
	
	[labelMail setTextColor:[Prefs getSystemColor:GET_COLOR_SURVOL :nil]];
	[labelPass setTextColor:[Prefs getSystemColor:GET_COLOR_SURVOL :nil]];
	
	[mailInput setBackgroundColor:[Prefs getSystemColor:GET_COLOR_BACKGROUND_TEXTFIELD :nil]];
	[mailInput setTextColor:[Prefs getSystemColor:GET_COLOR_CLICKABLE_TEXT :nil]];
	[mailInput setFormatter:[[[RakFormatterLength alloc] init : 100] autorelease]];
	[mailInput addController:self];

	[passInput setBackgroundColor:[Prefs getSystemColor:GET_COLOR_BACKGROUND_TEXTFIELD :nil]];
	[passInput setTextColor:[Prefs getSystemColor:GET_COLOR_CLICKABLE_TEXT :nil]];
}

- (void) validEmail : (BOOL) newAccount : (uint) session
{
	
}

#pragma mark - Delegate

- (void) switchOver : (NSNumber*) isDisplayed
{
	if(isDisplayed != nil && [isDisplayed isKindOfClass:[NSNumber class]] && ![isDisplayed boolValue])
	{
		[header removeFromSuperview];			[header release];
		[headerDetails removeFromSuperview];	[headerDetails release];
		
		
		[foreground release];
		[self release];
	}
}

@end

@implementation RakEmailField

- (instancetype) initWithCoder:(NSCoder *)aDecoder
{
	self = [super initWithCoder:aDecoder];
	
	if(self != nil)
	{
		currentStatus = AUTHEMAIL_STATE_NONE;
		
		((RakTextCell*)self.cell).customizedInjectionPoint = YES;
		((RakTextCell*)self.cell).centered = YES;
		self.wantCustomBorder = YES;
		
		[self setDelegate:self];
	}
	
	return self;
}

- (void) addController : (RakAuthController *) controller
{
	authController = controller;
}

- (NSColor *) getBorderColor
{
	if(currentStatus != AUTHEMAIL_STATE_INVALID && !checkNetworkState(CONNEXION_OK) && !checkNetworkState(CONNEXION_TEST_IN_PROGRESS))
	{
		currentStatus = AUTHEMAIL_STATE_INVALID;
	}
	
	switch (currentStatus)
	{
		case AUTHEMAIL_STATE_GOOD:
		{
			return [NSColor colorWithSRGBRed:0 green:1 blue:0 alpha:1.0];;
		}
			
		case AUTHEMAIL_STATE_INVALID:
		{
			return [NSColor colorWithSRGBRed:1 green:0 blue:0 alpha:1.0];;
		}
			
		case AUTHEMAIL_STATE_LOADING:
		{
			return [NSColor colorWithSRGBRed:1 green:0.5f blue:0 alpha:1.0];;
		}
			
		case AUTHEMAIL_STATE_NONE:
		default:
		{
			return [super getBorderColor];
		}
	}
}

#pragma mark - Delegate

- (void)textDidBeginEditing:(NSNotification *)aNotification
{
	self.currentEditingSession++;
	currentStatus = AUTHEMAIL_STATE_NONE;
}

- (void)textDidEndEditing:(NSNotification *)aNotification
{
	uint currentSession = self.currentEditingSession;
	
	currentStatus = AUTHEMAIL_STATE_LOADING;
	[self setNeedsDisplay];
	
	[self performSelectorInBackground:@selector(checkEmail:) withObject:@(currentSession)];
}

- (void) checkEmail : (NSNumber *) session
{
	if(session == nil || ![session isKindOfClass:[NSNumber class]])
	{
		currentStatus = AUTHEMAIL_STATE_INVALID;
		[self performSelectorOnMainThread:@selector(setNeedsDisplay) withObject:nil waitUntilDone:NO];
		return;
	}

	uint currentSession = [session unsignedIntValue];
	
	[CATransaction begin];
	[CATransaction setDisableActions:YES];
	
	[self checkEmailSub : currentSession];
	
	[CATransaction commit];
	[self performSelectorOnMainThread:@selector(setNeedsDisplay) withObject:nil waitUntilDone:NO];
}

- (void) checkEmailSub : (uint) currentSession
{
	if(checkNetworkState(CONNEXION_TEST_IN_PROGRESS))
	{
		while(currentSession == self.currentEditingSession && checkNetworkState(CONNEXION_TEST_IN_PROGRESS))
		{
			usleep(5000);
		}
	}
	
	if(currentSession == self.currentEditingSession && !checkNetworkState(CONNEXION_OK))
	{
		currentStatus = AUTHEMAIL_STATE_INVALID;
		return;
	}
	
	if([self.stringValue length] > 100)
	{
		currentStatus = AUTHEMAIL_STATE_INVALID;
		return;
	}
	
	NSString * string = [[[self stringValue] copy] autorelease];
	const char * data = [string cStringUsingEncoding:NSASCIIStringEncoding];
	
	byte retValue = checkLogin(data);
	
	if(currentSession != self.currentEditingSession)
		return;
	
	if(retValue == 2)
	{
		currentStatus = AUTHEMAIL_STATE_INVALID;
		return;
	}
	
	currentStatus = AUTHEMAIL_STATE_GOOD;
	
	if(authController != nil)
		[authController validEmail : retValue == 0 : currentSession];
}

@end