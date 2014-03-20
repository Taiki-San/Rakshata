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

- (id)init:(NSView*)contentView
{
    self = [super init];
    if (self)
	{
		flag = GUI_THREAD_SERIES;
		prefsUIIsOpen = false;
		
		self = [self initView:contentView];
		
		[self craftPrefButton:self.frame];
		
		/*Initialise la fenêtre de prefs*/
		winController = [[PrefsUI alloc] init];
		[winController setAnchor:button];
		
		/*Initialise le bandeau inférieur*/
		footer = [[RakFooter alloc] init:contentView];
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

- (NSColor*) getMainColor
{
	return [NSColor redColor];
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
	frame.size.width = var * self.superview.frame.size.width / 100;
	frame.origin.x = 0;
	
	[Prefs getPref:PREFS_GET_MDL_HEIGHT:&heightMDL];
	frame.size.height -= self.superview.frame.size.height * heightMDL / 100;
	frame.origin.y = self.superview.frame.size.height * heightMDL / 100;
	
	return frame;
}

@end
