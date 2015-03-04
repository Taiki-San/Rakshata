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
	WINDOW_HEIGHT = 400,
	WINDOW_WIDTH = 600,
};

@implementation RakPrefsWindow

+ (NSSize) defaultWindowSize
{
	return NSMakeSize(WINDOW_WIDTH, WINDOW_HEIGHT);
}

- (void) fillWindow
{
	[super fillWindow];
	
	buttonGeneral = [RakPrefsSelectionButton allocImageWithoutBackground:@"p_general" :RB_STATE_STANDARD :self :@selector(clicGeneral)];
	if(buttonGeneral != nil)
	{
		buttonGeneral.attributedTitle = [[NSAttributedString alloc] initWithString:@"Général" attributes:
										 @{NSForegroundColorAttributeName : [self textColor]}];
		
		[buttonGeneral setFrameOrigin:NSMakePoint(50, 300)];
		[contentView addSubview:buttonGeneral];
	}
	
	buttonRepo = [RakPrefsSelectionButton allocImageWithoutBackground:@"p_repo" :RB_STATE_STANDARD :self :@selector(clicRepo)];
	if(buttonRepo != nil)
	{
		buttonRepo.attributedTitle = [[NSAttributedString alloc] initWithString:@"Sources" attributes:
										 @{NSForegroundColorAttributeName : [self textColor]}];
		
		[buttonRepo setFrameOrigin:NSMakePoint(50 + 2 + buttonGeneral.bounds.size.width, 300)];
		
		[contentView addSubview:buttonRepo];
	}
}

#pragma mark - Color

- (NSColor *) textColor
{
	return [Prefs getSystemColor:GET_COLOR_CLICKABLE_TEXT :nil];
}

#pragma mark - Buttons responder

- (void) clicGeneral
{
	[self handleClic:buttonGeneral];
}

- (void) clicRepo
{
	[self handleClic:buttonRepo];
}

- (void) handleClic : (RakPrefsSelectionButton *) sender
{
	if(activeButton != nil)
	{
		[activeButton.cell setState:RB_STATE_STANDARD];
		[activeButton setNeedsDisplay];
	}
	
	activeButton = sender;
}

@end
