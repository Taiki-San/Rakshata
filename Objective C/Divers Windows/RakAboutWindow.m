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

#define PROJECT_URL @"https://www.rakshata.com/"
#define TAIKI_URL @"https://www.twitter.com/Taiki__San"
#define PATRICK_URL @"mailto:planchettegraph@gmail.com"
#define BLAG_URL @"https://www.twitter.com/RakBenoit"

#define EASTER_URL @"https://www.tapbots.com"

#define FOSS_URL @"https://www.rakshata.com/opensource.html"

enum
{
	WINDOW_WIDTH = 276,
	WINDOW_HEIGHT = 356,
	WINDOW_HEIGHT_WITH_EASTER = 526,
	
	BORDER_ICON = 128,
	BORDER_TOP_ICON = 10,
	BORDER_SEPARATOR = 10,
	SEPARATOR_OFFSET = 10,
	
	CAT_OFFSET = 3,
	TITLE_OFFSET = 30,
	INTERLINE_OFFSET = 5,
	
	EASTER_BORDER = 7,
	
	NB_CLIC_REQUIRED_FOR_EASTEREGG = 3
};

@implementation RakAboutWindow

- (void) createWindow
{
	if(window == nil)
	{
		window = [[RakWindow alloc] initWithContentRect:NSMakeRect(200, 200, WINDOW_WIDTH, WINDOW_HEIGHT) styleMask:NSTitledWindowMask|NSClosableWindowMask backing:NSBackingStoreBuffered defer:YES];
		
		window.delegate = [NSApp delegate];
		window.contentView = [[RakContentViewBack alloc] initWithFrame:NSMakeRect(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT)];;
		
		[window configure];
		[self fillWindow];
	}
	else
		[self updateFrames];
	
	[window orderFront:self];
}

- (BOOL) windowShouldClose:(id)sender
{
	[window orderOut:self];
	[self hideEaster];
	
	return NO;
}

- (void) dealloc
{
	[window close];
}

#pragma mark - Content management

- (void) fillWindow
{
	window.title = @"";
	
	NSView * _contentView = (id) window.contentView;
	
	if(_contentView == nil)
		return;
	else
	{
		contentView = [[RakAboutContent alloc] initWithFrame:NSMakeRect(WIDTH_BORDER_ALL, WIDTH_BORDER_ALL, _contentView.bounds.size.width - 2 * WIDTH_BORDER_ALL, _contentView.bounds.size.height - 2 * WIDTH_BORDER_ALL)];
		if(contentView == nil)
			return;
		
		[_contentView addSubview:contentView];
	}
	
	icon = [[RakAboutIcon alloc] initWithFrame:NSMakeRect(0, 0, BORDER_ICON, BORDER_ICON)];
	if(icon != nil)
	{
		icon.clicResponder = self;
		[contentView addSubview:icon];
	}
	
	projectName = [[RakAboutText alloc] initWithText:@"Rakshata" :[self mainTextColor] responder:self];
	if(projectName != nil)
	{
		projectName.font = [NSFont fontWithName:[Prefs getFontName:GET_FONT_ABOUT] size:18];
		[projectName sizeToFit];

		projectName.URL = PROJECT_URL;
		[projectName setupArea];
		
		[contentView addSubview:projectName];
	}
	
	
	NSDictionary * data = [[NSBundle mainBundle] infoDictionary];
	NSString * _version = [data objectForKey:@"CFBundleShortVersionString"], * build = [data objectForKey:@"CFBundleVersion"];
	
	if(data != nil && _version != nil && build != nil)
		version = [[RakText alloc] initWithText:[NSString stringWithFormat:@"Version %@ (%d)", _version, [build intValue]] :[self versionTextColor]];
	else
		version = [[RakText alloc] initWithText:[NSString stringWithFormat:@"Version %s", PRINTABLE_VERSION] :[self versionTextColor]];
	
	if(version != nil)
	{
		version.font = [NSFont fontWithName:[Prefs getFontName:GET_FONT_STANDARD] size:13];
		[version sizeToFit];
		
		[contentView addSubview:version];
	}
	
	//Print developer/design
	devTitle = [[RakText alloc] initWithText:@"Code:" :[self versionTextColor]];
	if(devTitle != nil)
	{
		devTitle.font = [NSFont fontWithName:[Prefs getFontName:GET_FONT_ABOUT] size:13];
		[devTitle sizeToFit];
		
		[contentView addSubview:devTitle];
	}
	
	taikiName = [[RakAboutText alloc] initWithText:@"Taiki" :[self mainTextColor] responder:self];
	if(taikiName != nil)
	{
		taikiName.URL = TAIKI_URL;
		
		[taikiName setupArea];
		
		[contentView addSubview:taikiName];
	}
	
	designTitle = [[RakText alloc] initWithText:@"Design:" :[self versionTextColor]];
	if(designTitle != nil)
	{
		designTitle.font = [NSFont fontWithName:[Prefs getFontName:GET_FONT_ABOUT] size:13];
		[designTitle sizeToFit];
		
		[contentView addSubview:designTitle];
	}
	
	planchette = [[RakAboutText alloc] initWithText:@"Patrick Blanchette" :[self mainTextColor] responder:self];
	if(planchette != nil)
	{
		planchette.URL = PATRICK_URL;
		
		[planchette setupArea];
		
		[contentView addSubview:planchette];
	}

	blag = [[RakAboutText alloc] initWithText:@"Blag" :[self mainTextColor] responder:self];
	if(blag != nil)
	{
		blag.URL = BLAG_URL;
		
		[blag setupArea];
		
		[contentView addSubview:blag];
	}
	
	FOSS = [[RakAboutText alloc] initWithText:NSLocalizedString(@"FOSS-USAGE", nil) :[self mainTextColor] responder:self];
	if(FOSS != nil)
	{
		FOSS.font = [NSFont fontWithName:[Prefs getFontName:GET_FONT_PLACEHOLDER] size:13];
		[FOSS sizeToFit];
		
		FOSS.URL = FOSS_URL;
		[FOSS setupArea];
		
		[contentView addSubview:FOSS];
	}
	
	copyright = [[RakText alloc] initWithText:NSLocalizedString(@"COPYRIGHT", nil) :[self versionTextColor]];
	if(copyright != nil)
	{
		copyright.font = [NSFont fontWithName:[Prefs getFontName:GET_FONT_STANDARD] size:11];
		[copyright sizeToFit];
		
		[contentView addSubview:copyright];
	}
	
	[self updateFrames];
}

#pragma mark - Responder

- (void) respondTo : (RakAboutText *) sender
{
	NSString * string = sender.URL;
	
	if(string != nil)
		ouvrirSite([string UTF8String]);
}

- (void) clicIcon
{
	if(easterCount < NB_CLIC_REQUIRED_FOR_EASTEREGG && ++easterCount == NB_CLIC_REQUIRED_FOR_EASTEREGG)
	{
		[self createEaster];
	}
}

- (void) createEaster
{
	_haveEaster = YES;
	
	if(mainEaster == nil)
	{
		mainEaster = [[RakText alloc] initWithText:@"I want to thanks the Tapbots team for showing me that it is possible to make extremely polished software with a small team. While I don't think that Rakshata reached the level of Tweetbot, we were looking at them while working of Rakshata for OSX for more than two years." :[self mainTextColor]];
		if(mainEaster != nil)
		{
			[mainEaster.cell setWraps:YES];
			mainEaster.fixedWidth = WINDOW_WIDTH - WINDOW_WIDTH / EASTER_BORDER;
			
			mainEaster.alignment = NSJustifiedTextAlignment;
			
			[contentView addSubview:mainEaster];
		}
	}
	else
		mainEaster.hidden = NO;
	mainEaster.alphaValue = 0;

	if(easterLink == nil)
	{
		easterLink = [[RakAboutText alloc] initWithText:@"Check them out!" :[self mainTextColor] responder:self];
		if(easterLink != nil)
		{
			easterLink.URL = EASTER_URL;
			[contentView addSubview:easterLink];
		}
	}
	else
		easterLink.hidden = NO;
	easterLink.alphaValue = 0;
	
	[NSAnimationContext beginGrouping];

	[self updateFrames];
	
	mainEaster.animator.alphaValue = 1;
	easterLink.animator.alphaValue = 1;
	
	[[NSAnimationContext currentContext] setCompletionHandler:^{
		[easterLink setupArea];
		[FOSS updateTrackingAreas];
	}];
	
	[NSAnimationContext endGrouping];
}

- (void) hideEaster
{
	_haveEaster = NO;
	
	mainEaster.hidden = YES;
	easterLink.hidden = YES;
}

#pragma mark - Color

- (NSColor *) mainTextColor
{
	return [[NSColor whiteColor] colorWithAlphaComponent:0.9];
}

- (NSColor *) versionTextColor
{
	return [[NSColor whiteColor] colorWithAlphaComponent:0.4];
}

#pragma mark - Update positions

#define GET_VIEW(a) ((_isInitialized ? a.animator : a))

- (void) updateFrames
{
	if(_haveEaster && window.frame.size.height != WINDOW_HEIGHT_WITH_EASTER)
	{
		NSRect windowFrame = window.frame;
		windowFrame.size.height = WINDOW_HEIGHT_WITH_EASTER;
		[window setFrame:windowFrame display:YES animate:YES];
	}
	else if(!_haveEaster && window.frame.size.height != WINDOW_HEIGHT)
	{
		NSRect windowFrame = window.frame;
		windowFrame.size.height = WINDOW_HEIGHT;
		[window setFrame:windowFrame display:YES animate:YES];
	}
	
	NSSize viewSize = contentView.bounds.size;
	CGFloat currentY = viewSize.height, steps[3] = {0, 0, 0};
	
	[GET_VIEW(icon) setFrameOrigin: NSMakePoint(viewSize.width / 2 - BORDER_ICON / 2, currentY = (currentY - BORDER_TOP_ICON - BORDER_ICON))];
	[GET_VIEW(projectName) setFrameOrigin:NSMakePoint(viewSize.width / 2 - projectName.bounds.size.width / 2, currentY = (currentY - BORDER_TOP_ICON - projectName.bounds.size.height))];
	[GET_VIEW(version) setFrameOrigin:NSMakePoint(viewSize.width / 2 - version.bounds.size.width / 2, currentY = (currentY - version.bounds.size.height))];

	steps[0] = currentY - BORDER_SEPARATOR;
	currentY -= 2 * BORDER_SEPARATOR;
	
	[GET_VIEW(devTitle) setFrameOrigin:NSMakePoint(viewSize.width / CAT_OFFSET - devTitle.bounds.size.width, currentY - devTitle.bounds.size.height)];
	[GET_VIEW(taikiName) setFrameOrigin:NSMakePoint(viewSize.width / 2, currentY - taikiName.bounds.size.height - (taikiName.bounds.size.height / 2 - devTitle.bounds.size.height / 2))];
	currentY = MIN(devTitle.frame.origin.y, taikiName.frame.origin.y) - INTERLINE_OFFSET;
	
	[GET_VIEW(designTitle) setFrameOrigin:NSMakePoint(viewSize.width / CAT_OFFSET - designTitle.bounds.size.width, currentY - designTitle.bounds.size.height)];
	[GET_VIEW(planchette) setFrameOrigin:NSMakePoint(viewSize.width / 2, currentY - planchette.bounds.size.height - (planchette.bounds.size.height / 2 - designTitle.bounds.size.height / 2))];
	currentY = MIN(designTitle.frame.origin.y, planchette.frame.origin.y) - INTERLINE_OFFSET - blag.bounds.size.height;
	
	[GET_VIEW(blag) setFrameOrigin:NSMakePoint(viewSize.width / 2, currentY)];

	steps[1] = currentY - BORDER_SEPARATOR;
	currentY -= 2 * BORDER_SEPARATOR;
	
	if(_haveEaster)
	{
		currentY -= mainEaster.bounds.size.height;
		[mainEaster setFrameOrigin:NSMakePoint(viewSize.width / 2 - mainEaster.bounds.size.width / 2, currentY)];
		
		currentY -= INTERLINE_OFFSET + easterLink.bounds.size.height;
		[easterLink setFrameOrigin:NSMakePoint(viewSize.width / 2 - easterLink.bounds.size.width / 2, currentY)];
		
		steps[2] = currentY - BORDER_SEPARATOR;
		currentY -= 2 * BORDER_SEPARATOR;
	}
	
	currentY -= FOSS.bounds.size.height;
	[GET_VIEW(FOSS) setFrameOrigin:NSMakePoint(viewSize.width / 2 - FOSS.bounds.size.width / 2, currentY)];
	currentY -= INTERLINE_OFFSET + copyright.bounds.size.height;
	[GET_VIEW(copyright) setFrameOrigin:NSMakePoint(viewSize.width / 2 - copyright.bounds.size.width / 2, currentY)];
	
	//Draw the separators
	CGFloat step0 = steps[0], step1 = steps[1], step2 = steps[2];
	contentView.haveAdditionalDrawing = YES;
	contentView.additionalDrawing = ^(NSSize size)
	{
		CGFloat _steps[3] = {step0, step1, step2};
		
		[[NSColor colorWithCalibratedWhite:0.2 alpha:1] setFill];
		
		for(byte i = 0; i < 3 && _steps[i] != 0; i++)
		{
			NSRectFill(NSMakeRect(size.width / SEPARATOR_OFFSET, _steps[i], size.width - 2 * size.width / SEPARATOR_OFFSET, 1));
		}
	};
	
	if(_isInitialized)
		_isInitialized = YES;
}

@end
