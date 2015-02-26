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

@implementation RakAboutWindow

- (void) createWindow
{
	if(window == nil)
	{
		window = [[RakWindow alloc] initWithContentRect:NSMakeRect(200, 200, 300, 500) styleMask:NSTitledWindowMask|NSClosableWindowMask backing:NSBackingStoreBuffered defer:YES];
		
		RakContentViewBack * contentView = [[RakContentViewBack alloc] initWithFrame:NSMakeRect(0, 0, 300, 500)];
		
		window.delegate = [NSApp delegate];
		window.contentView = contentView;
		
		[window configure];
		[self fillWindow];
	}
	
	[window orderFront:self];
}

- (BOOL) windowShouldClose:(id)sender
{
	[window orderOut:self];
	return NO;
}

- (void) dealloc
{
	[window close];
}

#pragma mark - Content management

enum
{
	BORDER_ICON = 128,
	BORDER_TOP_ICON = 10,
	BORDER_SEPARATOR = 7
};

- (void) fillWindow
{
	window.title = @"";
	
	NSView * _contentView = (id) window.contentView;
	RakAboutContent * contentView;
	
	if(_contentView == nil)
		return;
	else
	{
		contentView = [[RakAboutContent alloc] initWithFrame:NSMakeRect(WIDTH_BORDER_ALL, WIDTH_BORDER_ALL, _contentView.bounds.size.width - 2 * WIDTH_BORDER_ALL, _contentView.bounds.size.height - 2 * WIDTH_BORDER_ALL)];
		if(contentView == nil)
			return;
		
		[_contentView addSubview:contentView];
	}
	
	NSSize viewSize = contentView.bounds.size;
	CGFloat currentY = viewSize.height, steps[4] = {0, 0, 0, 0};
	icon = [[NSImageView alloc] initWithFrame:NSMakeRect(viewSize.width / 2 - BORDER_ICON / 2, currentY = (currentY - BORDER_TOP_ICON - BORDER_ICON), BORDER_ICON, BORDER_ICON)];
	if(icon != nil)
	{
		NSImage* image = [[NSWorkspace sharedWorkspace] iconForFile:[[NSBundle mainBundle] bundlePath]];
		icon.image = image;
		icon.imageScaling = NSImageScaleAxesIndependently;
		
		[contentView addSubview:icon];
	}
	
	projectName = [[RakText alloc] initWithText:@"Rakshata" :[self mainTextColor]];
	if(projectName != nil)
	{
		projectName.font = [NSFont fontWithName:[Prefs getFontName:GET_FONT_ABOUT] size:18];
		[projectName sizeToFit];
		[projectName setFrameOrigin:NSMakePoint(viewSize.width / 2 - projectName.bounds.size.width / 2, currentY = (currentY - BORDER_TOP_ICON - projectName.bounds.size.height))];
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
		
		[version setFrameOrigin:NSMakePoint(viewSize.width / 2 - version.bounds.size.width / 2, currentY = (currentY - version.bounds.size.height))];
		[contentView addSubview:version];
	}
	
	steps[0] = currentY - BORDER_SEPARATOR;
	currentY -= 2 * BORDER_SEPARATOR;
	
	//Print developer/design
	//Separator
	//Print FOSS
	//Easter egg: 5 clics sur icones => remerciements à Tapbots

	//Draw the separators
	CGFloat step0 = steps[0], step1 = steps[1], step2 = steps[2], step3 = steps[3];
	contentView.haveAdditionalDrawing = YES;
	contentView.additionalDrawing = ^(NSSize size)
	{
		CGFloat _steps[4] = {step0, step1, step2, step3};

		[[NSColor colorWithCalibratedWhite:0.2 alpha:1] setFill];
		
		for(byte i = 0; i < 4 && _steps[i] != 0; i++)
		{
			NSRectFill(NSMakeRect(size.width / 8, _steps[i], size.width - 2 * size.width / 8, 1));
		}
	};
}

- (NSColor *) mainTextColor
{
	return [[NSColor whiteColor] colorWithAlphaComponent:0.9];
}

- (NSColor *) versionTextColor
{
	return [[NSColor whiteColor] colorWithAlphaComponent:0.4];
}

@end
