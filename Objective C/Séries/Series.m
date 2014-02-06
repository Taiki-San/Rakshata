/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                          **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite   **
 **                                                                                          **
 *********************************************************************************************/

@implementation Series

- (id)init:(NSWindow*)window
{
    self = [super init];
    if (self)
	{
		flag = GUI_THREAD_SERIES;
		prefsUIIsOpen = false;
		
		[self setUpView:window.contentView];
		
		[self craftPrefButton:self.frame];
		
		/*Initialise la fenêtre de prefs*/
		winController = [[PrefsUI alloc] init];
		[winController setAnchor:button];
		
		/*****		TEST		*****/
		NSView *test = [[NSView alloc] initWithFrame:NSMakeRect(50, 0, 100, 400)];
		CALayer *viewLayer = [CALayer layer];
		[viewLayer setBackgroundColor:CGColorCreateGenericRGB(0.0, 0.0, 0.0, 1)]; //RGB plus Alpha Channel
		[test setWantsLayer:YES]; // view's backing store is using a Core Animation Layer
		[test setLayer:viewLayer];
		[self addSubview:test];
		
		/*Initialise le bandeau inférieur*/
		footer = [[RakFooter alloc] init:window];
		[self addSubview:footer];
	}
    return self;
}

/**		Pref UI		**/

- (void) craftPrefButton : (NSRect) tabSerieFrame
{
	NSRect frame;
	
	frame.origin.x = 25;			frame.origin.y = tabSerieFrame.size.height - 45;
	frame.size.width = 65;			frame.size.height = 28;
	button = [[RakButton alloc] initWithFrame:frame];
	
	[button setTitle:@"Prefs"];
	[button setButtonType:NSMomentaryLightButton];
	[button setBezelStyle:NSRoundedBezelStyle];
	
	[button setTarget:self];
	[button setAction:@selector(gogoWindow)];
	[self addSubview:button];
}

- (BOOL) getIfPrefUIOpen
{
	return prefsUIIsOpen;
}

- (BOOL) setIfPrefUIOpen : (BOOL) input
{
	BOOL retValue = prefsUIIsOpen != input;
	prefsUIIsOpen = input;
	return retValue;
}

- (void) gogoWindow
{
	[self setIfPrefUIOpen:YES];
	[winController showPopover];
}

- (BOOL) abortCollapseReaderTab
{
	return [self getIfPrefUIOpen];
}

- (void)mouseExited:(NSEvent *)theEvent	//Appelé quand je sors
{
	if(!prefsUIIsOpen)
		[super mouseExited:theEvent];
}

/**			Other		**/

- (void) setFrameSize:(NSSize)newSize
{
	[super setFrameSize:newSize];
	[button refreshViewSize];
}

- (void) refreshViewSize
{
	[super refreshViewSize];
	[self refreshLevelViews: self];
}

- (void) drawContentView: (NSRect) frame
{
	[[NSColor redColor] setFill];
	[super drawContentView:frame];
}

- (int) convertTypeToPrefArg : (bool) getX
{
	return PREFS_GET_TAB_SERIE_WIDTH + [super convertTypeToPrefArg:getX];
}

/**		Reader		**/
- (BOOL) isStillCollapsedReaderTab
{
	int state;
	[Prefs getPref:PREFS_GET_READER_TABS_STATE :&state];
	return (state & STATE_READER_TAB_SERIE_FOCUS) == 0;
}

- (NSRect) generateNSTrackingAreaSize : (NSRect) viewFrame
{
	CGFloat var, heightMDL;
	NSRect frame = viewFrame;
	[Prefs getPref:PREFS_GET_TAB_CT_POSX :&var];
	frame.size.width = var * self.window.frame.size.width / 100;
	frame.origin.x = 0;
	
	[Prefs getPref:PREFS_GET_MDL_HEIGHT:&heightMDL];
	frame.size.height -= self.window.frame.size.height * heightMDL / 100;
	frame.origin.y = self.window.frame.size.height * heightMDL / 100;
	
	return frame;
}

@end
