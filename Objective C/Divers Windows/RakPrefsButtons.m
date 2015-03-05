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
	BUTTON_OFFSET_Y = 1,
	BUTTON_OFFSET_X = 25,
	BUTTON_SEPARATOR_X = 10
};

@interface RakPrefsButtons()
{
	RakPrefsSelectionButton * buttonGeneral, * buttonRepo, * buttonFav, * buttonCustom;
	RakPrefsSelectionButton * __weak activeButton;
	RakPrefsWindow * responder;
	
	BOOL triggeredClic;
}

@end

@implementation RakPrefsButtons

- (instancetype) initWithFrame : (NSRect) frameRect : (RakPrefsWindow *) delegate
{
	self = [super initWithFrame:frameRect];
	
	if(self != nil)
	{
		responder = delegate;
		
		buttonGeneral = [RakPrefsSelectionButton allocImageWithoutBackground:@"p_general" :RB_STATE_STANDARD :self :@selector(clicGeneral)];
		if(buttonGeneral != nil)
		{
			buttonGeneral.attributedTitle = [[NSAttributedString alloc] initWithString:NSLocalizedString(@"PREFS-TITLE-GENERAL", nil) attributes:
											 @{NSForegroundColorAttributeName : [delegate textColor]}];
			
			[buttonGeneral setFrameOrigin:NSMakePoint(BUTTON_OFFSET_X, BUTTON_OFFSET_Y)];
			[self addSubview:buttonGeneral];
		}
		
		buttonRepo = [RakPrefsSelectionButton allocImageWithoutBackground:@"p_repo" :RB_STATE_STANDARD :self :@selector(clicRepo)];
		if(buttonRepo != nil)
		{
			buttonRepo.attributedTitle = [[NSAttributedString alloc] initWithString:NSLocalizedString(@"PREFS-TITLE-REPO", nil) attributes:
										  @{NSForegroundColorAttributeName : [delegate textColor]}];
			
			[buttonRepo setFrameOrigin:NSMakePoint(NSMaxX(buttonGeneral.frame) + BUTTON_SEPARATOR_X, BUTTON_OFFSET_Y)];
			
			[self addSubview:buttonRepo];
		}
		
		buttonFav = [RakPrefsSelectionButton allocImageWithoutBackground:@"p_fav" :RB_STATE_STANDARD :self :@selector(clicFav)];
		if(buttonFav != nil)
		{
			buttonFav.attributedTitle = [[NSAttributedString alloc] initWithString:NSLocalizedString(@"PREFS-TITLE-FAVS", nil) attributes:
										 @{NSForegroundColorAttributeName : [delegate textColor]}];
			
			[buttonFav setFrameOrigin:NSMakePoint(NSMaxX(buttonRepo.frame) + BUTTON_SEPARATOR_X, BUTTON_OFFSET_Y)];
			
			[self addSubview:buttonFav];
		}

	}
	
	return self;
}

- (void) drawRect : (NSRect) dirtyRect
{
	[[self backgroundColor] setFill];
	NSRectFill(_bounds);
	
	[[self borderColor] setFill];
	NSRectFill(NSMakeRect(0, 0, _bounds.size.width, 1));
}

- (NSColor *) backgroundColor
{
	return [Prefs getSystemColor:GET_COLOR_BACKGROUND_PREFS_HEADER :nil];
}

- (NSColor *) borderColor
{
	return [Prefs getSystemColor:GET_COLOR_BORDER_PREFS_HEADER :nil];
}

#pragma mark - Clic management

- (void) selectElem : (byte) code
{
	triggeredClic = YES;
	
	switch (code)
	{
		case PREFS_BUTTON_CODE_GENERAL:
			[self clicGeneral];
			break;
			
		case PREFS_BUTTON_CODE_REPO:
			[self clicRepo];
			break;
			
		case PREFS_BUTTON_CODE_FAV:
			[self clicFav];
			break;
			
		case PREFS_BUTTON_CODE_CUSTOM:
			[self clicCustom];
			break;
			
		default:
			break;
	}
	
	triggeredClic = NO;
}

- (void) clicGeneral
{
	[self handleClic:buttonGeneral : PREFS_BUTTON_CODE_GENERAL];
}

- (void) clicRepo
{
	[self handleClic:buttonRepo : PREFS_BUTTON_CODE_REPO];
}

- (void) clicFav
{
	[self handleClic:buttonFav : PREFS_BUTTON_CODE_FAV];
}

- (void) clicCustom
{
	[self handleClic:nil : PREFS_BUTTON_CODE_CUSTOM];
}

- (void) handleClic : (RakPrefsSelectionButton *) sender : (byte) code
{
	if(activeButton != nil)
	{
		[activeButton.cell setState:RB_STATE_STANDARD];
		[activeButton setNeedsDisplay];
	}
	
	activeButton = sender;
	
	self.window.title = activeButton.title;
	
	[responder focusChanged:code];
	
	if(triggeredClic)
	{
		[sender.cell setState:RB_STATE_HIGHLIGHTED];
		[sender setNeedsDisplay];
	}
}

@end