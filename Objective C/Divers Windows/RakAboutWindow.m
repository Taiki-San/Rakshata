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
	BORDER_SEPARATOR = 3
};

- (void) fillWindow
{
	window.title = @"";
	
	NSView * contentView = window.contentView;
	NSSize viewSize = contentView.bounds.size;
	CGFloat currentY = viewSize.height;
	
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
	
	step = currentY - BORDER_SEPARATOR;
	currentY -= 2 * BORDER_SEPARATOR;
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
