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
 ********************************************************************************************/

enum
{
	BORDER = 200,

	WIDTH = 800,
	HEIGHT = 210,

	THEME_BASE_Y = 160,
	EMAIL_BASE_Y = 90,
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
	self = [super initWithFrame:NSMakeRect(0, 0, WIDTH, HEIGHT)];
	
	if(self != nil)
	{
		//Theme change
		themeSwitch = [[RakSegmentedControl alloc] initWithFrame:_bounds :@[NSLocalizedString(@"PREFS-GENERAL-THEME-DARK", nil), NSLocalizedString(@"PREFS-GENERAL-THEME-LIGHT", nil), NSLocalizedString(@"PREFS-GENERAL-THEME-CUSTOM", nil)]];
		
		if(themeSwitch != nil)
		{
			[themeSwitch setEnabled:YES forSegment:0];
			[themeSwitch setEnabled:YES forSegment:1];
			
			[themeSwitch setSelected:YES forSegment:[Prefs getCurrentTheme:nil] - 1];
			
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
		
		email = [[RakText alloc] initWithText:[NSString localizedStringWithFormat:NSLocalizedString(@"PREFS-GENERAL-LOGGED-IN-ON-%s", nil), COMPTE_PRINCIPAL_MAIL] :[self textColor]];
		if(email != nil)
		{
			[self addSubview: email];
		}
		
		disconnect = [RakButton allocWithText:NSLocalizedString(@"PREFS-GENERAL-LOGOUT", nil)];
		if(disconnect != nil)
		{
			[disconnect setEnabled:NO];
			
			[email setFrameOrigin:NSMakePoint(_bounds.size.width / 2 - (email.bounds.size.width + disconnect.bounds.size.width) / 2, EMAIL_BASE_Y - email.bounds.size.height / 2)];
			[disconnect setFrameOrigin:NSMakePoint(NSMaxX(email.frame) + 10, EMAIL_BASE_Y - disconnect.bounds.size.height / 2)];
			
			[self addSubview: disconnect];
			
		}
		else if(email != nil)
			[email setFrameOrigin:NSMakePoint(_bounds.size.width / 2 - email.bounds.size.width / 2, EMAIL_BASE_Y - email.bounds.size.height / 2)];
		
		resetRemind = [RakButton allocWithText:NSLocalizedString(@"PREFS-GENERAL-RESET-ALERTS", nil)];
		if(resetRemind != nil)
		{
			[resetRemind setEnabled:[RakPrefsRemindPopover haveAnyRemindedValue]];
			
			resetRemind.target = self;
			resetRemind.action = @selector(toogleReset);
			
			[resetRemind setFrameOrigin:NSMakePoint(_bounds.size.width / 2 - resetRemind.bounds.size.width / 2, RESET_BASE_Y)];
			
			[self addSubview:resetRemind];
		}
	}
	
	return self;
}

- (NSRect) frame
{
	return NSMakeRect(0, PREF_BUTTON_BAR_HEIGHT, WIDTH, HEIGHT);
}

- (NSColor *) titleColor
{
	return [Prefs getSystemColor:GET_COLOR_ACTIVE :nil];
}

- (NSColor *) textColor
{
	return [Prefs getSystemColor:GET_COLOR_INACTIVE :nil];
}

#pragma mark - Backend

- (void) feedAnimationController : (RakCTAnimationController *) animationController
{
	[animationController addAction : self : @selector(updateTheme:)];
}

- (void) updateTheme : (RakSegmentedControl*) sender
{
	if(sender == nil)
		sender = themeSwitch;
	
	switch ([sender selectedSegment])
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
	}
}

- (void) toogleReset
{
	[RakPrefsRemindPopover flushRemindedValues];
	[resetRemind setEnabled:NO];
}

@end
