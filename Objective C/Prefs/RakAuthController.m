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

enum
{
	AUTH_STATE_GOOD,
	AUTH_STATE_LOADING,
	AUTH_STATE_NONE,
	AUTH_STATE_INVALID
};

//Arbitrary frames come from IB

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
	
	footerPlaceholder = [[RakText alloc] initWithText:container.bounds : @"Votre compte vous donne accès aux créations et offres de nombreux artistes\nPas encore de compte? Remplissez, on se charge du reste!\nElle ne sera transmise à aucun tiers" : [Prefs getSystemColor : GET_COLOR_ACTIVE : nil]];
	[footerPlaceholder setAlignment:NSCenterTextAlignment];
	[footerPlaceholder sizeToFit];
	
	[footerPlaceholder setFrameOrigin: NSMakePoint(container.frame.size.width / 2 - footerPlaceholder.frame.size.width / 2, 10)];
	[container addSubview:footerPlaceholder];
	
	[foreground switchState];
}

- (void) updateMainView
{
	[header setTextColor:[Prefs getSystemColor:GET_COLOR_SURVOL :nil]];
	[header sizeToFit];
	[header setFrameOrigin:NSMakePoint(self.view.frame.size.width / 2 - header.frame.size.width / 2, header.frame.origin.y)];
	
	[labelMail setTextColor:[Prefs getSystemColor:GET_COLOR_SURVOL :nil]];
	[labelPass setTextColor:[Prefs getSystemColor:GET_COLOR_SURVOL :nil]];
	
	mailInput = [[[RakEmailField alloc] initWithFrame:_containerMail.bounds] autorelease];
	[_containerMail addSubview:mailInput];
	[mailInput addController:self];

	passInput = [[[RakPassField alloc] initWithFrame:_containerPass.bounds] autorelease];
	[_containerPass addSubview:passInput];
	
	[mailInput setNextKeyView:passInput];	//don't work
	[passInput setNextKeyView:mailInput];
}

- (void) wakePassUp
{
	passInput.wantCustomBorder = YES;
	passInput.currentStatus = AUTH_STATE_NONE;
	
	if([NSThread isMainThread])
		[passInput setNeedsDisplay:YES];
	else
		[passInput performSelectorOnMainThread:@selector(setNeedsDisplay) withObject:nil waitUntilDone:NO];
}

- (void) validEmail : (BOOL) newAccount : (uint) session
{
	//For now, we assume we start from an empty state
	NSRect frame;
	
	[NSAnimationContext beginGrouping];
	
	[[NSAnimationContext currentContext] setDuration:0.2f];

	//We remove the placeholder
	frame = footerPlaceholder.frame;
	frame.origin.y = -frame.size.height;
	[footerPlaceholder.animator setFrame:frame];
	[footerPlaceholder.animator setAlphaValue:0];
	
	if(newAccount)
		[self animationSignup : YES];
	else
		[self animateLogin : YES];
	
	[NSAnimationContext endGrouping];
}

#pragma mark - Animation

- (void) animationSignup : (BOOL) appear
{
	if(privacy == nil)
	{
		privacy = [[[RakTextClickable alloc] initWithText:container.bounds :@"Vos coordonnées ne seront pas transmise à des tiers" :[Prefs getSystemColor:GET_COLOR_SURVOL :nil]] autorelease];
		[privacy setFrameOrigin:NSMakePoint(container.bounds.size.width / 2 - privacy.bounds.size.width / 2, 0)];	//y = 17
		
		privacy.URL = @"https://www.rakshata.com/privacy";
		[container addSubview:privacy];
	}
	else if(privacy.isHidden)
		[privacy setHidden:NO];
	
	if(terms == nil)
	{
		terms = [[[RakTextClickable alloc] initWithText:container.bounds :@"Veuillez accepter les conditions d'utilisation" :[Prefs getSystemColor:GET_COLOR_SURVOL :nil]] autorelease];
		[terms setFrameOrigin:NSMakePoint(0, 49)];
		
		terms.URL = @"https://www.rakshata.com/terms";
		[container addSubview:terms];
	}
	else if(terms.isHidden)
		[terms setHidden:NO];
	
	if(accept == nil)
	{
		accept = [[NSButton alloc] initWithFrame : container.bounds];
		[accept setButtonType:NSSwitchButton];
		[accept setImagePosition:NSImageOnly];
		[accept sizeToFit];
		
		[accept setFrameOrigin:NSMakePoint(0, 49 + terms.bounds.size.height / 2 - accept.bounds.size.height / 2)];
		[container addSubview:accept];
	}
	else if(accept.isHidden)
		[accept setHidden:NO];
	
	if(confirm == nil)
	{
		confirm = [[RakButton allocWithText:@"Créer un compte" : container.bounds] autorelease];
		[confirm sizeToFit];
		[confirm setFrameOrigin:NSMakePoint(container.bounds.size.width / 2 - confirm.bounds.size.width / 2, 14)];
		[container addSubview:confirm];
	}
	else if(confirm.isHidden)
		[confirm setHidden:NO];
	
	if(appear)
	{
		//Resize main view
		NSRect frame = self.view.frame;
		[self.view.animator setFrame:NSMakeRect(frame.origin.x, self.view.superview.bounds.size.height / 2 - originalSize.height / 2 - 51, frame.size.width, frame.size.height +  51)];
		[container.animator setFrame:NSMakeRect(0, 0, container.bounds.size.width, container.bounds.size.height + 51)];
		
		[privacy setFrameOrigin:NSMakePoint(privacy.frame.origin.x, container.bounds.size.height)];
		[privacy.animator setFrameOrigin:NSMakePoint(privacy.frame.origin.x, 74)];
		
		[accept setFrameOrigin:NSMakePoint(container.bounds.size.width, accept.frame.origin.y)];
		[accept.animator setFrameOrigin:NSMakePoint(NSMaxX(terms.frame) + 10, accept.frame.origin.y)];
		
		[terms setFrameOrigin:NSMakePoint(-terms.bounds.size.width, terms.frame.origin.y)];
		[terms.animator setFrameOrigin:NSMakePoint(container.bounds.size.width / 2 - (terms.bounds.size.width + 32) / 2, terms.frame.origin.y)]; 	//32 = border + button width
		
		[confirm setAlphaValue:0];
		[confirm.animator setAlphaValue:1];
		[confirm setFrameOrigin:NSMakePoint(confirm.frame.origin.x, -confirm.frame.size.height)];
		[confirm.animator setFrameOrigin:NSMakePoint(confirm.frame.origin.x, 14)];
	}
}

- (void) postAnimationWorkSignup : (BOOL) appear
{
	if(!appear)
	{
		[privacy setHidden:YES];
		[terms setHidden:YES];
		[accept setHidden:YES];
		[confirm setHidden:YES];
	}
}

- (void) animateLogin : (BOOL) appear
{
	if(forgottenPass == nil)
	{
		forgottenPass = [[RakButton allocWithText:@"Mot de passe oublié?" : container.bounds] autorelease];
		[forgottenPass sizeToFit];
		[forgottenPass setFrameOrigin:NSMakePoint(0, container.bounds.size.height / 2 - forgottenPass.frame.size.height / 2 + 3)];
		[container addSubview:forgottenPass];
	}
	else if(forgottenPass.isHidden)
		[forgottenPass setHidden:NO];
	
	if(login == nil)
	{
		login = [[RakButton allocWithText:@"Connexion" : container.bounds] autorelease];
		[login sizeToFit];
		[container addSubview:login];
		[login setFrameOrigin:NSMakePoint(0, container.bounds.size.height / 2 - login.frame.size.height / 2 + 3)];
	}
	else if(login.isHidden)
		[login setHidden:NO];

	CGFloat border = (container.bounds.size.width - 40 - forgottenPass.bounds.size.width - login.bounds.size.width) / 3;
	
	if(appear)
	{
		[forgottenPass setFrameOrigin:NSMakePoint(-forgottenPass.bounds.size.width, forgottenPass.frame.origin.y)];
		[forgottenPass.animator setFrameOrigin:NSMakePoint(border, forgottenPass.frame.origin.y)];
		
		[login setFrameOrigin:NSMakePoint(container.bounds.size.width, login.frame.origin.y)];
		[login.animator setFrameOrigin:NSMakePoint(container.bounds.size.width - border - login.frame.size.width, login.frame.origin.y)];
	}
	else
	{
		[forgottenPass setFrameOrigin:NSMakePoint(border, forgottenPass.frame.origin.y)];
		[forgottenPass.animator setFrameOrigin:NSMakePoint(-forgottenPass.bounds.size.width, forgottenPass.frame.origin.y)];
		
		[login setFrameOrigin:NSMakePoint(container.bounds.size.width - border - login.frame.size.width, login.frame.origin.y)];
		[login.animator setFrameOrigin:NSMakePoint(container.bounds.size.width, login.frame.origin.y)];
	}
}

- (void) postAnimationWorkLogin : (BOOL) appear
{
	if(!appear)
	{
		[forgottenPass setHidden:YES];
		[login setHidden:YES];
	}
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
		currentStatus = AUTH_STATE_NONE;
		
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
	if(currentStatus != AUTH_STATE_INVALID && !checkNetworkState(CONNEXION_OK) && !checkNetworkState(CONNEXION_TEST_IN_PROGRESS))
	{
		currentStatus = AUTH_STATE_INVALID;
	}
	
	switch (currentStatus)
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

- (void)textDidBeginEditing:(NSNotification *)aNotification
{
	self.currentEditingSession++;
	currentStatus = AUTH_STATE_NONE;
}

- (void)textDidEndEditing:(NSNotification *)aNotification
{
	uint currentSession = self.currentEditingSession;
	
	currentStatus = AUTH_STATE_LOADING;
	[self setNeedsDisplay];
	
	[self performSelectorInBackground:@selector(checkEmail:) withObject:@(currentSession)];
}

#pragma mark - Core

- (void) checkEmail : (NSNumber *) session
{
	if(session != nil && [session isKindOfClass:[NSNumber class]])
	{
		uint currentSession = [session unsignedIntValue];
		
		[CATransaction begin];
		[CATransaction setDisableActions:YES];
		
		[self checkEmailSub : currentSession];
		
		if(currentStatus != AUTH_STATE_GOOD)
			[CATransaction commit];
	}
	else
		currentStatus = AUTH_STATE_INVALID;
	
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
		currentStatus = AUTH_STATE_INVALID;
		return;
	}
	
	if([self.stringValue length] > 100)
	{
		currentStatus = AUTH_STATE_INVALID;
		return;
	}
	
	NSString * string = [[[self stringValue] copy] autorelease];
	const char * data = [string cStringUsingEncoding:NSASCIIStringEncoding];
	
	byte retValue = checkLogin(data);
	
	if(currentSession != self.currentEditingSession)
		return;
	
	if(retValue == 2)
	{
		currentStatus = AUTH_STATE_INVALID;
		return;
	}
	
	currentStatus = AUTH_STATE_GOOD;
	
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

- (void)textDidEndEditing:(NSNotification *)aNotification
{
	self.currentStatus = AUTH_STATE_LOADING;
	[self setNeedsDisplay];
}

+ (Class) cellClass
{
	return [RakPassFieldCell class];
}

@end