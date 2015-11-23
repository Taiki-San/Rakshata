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
		[Prefs registerForChange:self forType:KVO_THEME];
		
		responder = delegate;
		
		buttonGeneral = [RakPrefsSelectionButton allocImageWithoutBackground:@"pSettings" :self :@selector(clicGeneral)];
		if(buttonGeneral != nil)
		{
			[buttonGeneral setFrameSize:NSMakeSize(buttonGeneral.bounds.size.width + 7, buttonGeneral.bounds.size.height + 7)];
			buttonGeneral.attributedTitle = [self attributedStringWithLocalization:@"PREFS-TITLE-GENERAL"];
			
			[buttonGeneral setFrameOrigin:NSMakePoint(BUTTON_OFFSET_X, BUTTON_OFFSET_Y)];
			[self addSubview:buttonGeneral];
		}
		
		buttonRepo = [RakPrefsSelectionButton allocImageWithoutBackground:@"pRepo" :self :@selector(clicRepo)];
		if(buttonRepo != nil)
		{
			buttonRepo.attributedTitle = [self attributedStringWithLocalization:@"PREFS-TITLE-REPO"];
			
			[buttonRepo setFrameOrigin:NSMakePoint(NSMaxX(buttonGeneral.frame) + BUTTON_SEPARATOR_X, BUTTON_OFFSET_Y)];
			
			[self addSubview:buttonRepo];
		}
		
		buttonFav = [RakPrefsSelectionButton allocImageWithoutBackground:@"pFavs" :self :@selector(clicFav)];
		if(buttonFav != nil)
		{
			[buttonFav setFrameSize:buttonGeneral.bounds.size];
			buttonFav.attributedTitle = [self attributedStringWithLocalization:@"PREFS-TITLE-FAVS"];
			
			//The english version is cropped unless we increase the width
			[buttonFav setFrameSize:NSMakeSize(buttonFav.bounds.size.width + 2, buttonFav.bounds.size.height)];
			[buttonFav setFrameOrigin:NSMakePoint(NSMaxX(buttonRepo.frame) + BUTTON_SEPARATOR_X, BUTTON_OFFSET_Y)];
			
			[self addSubview:buttonFav];
		}
		
	}
	
	return self;
}

- (NSAttributedString *) attributedStringWithLocalization : (NSString *) localization
{
	NSMutableParagraphStyle *paragraphStyle = NSMutableParagraphStyle.new;
	paragraphStyle.alignment                = kCTTextAlignmentCenter;

	return [[NSAttributedString alloc] initWithString:NSLocalizedString(localization, nil) attributes:
			@{NSForegroundColorAttributeName : [responder textColor],
			  NSParagraphStyleAttributeName : paragraphStyle}];
}

- (void) viewDidMoveToWindow
{
	((RakContentViewBack *) self.window.contentView).title = activeButton.attributedTitle.string;
}

- (void) dealloc
{
	[Prefs deRegisterForChange:self forType:KVO_THEME];
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
	return [Prefs getSystemColor:COLOR_PREFS_HEADER_BACKGROUND];
}

- (NSColor *) borderColor
{
	return [Prefs getSystemColor:COLOR_PREFS_HEADER_BORDER];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];

	buttonGeneral.attributedTitle = [self attributedStringWithLocalization:@"PREFS-TITLE-GENERAL"];
	buttonRepo.attributedTitle = [self attributedStringWithLocalization:@"PREFS-TITLE-REPO"];
	buttonFav.attributedTitle = [self attributedStringWithLocalization:@"PREFS-TITLE-FAVS"];

	[self setNeedsDisplay:YES];
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
	
	((RakContentViewBack *) self.window.contentView).title = activeButton.attributedTitle.string;
	
	[Prefs setPref:PREFS_SET_ACTIVE_PREFS_PANEL :code];
	[responder focusChanged:code];
	
	if(triggeredClic)
	{
		[sender.cell setState:RB_STATE_HIGHLIGHTED];
		[sender setNeedsDisplay];
	}
}

@end