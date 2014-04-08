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

- (id)init : (NSView*)contentView : (NSString *) state
{
    self = [super init];
    if (self)
	{
		flag = GUI_THREAD_SERIES;
		prefsUIIsOpen = false;
		
		self = [self initView:contentView : state];
		[self initContent];
	}
    return self;
}

- (void) initContent
{
	preferenceButton = [RakButton initForSeries : self : @"X" : NSMakePoint(SR_PREF_BUTTON_BORDERS, self.frame.size.height - SR_PREF_BUTTON_BORDERS) : self : @selector(gogoWindow)];
	
	/*Initialise la fenêtre de prefs*/
	winController = [[PrefsUI alloc] init];
	[winController setAnchor:preferenceButton];
	
}

/**		Pref UI		**/

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

/**			Other		**/

- (void) setFrame:(NSRect)frameRect
{
	[super setFrame:frameRect];
	
	[preferenceButton setFrameOrigin:NSMakePoint(preferenceButton.frame.origin.x, frameRect.size.height - SR_PREF_BUTTON_BORDERS - preferenceButton.frame.size.height / 2)];
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

- (NSColor*) getMainColor
{
	return [NSColor redColor];
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
