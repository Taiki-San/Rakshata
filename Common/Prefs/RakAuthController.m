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

//Arbitrary frames come from IB

@implementation RakAuthController

- (void) launch
{
	if(![RakApp isKindOfClass:[RakAppDelegate class]])
		return;
	
	initialAnimation = YES;
	[Prefs registerForChange:self forType:KVO_THEME];
	
	self.view.wantsLayer = YES;
	self.view.layer.backgroundColor = [Prefs getSystemColor:COLOR_TABS_BACKGROUND].CGColor;
	self.view.layer.cornerRadius = 4;
	
	baseHeight = self.view.bounds.size.height;
	baseContainerHeight = container.bounds.size.height;
	
	currentMode = AUTH_MODE_DEFAULT;
	self.postProcessing = NO;
	
	[self updateMainView];
	
	foreground = [[RakAuthForegroundView alloc] init : self.view];
	foreground.delegate = self;
	
	footerPlaceholder = [[RakText alloc] initWithText:container.bounds : NSLocalizedString(@"AUTH-FOOTER-PLACEHOLDER", nil) : [Prefs getSystemColor : COLOR_ACTIVE]];
	[footerPlaceholder setAlignment:NSTextAlignmentCenter];
	[footerPlaceholder sizeToFit];
	
	NSPoint origin = NSCenteredViews(container, footerPlaceholder);	origin.y += 5;
	[footerPlaceholder setFrameOrigin: origin];
	[container addSubview:footerPlaceholder];
	
	[foreground switchState];
}

- (void) updateMainView
{
	[header setTextColor:[Prefs getSystemColor:COLOR_SURVOL]];
	[header sizeToFit];
	[header setFrameOrigin:NSMakePoint(self.view.frame.size.width / 2 - header.frame.size.width / 2, header.frame.origin.y)];
	
	[labelMail setTextColor:[Prefs getSystemColor:COLOR_SURVOL]];
	[labelPass setTextColor:[Prefs getSystemColor:COLOR_SURVOL]];
	
	mailInput = [[RakEmailField alloc] initWithFrame:_containerMail.frame];
	[self.view addSubview:mailInput];
	[_containerMail removeFromSuperview];
	[mailInput addController:self];
	
	passInput = [[RakPassField alloc] initWithFrame:_containerPass.frame];
	[self.view addSubview:passInput];
	[_containerPass removeFromSuperview];
	[passInput addController : self];
}

#pragma mark - Communication with internal elements

- (void) wakePassUp
{
	passInput.wantCustomBorder = YES;
	passInput.currentStatus = AUTH_STATE_NONE;
	
	if([NSThread isMainThread])
		[passInput setNeedsDisplay:YES];
	else
		[passInput performSelectorOnMainThread:@selector(setNeedsDisplay) withObject:nil waitUntilDone:YES];
}

- (void) validEmail : (BOOL) newAccount : (uint) session
{
	//For now, we assume we start from an empty state
	NSRect frame;
	
	[NSAnimationContext beginGrouping];
	[[NSAnimationContext currentContext] setDuration:0.2f];
	
	if(initialAnimation)	//We remove the placeholder
	{
		frame = footerPlaceholder.frame;
		frame.origin.y = -frame.size.height;
		[footerPlaceholder.animator setFrame:frame];
		[footerPlaceholder.animator setAlphaValue:0];
		initialAnimation = NO;
	}
	else if(newAccount && currentMode == AUTH_MODE_LOGIN)
		[self animateLogin : NO];
	
	else if(!newAccount && currentMode == AUTH_MODE_NEW_ACCOUNT)
		[self animationSignup : NO];
	
	if(newAccount)
	{
		[self animationSignup : YES];
		
		[[NSAnimationContext currentContext] setCompletionHandler:^{
			[self postAnimationWorkSignup:YES];
			
			if(currentMode == AUTH_MODE_LOGIN)
				[self postAnimationWorkLogin:NO];
		}];
	}
	else
	{
		[self animateLogin : YES];
		
		[[NSAnimationContext currentContext] setCompletionHandler:^{
			[self postAnimationWorkLogin:YES];
			
			if(currentMode == AUTH_MODE_LOGIN)
				[self postAnimationWorkSignup:NO];
		}];
	}
	
	[NSAnimationContext endGrouping];
	
	currentMode = newAccount ? AUTH_MODE_NEW_ACCOUNT : AUTH_MODE_LOGIN;
}

- (void) clickedLogin
{
	[self.view.window makeFirstResponder:self.view];
	
	if(!self.postProcessing && [self isMailValid] && [self isPassValid])
		[self clickedConfirm:NO];
}

- (void) clickedSignup
{
	[self.view.window makeFirstResponder:self.view];
	
	if(!self.postProcessing && [self isMailValid] && [self isPassValid] && [self isTermAccepted])
		[self clickedConfirm:YES];
}

- (void) clickedConfirm : (BOOL) signup
{
	const char * email = [mailInput.stringValue UTF8String], * pass = [passInput.stringValue UTF8String];
	
	switch (login((char*) email, (char*) pass, signup))
	{
		case 1:
		{
			passInput.currentStatus = AUTH_STATE_GOOD;
			[passInput display];
			
			self.postProcessing = YES;
			
			if(COMPTE_PRINCIPAL_MAIL == NULL || strcmp(email, COMPTE_PRINCIPAL_MAIL))
			{
				updateEmail(email);
				[self.view.window makeFirstResponder:self.view];
				[self performSelectorInBackground:@selector(loginPostProcessing) withObject:nil];
			}
			else
			{
				dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.2 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{ [foreground switchState]; });
			}
			break;
		}
			
		case 0:
		{
			passInput.currentStatus = AUTH_STATE_INVALID;
			[passInput setNeedsDisplay];
			break;
		}
			
		case 2:
		{
			break;
		}
	}
}

- (void) loginPostProcessing
{
	createSecurePasswordDB(NULL);
	[foreground performSelectorOnMainThread:@selector(switchState) withObject:nil waitUntilDone:NO];
}

#pragma mark - Check

- (BOOL) isMailValid
{
	return mailInput.currentStatus == AUTH_STATE_GOOD;
}

- (BOOL) isPassValid
{
	if([passInput.stringValue length] == 0)
	{
		passInput.currentStatus = AUTH_STATE_INVALID;
		[passInput setNeedsDisplay];
		return NO;
	}
	return YES;
}

- (BOOL) isTermAccepted
{
	return accept.state == NSOnState;
}

#pragma mark - Animation

- (void) animationSignup : (BOOL) appear
{
	if(privacy == nil)
	{
		privacy = [[RakClickableText alloc] initWithText:NSLocalizedString(@"AUTH-PRIVACY", nil) :[Prefs getSystemColor:COLOR_CLICKABLE_TEXT] responder:self];
		[privacy setFrameOrigin:NSMakePoint(container.bounds.size.width / 2 - privacy.bounds.size.width / 2, 0)];	//y = 17
		
		privacy.URL = @"https://www.rakshata.com/privacy";
		[container addSubview:privacy];
	}
	else
		[privacy setHidden:NO];
	
	if(terms == nil)
	{
		terms = [[RakClickableText alloc] initWithText:NSLocalizedString(@"AUTH-CGU", nil) :[Prefs getSystemColor:COLOR_CLICKABLE_TEXT] responder:self];
		[terms setFrameOrigin:NSMakePoint(container.bounds.size.width / 2 - (terms.bounds.size.width + 32) / 2, 49)];
		
		terms.URL = @"https://www.rakshata.com/terms";
		[container addSubview:terms];
	}
	else
		[terms setHidden:NO];
	
	if(accept == nil)
	{
		accept = [[RakAuthTermsButton alloc] init];
		accept.controller = self;
		
		[accept setFrameOrigin:NSMakePoint(0, 49 + terms.bounds.size.height / 2 - accept.bounds.size.height / 2)];
		[container addSubview:accept];
	}
	else
		[accept setHidden:NO];
	
	if(confirm == nil)
	{
		confirm = [RakButton allocWithText:NSLocalizedString(@"AUTH-CREATE", nil)];
		if(confirm != nil)
		{
			[confirm.cell setActiveAllowed:NO];
			[confirm setFrameOrigin:NSMakePoint(container.bounds.size.width / 2 - confirm.bounds.size.width / 2, 14)];
			[container addSubview:confirm];
			[confirm setTarget:self];
			[confirm setAction:@selector(clickedSignup)];
		}
	}
	else
		[confirm setHidden:NO];
	
	NSRect frame = self.view.frame;
	if(appear && !self.offseted)
	{
		//Resize main view
		[self.view.animator setFrame:NSMakeRect(frame.origin.x, self.view.superview.bounds.size.height / 2 - frame.size.height / 2 - 51, frame.size.width, baseHeight +  51)];
		
		[mailInput.animator setFrameOrigin:NSMakePoint(mailInput.frame.origin.x, mailInput.frame.origin.y + 51)];
		[passInput.animator setFrameOrigin:NSMakePoint(passInput.frame.origin.x, passInput.frame.origin.y + 51)];
		
		[container.animator setFrame:NSMakeRect(0, 0, container.bounds.size.width, baseContainerHeight + 51)];
		
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
		
		self.offseted = YES;
	}
	else if(!appear && self.offseted)
	{
		[self.view.animator setFrame : NSMakeRect(frame.origin.x, self.view.superview.bounds.size.height / 2 - frame.size.height / 2, frame.size.width, baseHeight)];
		
		[mailInput.animator setFrameOrigin:NSMakePoint(mailInput.frame.origin.x, mailInput.frame.origin.y - 51)];
		[passInput.animator setFrameOrigin:NSMakePoint(passInput.frame.origin.x, passInput.frame.origin.y - 51)];
		
		[container.animator setFrame : NSMakeRect(0, 0, container.bounds.size.width, baseContainerHeight)];
		
		[privacy setFrameOrigin:NSMakePoint(privacy.frame.origin.x, 74)];
		[privacy.animator setFrameOrigin:NSMakePoint(privacy.frame.origin.x, container.bounds.size.height)];
		
		[accept setFrameOrigin:NSMakePoint(NSMaxX(terms.frame) + 10, accept.frame.origin.y)];
		[accept.animator setFrameOrigin:NSMakePoint(container.bounds.size.width, accept.frame.origin.y)];
		
		[terms setFrameOrigin:NSMakePoint(container.bounds.size.width / 2 - (terms.bounds.size.width + 32) / 2, terms.frame.origin.y)];
		[terms.animator setFrameOrigin:NSMakePoint(-terms.bounds.size.width, terms.frame.origin.y)];
		
		[confirm setAlphaValue:1];
		[confirm.animator setAlphaValue:0];
		[confirm setFrameOrigin:NSMakePoint(confirm.frame.origin.x, 14)];
		[confirm.animator setFrameOrigin:NSMakePoint(confirm.frame.origin.x, -confirm.frame.size.height)];
		
		self.offseted = NO;
	}
}

- (void) postAnimationWorkSignup : (BOOL) appear
{
	if(!appear)
	{
		[privacy setHidden:YES];
		[terms setHidden:YES];
		[terms setFrameOrigin:NSMakePoint(container.bounds.size.width / 2 - (terms.bounds.size.width + 32) / 2, terms.frame.origin.y)];
		[accept setHidden:YES];
		[confirm setHidden:YES];
	}
}

- (void) animateLogin : (BOOL) appear
{
	NSRect frame = container.bounds;	frame.origin = NSZeroPoint;
	
	if(forgottenPass == nil)
	{
		forgottenPass = [RakButton allocWithText:NSLocalizedString(@"AUTH-PASS-FORGOTTEN", nil)];
		if(forgottenPass != nil)
		{
			[forgottenPass setFrameOrigin:NSMakePoint(0, frame.size.height / 2 - forgottenPass.frame.size.height / 2 + 3)];
			[container addSubview:forgottenPass];
		}
	}
	else
		[forgottenPass setHidden:NO];
	
	if(_login == nil)
	{
		_login = [RakButton allocWithText:NSLocalizedString(@"AUTH-LOGIN", nil)];
		if(_login != nil)
		{
			[_login setFrameOrigin:NSMakePoint(0, frame.size.height / 2 - _login.frame.size.height / 2 + 3)];
			
			[_login setTarget:self];
			[_login setAction:@selector(clickedLogin)];
			
			[container addSubview:_login];
		}
	}
	else
		[_login setHidden:NO];
	
	CGFloat border = (container.bounds.size.width - 40 - forgottenPass.bounds.size.width - _login.bounds.size.width) / 3;
	
	if(appear)
	{
		[forgottenPass setFrameOrigin:NSMakePoint(-forgottenPass.bounds.size.width, forgottenPass.frame.origin.y)];
		[forgottenPass.animator setFrameOrigin:NSMakePoint(border, forgottenPass.frame.origin.y)];
		
		[_login setFrameOrigin:NSMakePoint(frame.size.width, _login.frame.origin.y)];
		[_login.animator setFrameOrigin:NSMakePoint(frame.size.width - border - _login.frame.size.width, _login.frame.origin.y)];
	}
	else
	{
		[forgottenPass setFrameOrigin:NSMakePoint(border, forgottenPass.frame.origin.y)];
		[forgottenPass.animator setFrameOrigin:NSMakePoint(-forgottenPass.bounds.size.width, forgottenPass.frame.origin.y)];
		
		[_login setFrameOrigin:NSMakePoint(frame.size.width - border - _login.frame.size.width, _login.frame.origin.y)];
		[_login.animator setFrameOrigin:NSMakePoint(frame.size.width, _login.frame.origin.y)];
	}
}

- (void) postAnimationWorkLogin : (BOOL) appear
{
	if(!appear)
	{
		[forgottenPass setHidden:YES];
		[_login setHidden:YES];
	}
}

#pragma mark - Delegate

- (void) switchOver : (NSNumber*) isDisplayed
{
	if(isDisplayed != nil && [isDisplayed isKindOfClass:[NSNumber class]])
	{
		if([isDisplayed boolValue])
		{
			[self.view.window makeFirstResponder : mailInput];
		}
		else
		{
			[header removeFromSuperview];
			[labelMail removeFromSuperview];
			[mailInput removeFromSuperview];
			[labelPass removeFromSuperview];
			[passInput removeFromSuperview];
			[container removeFromSuperview];
			
			[RakApp loginPromptClosed];
		}
	}
}

- (void) focusLeft : (id) caller : (NSUInteger) flag
{
	char offset = 1;
	switch (flag)
	{
		case NSReturnTextMovement:
		{
			[self enterPressed];
			break;
		}
			
		case NSBacktabTextMovement:
		{
			offset = -1;
		}
		case NSTabTextMovement:
		{
			id tab[3] = {mailInput, passInput, accept};
			byte nbElem = currentMode == AUTH_MODE_NEW_ACCOUNT ? 3 : 2;
			
			for(byte i = 0; i < nbElem; i++)
			{
				if(tab[i] == caller)
				{
					[self.view.window makeFirstResponder: tab[(nbElem + i + offset) % nbElem]];
					break;
				}
			}
			
			break;
		}
	}
}

- (void) enterPressed
{
	if(mailInput.currentStatus != AUTH_STATE_GOOD)
		[self.view.window makeFirstResponder:mailInput];
	else if([[passInput stringValue] length] == 0)
		[self.view.window makeFirstResponder:passInput];
	
	else if(currentMode == AUTH_MODE_NEW_ACCOUNT)
	{
		if([self isTermAccepted])
		{
			[confirm performClick:self];
			[self clickedSignup];
		}
		else
			[self.view.window makeFirstResponder:accept];
		
	}
	else if(currentMode == AUTH_MODE_LOGIN)
	{
		[_login performClick:self];
		[self clickedLogin];
	}
}

- (void) respondTo : (RakClickableText *) sender
{
	NSString * string = sender.URL;
	
	if(string != nil)
		ouvrirSite([string UTF8String]);
}

@end

