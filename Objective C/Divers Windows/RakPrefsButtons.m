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
		[Prefs getCurrentTheme:self];
		
		responder = delegate;
		
		NSMutableParagraphStyle *paragraphStyle = NSMutableParagraphStyle.new;
		paragraphStyle.alignment                = kCTTextAlignmentCenter;
		
		buttonGeneral = [RakPrefsSelectionButton allocImageWithoutBackground:@"p_settings" :RB_STATE_STANDARD :self :@selector(clicGeneral)];
		if(buttonGeneral != nil)
		{
			[buttonGeneral setFrameSize:NSMakeSize(buttonGeneral.bounds.size.width + 7, buttonGeneral.bounds.size.height + 7)];
			buttonGeneral.attributedTitle = [[NSAttributedString alloc] initWithString:NSLocalizedString(@"PREFS-TITLE-GENERAL", nil) attributes:
											 @{NSForegroundColorAttributeName : [delegate textColor],
											   NSParagraphStyleAttributeName : paragraphStyle}];
			
			[buttonGeneral setFrameOrigin:NSMakePoint(BUTTON_OFFSET_X, BUTTON_OFFSET_Y)];
			[self addSubview:buttonGeneral];
		}
		
		buttonRepo = [RakPrefsSelectionButton allocImageWithoutBackground:@"p_repo" :RB_STATE_STANDARD :self :@selector(clicRepo)];
		if(buttonRepo != nil)
		{
			buttonRepo.attributedTitle = [[NSAttributedString alloc] initWithString:NSLocalizedString(@"PREFS-TITLE-REPO", nil) attributes:
										  @{NSForegroundColorAttributeName : [delegate textColor],
											NSParagraphStyleAttributeName : paragraphStyle}];
			
			[buttonRepo setFrameOrigin:NSMakePoint(NSMaxX(buttonGeneral.frame) + BUTTON_SEPARATOR_X, BUTTON_OFFSET_Y)];
			
			[self addSubview:buttonRepo];
		}
		
		buttonFav = [RakPrefsSelectionButton allocImageWithoutBackground:@"p_favs" :RB_STATE_STANDARD :self :@selector(clicFav)];
		if(buttonFav != nil)
		{
			[buttonFav setFrameSize:buttonGeneral.bounds.size];
			buttonFav.attributedTitle = [[NSAttributedString alloc] initWithString:NSLocalizedString(@"PREFS-TITLE-FAVS", nil) attributes:
										 @{NSForegroundColorAttributeName : [delegate textColor],
										   NSParagraphStyleAttributeName : paragraphStyle}];
			
			//The english version is cropped unless we increase the width
			[buttonFav setFrameSize:NSMakeSize(buttonFav.bounds.size.width + 2, buttonFav.bounds.size.height)];
			[buttonFav setFrameOrigin:NSMakePoint(NSMaxX(buttonRepo.frame) + BUTTON_SEPARATOR_X, BUTTON_OFFSET_Y)];
			
			[self addSubview:buttonFav];
		}
		
	}
	
	return self;
}

- (void) viewDidMoveToWindow
{
	self.window.title = activeButton.attributedTitle.string;
}

- (void) dealloc
{
	[Prefs deRegisterForChanges:self];
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
	return [Prefs getSystemColor:COLOR_BACKGROUND_PREFS_HEADER :nil];
}

- (NSColor *) borderColor
{
	return [Prefs getSystemColor:COLOR_BORDER_PREFS_HEADER :nil];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if ([object class] != [Prefs class])
		return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	
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
	
	self.window.title = activeButton.attributedTitle.string;
	
	[Prefs setPref:PREFS_SET_ACTIVE_PREFS_PANEL :code];
	[responder focusChanged:code];
	
	if(triggeredClic)
	{
		[sender.cell setState:RB_STATE_HIGHLIGHTED];
		[sender setNeedsDisplay];
	}
}

@end