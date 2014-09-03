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

@implementation RakFavsInfo

- (instancetype) autoInit
{
	return [self initWithFrame: NSMakeRect(0, 0, 160, 155)];
}

- (void) launchPopover : (NSView *) anchor
{
	//We check if the user asked not to be annoyed again
	BOOL alreadyAsked, answer = NO;
	alreadyAsked = [RakPrefsRemindPopover getValueReminded : PREFS_REMIND_FAVS : &answer];
	if(!alreadyAsked || !answer || [(RakAppDelegate*) [NSApp delegate] window].shiftPressed)
	{
		_anchor = anchor;
		
		[Prefs getCurrentTheme:self];
		
		[self internalInit: anchor : NSMakeRect(0, 0, _anchor.frame.size.width, 0) : YES];
		
		if(!alreadyAsked)
			[RakPrefsRemindPopover setValueReminded : PREFS_REMIND_FAVS : YES];
	}
}

- (void) setupView
{
	RakText * contentText = [[[RakText alloc] initWithText:self.frame :@"Rakshata vérifie\nsi de nouvelles\nsorties existent\npour vos favoris\nà chaque lancement\net vous en notifie" :[Prefs getSystemColor : GET_COLOR_ACTIVE:nil]] autorelease];
	[contentText setAlignment:NSCenterTextAlignment];
	[contentText setFont:[NSFont fontWithName:[Prefs getFontName:GET_FONT_RD_BUTTONS] size:13]];
	[contentText sizeToFit];
	[self addSubview : contentText];
	[contentText setFrameOrigin:NSMakePoint(self.frame.size.width / 2 - contentText.frame.size.width / 2 , self.frame.size.height - 10 - contentText.frame.size.height)];
	
	RakButton * button = [[RakButton allocWithText:@"Ok" : self.bounds] autorelease];
	[button sizeToFit];
	[button setFrameOrigin:NSMakePoint(self.bounds.size.width / 2 - button.bounds.size.width / 2, 10)];
	[button setTarget:self];
	[button setAction:@selector(closePopover)];
	[self addSubview:button];
}

- (void) configurePopover:(INPopoverController *)internalPopover
{
	[super configurePopover:internalPopover];
	
	internalPopover.closesWhenApplicationBecomesInactive = YES;
	internalPopover.closesWhenPopoverResignsKey = YES;
}

- (void) additionalUpdateOnThemeChange
{
	[super additionalUpdateOnThemeChange];
	
	for(RakText * view in self.subviews)
	{
		if([view class] == [RakText class])
		{
			[view setTextColor:[Prefs getSystemColor : GET_COLOR_ACTIVE:nil]];
			break;
		}
	}
}

@end
