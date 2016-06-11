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
 **			file, You can obtain one at https://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 ********************************************************************************************/

@implementation RakFavsInfo

- (instancetype) autoInit
{
	return [self initWithFrame: NSMakeRect(0, 0, 160, 155)];
}

- (void) launchPopover : (RakView *) anchor
{
	//We check if the user asked not to be annoyed again
	BOOL alreadyAsked, answer = NO;
	alreadyAsked = [RakPrefsRemindPopover getValueReminded : PREFS_REMIND_FAVS : &answer];
	if(!alreadyAsked || !answer || [RakApp window].shiftPressed)
	{
		_anchor = anchor;
		
		[self internalInitWithAnchor:anchor
							 atFrame:NSMakeRect(0, 0, _anchor.frame.size.width, 0)
						wantCallback:YES
				closeOnContextChange:YES];
		
		if(!alreadyAsked)
			[RakPrefsRemindPopover setValueReminded : PREFS_REMIND_FAVS : YES];
	}
}

- (void) setupView
{
	RakText * contentText = [[RakText alloc] initWithText:self.frame :NSLocalizedString(@"FAVORITE-DESCRIPTION", nil) :[Prefs getSystemColor:COLOR_ACTIVE]];
	if(contentText != nil)
	{
		[contentText setAlignment:NSTextAlignmentCenter];
		[contentText setFont:[Prefs getFont:FONT_RD_BUTTONS ofSize:13]];
		[contentText sizeToFit];
		
		[self addSubview : contentText];
	}
	
	RakButton * button = [RakButton allocWithText:NSLocalizedString(@"OK", nil)];
	if(button != nil)
	{
		[button setTarget:self];
		[button setAction:@selector(closePopover)];
		
		[self addSubview:button];
	}
	
	CGFloat cumulatedHeight = 10 + contentText.bounds.size.height + 10 + button.bounds.size.height + 10;
	
	[self setFrameSize:NSMakeSize(_bounds.size.width, cumulatedHeight)];

	[contentText setFrameOrigin:NSMakePoint(_bounds.size.width / 2 - contentText.frame.size.width / 2 , _bounds.size.height - 10 - contentText.frame.size.height)];
	[button setFrameOrigin:NSMakePoint(_bounds.size.width / 2 - button.bounds.size.width / 2, 10)];
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
			[view setTextColor:[Prefs getSystemColor:COLOR_ACTIVE]];
			break;
		}
	}
}

@end
