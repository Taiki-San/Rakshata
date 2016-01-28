/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 ********************************************************************************************/

enum
{
	BORDER = 150,
	
	WIDTH = 600,
	HEIGHT = 170,
	
	THEME_BASE_Y = 130,
	EMAIL_BASE_Y = 83,
	RESET_BASE_Y = 25
};

@interface RakPrefsGeneralView ()
{
	RakSegmentedControl * themeSwitch;
	
	RakText * textTheme, * email;
	
	RakButton * disconnect, * resetRemind;
}

@end

@implementation RakPrefsGeneralView

- (instancetype) initWithFrame : (NSRect) frameRect
{
	self = [super initWithFrame:NSMakeRect(frameRect.origin.x, frameRect.origin.y, WIDTH, HEIGHT)];
	
	if(self != nil)
	{
		[Prefs registerForChange:self forType:KVO_THEME];

		//Theme change
		themeSwitch = [[RakSegmentedControl alloc] initWithFrame:_bounds :@[NSLocalizedString(@"PREFS-GENERAL-THEME-DARK", nil), NSLocalizedString(@"PREFS-GENERAL-THEME-LIGHT", nil), NSLocalizedString(@"PREFS-GENERAL-THEME-CUSTOM", nil)]];
		
		if(themeSwitch != nil)
		{
			[themeSwitch setEnabled:YES forSegment:0];
			[themeSwitch setEnabled:YES forSegment:1];
			
			if(checkFileExist(CUSTOM_COLOR_FILE))
				[themeSwitch setEnabled:YES forSegment:2];
			
			[themeSwitch setSelected:YES forSegment:[Prefs getCurrentTheme] - 1];
			
			[themeSwitch setFrameOrigin:NSMakePoint(BORDER, THEME_BASE_Y - themeSwitch.bounds.size.height / 2)];
			
			[self addSubview:themeSwitch];
		}
		
		textTheme = [[RakText alloc] initWithText:NSLocalizedString(@"PREFS-GENERAL-CHOOSE-THEME", nil) : [self titleColor]];
		if(textTheme != nil)
		{
			[textTheme setFrameOrigin:NSMakePoint(BORDER - textTheme.frame.size.width, THEME_BASE_Y - textTheme.bounds.size.height / 2)];
			
			[self addSubview:textTheme];
		}
		
		//Email
		email = [[RakText alloc] initWithText:[self emailString] :[self textColor]];
		if(email != nil)
		{
			[self addSubview: email];
		}
		
		disconnect = [RakButton allocWithText:NSLocalizedString(@"PREFS-GENERAL-LOGOUT", nil)];
		if(disconnect != nil)
		{
			[disconnect.cell setActiveAllowed:NO];
			[self emailFrameUpdate];
			
			disconnect.target = self;
			disconnect.action = @selector(disconnect);
			
			[self addSubview: disconnect];
			
		}
		else if(email != nil)
			[email setFrameOrigin:NSMakePoint(_bounds.size.width / 2 - email.bounds.size.width / 2, EMAIL_BASE_Y - email.bounds.size.height / 2)];
		
		resetRemind = [RakButton allocWithText:NSLocalizedString(@"PREFS-GENERAL-RESET-ALERTS", nil)];
		if(resetRemind != nil)
		{
			[resetRemind setEnabled:[RakPrefsRemindPopover haveAnyRemindedValue]];
			[resetRemind.cell setActiveAllowed:NO];
			
			resetRemind.target = self;
			resetRemind.action = @selector(toogleReset);
			
			[resetRemind setFrameOrigin:NSMakePoint(_bounds.size.width / 2 - resetRemind.bounds.size.width / 2, RESET_BASE_Y)];
			
			[self addSubview:resetRemind];
		}
		
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(emailUpdate) name:NOTIFICATION_MAIL object:nil];
	}
	
	return self;
}

- (void) dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	[Prefs deRegisterForChange:self forType:KVO_THEME];
}

- (NSString *) emailString
{
	if(COMPTE_PRINCIPAL_MAIL != NULL)
		return [NSString localizedStringWithFormat:NSLocalizedString(@"PREFS-GENERAL-LOGGED-IN-ON-%s", nil), COMPTE_PRINCIPAL_MAIL];

	return NSLocalizedString(@"PREFS-GENERAL-NOT-LOGGED-IN", nil);
}

- (NSRect) frame
{
	return NSMakeRect(0, PREF_BUTTON_BAR_HEIGHT, WIDTH, HEIGHT);
}

- (RakColor *) titleColor
{
	return [Prefs getSystemColor:COLOR_ACTIVE];
}

- (RakColor *) textColor
{
	return [Prefs getSystemColor:COLOR_INACTIVE];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	
	textTheme.textColor = [self titleColor];
	email.textColor = [self textColor];
	
	[self setNeedsDisplay:YES];
}

#pragma mark - Backend

- (void) emailUpdate
{
	email.stringValue = [self emailString];
	[email sizeToFit];
	
	[self emailFrameUpdate];
}

- (void) emailFrameUpdate
{
	if(COMPTE_PRINCIPAL_MAIL != NULL)
	{
		[email setFrameOrigin:NSMakePoint(_bounds.size.width / 2 - (email.bounds.size.width + disconnect.bounds.size.width) / 2, EMAIL_BASE_Y - email.bounds.size.height / 2)];
		[disconnect setFrameOrigin:NSMakePoint(NSMaxX(email.frame) + 10, EMAIL_BASE_Y - disconnect.bounds.size.height / 2)];
		
		if(disconnect.isHidden)
			disconnect.hidden = NO;
		
		disconnect.alphaValue = 1;
	}
	else
	{
		disconnect.hidden = YES;
		[email setFrameOrigin:NSMakePoint(_bounds.size.width / 2 - email.bounds.size.width / 2, EMAIL_BASE_Y - email.bounds.size.height / 2)];
	}
}

- (void) feedAnimationController : (RakCTAnimationController *) animationController
{
	[animationController addAction : self];
}

- (void) disconnect
{
	[RakApp.MDL removingEmailAddress];
	deleteEmail();
	
	email.stringValue = [self emailString];
	[email sizeToFit];
	
	disconnect. alphaAnimated = 0;
	[email setFrameOriginAnimated:NSMakePoint(_bounds.size.width / 2 - email.bounds.size.width / 2, EMAIL_BASE_Y - email.bounds.size.height / 2)];
}

- (void) animationOver : (RakAnimationController *) controller
{
	[self updateTheme];
}

- (void) updateTheme
{
	switch ([themeSwitch selectedSegment])
	{
		case 0:
		{
			[Prefs setCurrentTheme:THEME_CODE_DARK];
			break;
		}
			
		case 1:
		{
			[Prefs setCurrentTheme:THEME_CODE_LIGHT];
			break;
		}
			
		case 2:
		{
			[Prefs setCurrentTheme:THEME_CODE_CUSTOM];
			break;
		}
	}
}

- (void) toogleReset
{
	[RakPrefsRemindPopover flushRemindedValues];
	[resetRemind setEnabled:NO];
}

@end
