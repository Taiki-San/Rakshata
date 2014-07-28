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
	originalSize = self.view.frame.size;
	
	RakText * inactiveConfirm = [[RakText alloc] initWithText:container.bounds : @"Pas encore de compte? Remplissez, on se charge du reste!\nElle ne sera transmise à aucun tiers" : [Prefs getSystemColor : GET_COLOR_ACTIVE : nil]];
	[inactiveConfirm setAlignment:NSCenterTextAlignment];
	[inactiveConfirm sizeToFit];
	
	[inactiveConfirm setFrameOrigin: NSMakePoint(container.frame.size.width / 2 - inactiveConfirm.frame.size.width / 2, 10)];
	[container addSubview:inactiveConfirm];
	
	[foreground switchState];
}

- (void) updateMainView
{
	[header setTextColor:[Prefs getSystemColor:GET_COLOR_SURVOL :nil]];
	[header sizeToFit];
	[header setFrameOrigin:NSMakePoint(self.view.frame.size.width / 2 - header.frame.size.width / 2, header.frame.origin.y)];
	
	[labelMail setTextColor:[Prefs getSystemColor:GET_COLOR_SURVOL :nil]];
	[labelPass setTextColor:[Prefs getSystemColor:GET_COLOR_SURVOL :nil]];
	
	mailInput = [[[RakEmailField alloc] initWithFrame:_containerEmail.frame] autorelease];
	[self.view addSubview:mailInput];	[_containerEmail removeFromSuperview];	_containerEmail = nil;
	[mailInput addController:self];

	passInput = [[[RakPassField alloc] initWithFrame: _containerPass.frame] autorelease];
	[self.view addSubview:passInput];	[_containerPass removeFromSuperview];	_containerPass = nil;
	
	[mailInput setNextKeyView:passInput];	//don't work
	[passInput setNextKeyView:mailInput];
}

- (void) validEmail : (BOOL) newAccount : (uint) session
{
	
}

#pragma mark - Delegate

- (void) switchOver : (NSNumber*) isDisplayed
{
	if(isDisplayed != nil && [isDisplayed isKindOfClass:[NSNumber class]] && ![isDisplayed boolValue])
	{
		[header removeFromSuperview];
		[labelMail removeFromSuperview];
		[mailInput removeFromSuperview];
		[labelPass removeFromSuperview];
		[passInput removeFromSuperview];
		[container removeFromSuperview];
		
		[self release];
	}
}

@end

@implementation RakEmailField

- (instancetype) initWithFrame : (NSRect) frameRect
{
	self = [super initWithFrame:frameRect];
	
	if(self != nil)
	{
		currentStatus = AUTHEMAIL_STATE_NONE;
		
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

@implementation RakPassField

- (instancetype) initWithFrame : (NSRect) frameRect
{
	self = [super initWithFrame:frameRect];
	
	if(self != nil)
	{
		[self setBezeled:NO];
		[self setBordered:YES];
		self.wantCustomBorder = YES;

		[self setBackgroundColor:[Prefs getSystemColor:GET_COLOR_BACKGROUND_TEXTFIELD :nil]];
		[self setTextColor:[Prefs getSystemColor:GET_COLOR_CLICKABLE_TEXT :nil]];
	}
	
	return self;
}

+ (Class) cellClass
{
	return [RakPassFieldCell class];
}

@end