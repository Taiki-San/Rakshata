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

@implementation RakReaderSuggestions

- (instancetype) autoInit
{
	return [self initWithFrame: NSMakeRect(0, 0, 160, 155)];
}

- (void) launchPopover : (NSView *) anchor withProjectID : (uint) cacheDBID
{
	//We check if the user asked not to be annoyed again
	BOOL alreadyAsked, answer = NO;
	alreadyAsked = [RakPrefsRemindPopover getValueReminded : PREFS_REMIND_SUGGESTION : &answer];
	if(!alreadyAsked || !answer || [(RakAppDelegate*) [NSApp delegate] window].shiftPressed)
	{
		_anchor = anchor;
		_cacheDBID = cacheDBID;
		
		[self internalInit: anchor : NSMakeRect(0, 0, _anchor.frame.size.width, 0) : YES];
		
		if(!alreadyAsked)
			[RakPrefsRemindPopover setValueReminded : PREFS_REMIND_SUGGESTION : YES];
	}
}

#pragma mark - View configuration

- (void) setupView
{
	
}

- (void) configurePopover:(INPopoverController *)internalPopover
{
	[super configurePopover:internalPopover];
	
	internalPopover.closesWhenApplicationBecomesInactive = YES;
	internalPopover.closesWhenPopoverResignsKey = YES;
}

#pragma mark - We need to pick the project

//Quand arrive à fin de liste, pop-up sur bouton suivant pour proposer suggestions
//↪ 1 du même auteur (si impossible, même source)
//↪ 1 du même tag (si impossible, catégorie)


- (void) suggestionByAuthor
{
	[self suggestionBySource];
}

- (void) suggestionBySource
{
	
}

- (void) suggestionByTag
{
	[self suggestionByCat];
}

- (void) suggestionByCat
{
	
}

@end
