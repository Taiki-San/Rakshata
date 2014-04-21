/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite  **
 **                                                                                         **
 *********************************************************************************************/

@implementation Series

- (id)init : (NSView*)contentView : (NSString *) state
{
    self = [super init];
    if (self)
	{
		flag = GUI_THREAD_SERIES;
		prefsUIIsOpen = false;
		
		self = [self initView:contentView : state];
		[self initContent : state];
	}
    return self;
}

- (void) initContent : (NSString *) state
{
	/*Initialise la fenêtre de prefs, la position en Y est celle du back button*/
	preferenceButton = [RakButton allocForSeries : self : @"X" : NSMakePoint(SR_PREF_BUTTON_BORDERS, self.frame.size.height - RBB_TOP_BORDURE - RBB_BUTTON_HEIGHT) : self : @selector(gogoWindow)];
	
	winController = [[PrefsUI alloc] init];
	[winController setAnchor:preferenceButton];
	
	[self setupBackButton];
	
	coreView = [[RakSerieView alloc] initContent:[self getCoreviewFrame] : state];
	[self addSubview:coreView];
}

- (NSString *) byebye
{
	NSString * output;
	
	if (coreView == nil || (output = [coreView getContextToGTFO]) == nil)
		return [super byebye];
	else
		[self removeFromSuperview];
	
	return output;
}

- (id) retain
{
	[preferenceButton retain];
	[coreView retain];
	
	return [super retain];
}

- (oneway void) release
{
	[preferenceButton release];
	[coreView release];
	
	[super release];
}

- (void) dealloc
{
	[preferenceButton removeFromSuperview];
	[coreView removeFromSuperview];

	[super retain];
	[self removeFromSuperview];
	
	[super dealloc];
}

/**		Pref UI		**/
#pragma mark - Preference UI

- (void) gogoWindow
{
	prefsUIIsOpen = true;
	[winController showPopover];
}

- (BOOL) abortCollapseReaderTab
{
	return prefsUIIsOpen;
}

- (void)mouseExited:(NSEvent *)theEvent	//Appelé quand je sors
{
	if(!prefsUIIsOpen)
		[super mouseExited:theEvent];
}

#pragma mark - Back button

- (void) setupBackButton
{
	backButton = [[RakBackButton alloc] initWithFrame:[self backButtonFrame]: false];
	[backButton setTarget:self];
	[backButton setAction:@selector(backButtonClicked)];
	[backButton setHidden:!readerMode];
	[self addSubview:backButton];
}

- (NSRect) backButtonFrame
{
	if(preferenceButton != nil)
	{
		NSRect frame = [self bounds];
		
		frame.origin.x = preferenceButton.frame.size.width;
		frame.size.width -= frame.origin.x;
		
		return frame;
	}
	else
		return [self frame];
}

- (void) backButtonClicked
{
	noDrag = true;
	[self mouseUp:NULL];
}

#pragma mark - Routine to setup and communicate with coreview

- (NSRect) getCoreviewFrame
{
	NSRect frame = [self bounds];
	
	frame.size.height -= 2 * (frame.size.height - backButton.frame.origin.y) - backButton.frame.size.height + SR_READERMODE_BOTTOMBAR_WIDTH;
	frame.origin.x = SR_READERMODE_LATERAL_BORDER * frame.size.width / 100.0f;
	frame.origin.y = SR_READERMODE_BOTTOMBAR_WIDTH;
	frame.size.width -= 2* frame.origin.x;	//Pas obligé de recalculer
	
	return frame;
}

/**			Other		**/
#pragma mark - RakTabView routines

- (void) setFrame:(NSRect)frameRect
{
	[super setFrame:frameRect];
	
	[preferenceButton setFrameOrigin:NSMakePoint(preferenceButton.frame.origin.x, frameRect.size.height - RBB_TOP_BORDURE - RBB_BUTTON_HEIGHT)];
	[backButton setFrame:[self backButtonFrame]];
	
	[coreView setFrame:[self getCoreviewFrame]];
}

- (void) refreshViewSize
{
	[super refreshViewSize];
	[self refreshLevelViews: self : REFRESHVIEWS_CHANGE_READER_TAB];
}

- (NSRect) getFrameOfNextTab
{
	NSRect output;
	[Prefs getPref:PREFS_GET_TAB_CT_FRAME :&output];
	
	NSSize sizeSuperView = [self.superview frame].size;
	
	output.origin.x *= sizeSuperView.width / 100.0f;
	output.origin.y *= sizeSuperView.height / 100.0f;
	output.size.width *= sizeSuperView.width / 100.0f;
	output.size.height *= sizeSuperView.height / 100.0f;
	
	return output;
}

- (int) getCodePref : (int) request
{
	int output;
	
	switch (request)
	{
		case CONVERT_CODE_POSX:
		{
			output = PREFS_GET_TAB_SERIE_POSX;
			break;
		}
			
		case CONVERT_CODE_POSY:
		{
			output = PREFS_GET_TAB_SERIE_POSY;
			break;
		}
			
		case CONVERT_CODE_HEIGHT:
		{
			output = PREFS_GET_TAB_SERIE_HEIGHT;
			break;
		}
			
		case CONVERT_CODE_WIDTH:
		{
			output = PREFS_GET_TAB_SERIE_WIDTH;
			break;
		}
			
		case CONVERT_CODE_FRAME:
		{
			output = PREFS_GET_TAB_SERIE_FRAME;
			break;
		}
			
		default:
			output = 0;
	}
	
	return output;
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
	CGFloat var;
	NSRect frame = viewFrame;
	[Prefs getPref:PREFS_GET_TAB_CT_POSX :&var];
	frame.size.width = var * self.superview.frame.size.width / 100;
	frame.origin.x = frame.origin.y = 0;
	return frame;
}

@end
