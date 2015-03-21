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
	THEME_BASE_Y = 350,
	EMAIL_BASE_Y = 270
};

@interface RakPrefsGeneralView ()
{
	RakSegmentedControl * themeSwitch;
	
	RakText * textTheme, * email;
	
	RakButton * disconnect;
}

@end

@implementation RakPrefsGeneralView

- (instancetype) initWithFrame : (NSRect) frameRect
{
	self = [super initWithFrame:frameRect];
	
	if(self != nil)
	{
		//Theme change
		
		themeSwitch = [[RakSegmentedControl alloc] initWithFrame:_bounds :@[@"Thème sombre", @"Thème clair", @"Thème customisé"]];
		
		if(themeSwitch != nil)
		{
			[themeSwitch setEnabled:YES forSegment:0];
			[themeSwitch setEnabled:YES forSegment:1];
			
			[themeSwitch setSelected:YES forSegment:[Prefs getCurrentTheme:nil] - 1];
			
			[themeSwitch setFrameOrigin:NSMakePoint(BORDER, THEME_BASE_Y - themeSwitch.bounds.size.height / 2)];
			
			[self addSubview:themeSwitch];
		}
		
		textTheme = [[RakText alloc] initWithText:@"Thème graphique : " : [self titleColor]];
		if(textTheme != nil)
		{
			[textTheme setFrameOrigin:NSMakePoint(BORDER - textTheme.frame.size.width, THEME_BASE_Y - textTheme.bounds.size.height / 2)];
			
			[self addSubview:textTheme];
		}
		
		//Email
		
		email = [[RakText alloc] initWithText:[NSString stringWithFormat:@"Vous êtes connecté avec le compte %s", COMPTE_PRINCIPAL_MAIL] :[self textColor]];
		if(email != nil)
		{
			[self addSubview: email];
		}
		
		disconnect = [RakButton allocWithText:@"Déconnecter" :NSZeroRect];
		if(disconnect != nil)
		{
			[disconnect sizeToFit];
			
			[email setFrameOrigin:NSMakePoint(_bounds.size.width / 2 - (email.bounds.size.width + disconnect.bounds.size.width) / 2, EMAIL_BASE_Y - email.bounds.size.height / 2)];
			[disconnect setFrameOrigin:NSMakePoint(NSMaxX(email.frame) + 10, EMAIL_BASE_Y - disconnect.bounds.size.height / 2)];
			
			[self addSubview: disconnect];
			
		}
		else if(email != nil)
			[email setFrameOrigin:NSMakePoint(_bounds.size.width / 2 - email.bounds.size.width / 2, EMAIL_BASE_Y - email.bounds.size.height / 2)];
	}
	
	return self;
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

@end
