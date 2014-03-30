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
 ********************************************************************************************/

@implementation CTSelec

- (id)init : (NSView*)contentView : (NSString *) state
{
    self = [super init];
    if (self)
	{
		flag = GUI_THREAD_CT;
		self = [self initView:contentView : state];
		
		self.layer.borderColor = [Prefs getSystemColor:GET_COLOR_BORDER_TABS].CGColor;
		self.layer.borderWidth = 2;
		
		backButton = [[RakBackButton alloc] initWithFrame:[self frame]:1];
		[backButton setTarget:self];
		[backButton setAction:@selector(backButtonClicked)];
		[backButton setHidden:!readerMode];
		[self addSubview:backButton];
		
		//Calculate contentView size
		
		MANGAS_DATA *mangaData = getCopyCache(RDB_LOADALL | SORT_NAME, NULL);
		coreView = [[RakChapterView alloc] initWithFrame:[self calculateContentViewSize] : mangaData[21]];
		[self addSubview:coreView];
	}
    return self;
}

// !!!!!!!! DEBUG PURPOSE ONLY !!!!!!!!!!!
- (void)mouseExited:(NSEvent *)theEvent
{

}

- (void) dealloc
{
	[backButton removeFromSuperview];
	[backButton release];
	[super dealloc];
}

- (void) backButtonClicked
{
	[self mouseDown:NULL];
}

- (NSColor*) getMainColor
{
	return [Prefs getSystemColor:GET_COLOR_BACKGROUND_TABS];
}

- (int) getCodePref : (int) request
{
	int output;
	
	switch (request)
	{
		case CONVERT_CODE_POSX:
		{
			output = PREFS_GET_TAB_CT_POSX;
			break;
		}
			
		case CONVERT_CODE_POSY:
		{
			output = PREFS_GET_TAB_CT_POSY;
			break;
		}
			
		case CONVERT_CODE_HEIGHT:
		{
			output = PREFS_GET_TAB_CT_HEIGHT;
			break;
		}
			
		case CONVERT_CODE_WIDTH:
		{
			output = PREFS_GET_TAB_CT_WIDTH;
			break;
		}
	}
	
	return output;
}

- (void) setUpViewForAnimation : (BOOL) reader
{
	[backButton setHidden:!reader];
}

#pragma mark - Reader code
/**		Reader		**/

- (void) readerIsOpening
{
	[super readerIsOpening];
	
}

- (BOOL) isStillCollapsedReaderTab
{
	int state;
	[Prefs getPref:PREFS_GET_READER_TABS_STATE :&state];
	return (state & STATE_READER_TAB_CT_FOCUS) == 0;
}

- (NSRect) generateNSTrackingAreaSize : (NSRect) viewFrame
{
	CGFloat posCT, posReader;
	NSRect frame = viewFrame;
	[Prefs getPref:PREFS_GET_TAB_CT_POSX :&posCT];
	[Prefs getPref:PREFS_GET_TAB_READER_POSX :&posReader];
	frame.size.width = (posReader - posCT) * self.superview.frame.size.width / 100;
	frame.origin.x = frame.origin.y = 0;
	return frame;
}

#pragma mark - Self code used in reader mode

- (NSRect) calculateContentViewSize
{
	NSRect frame = [self frame];

	//frame.origin.y = bordure + size, 2*bordure + size = 2*y - size
	frame.size.height -= 2 * (frame.size.height - backButton.frame.origin.y) - backButton.frame.size.height + CT_READERMODE_BOTTOMBAR_WIDTH;
	frame.origin.x = CT_READERMODE_LATERAL_BORDER * frame.size.width / 100.0f;
	frame.origin.y = CT_READERMODE_BOTTOMBAR_WIDTH;
	frame.size.width -= 2* frame.origin.x;	//Pas obligé de recalculer

	return frame;
}

- (void) setFrame:(NSRect)frameRect
{
	[super setFrame:frameRect];
	[coreView setFrame:[self calculateContentViewSize]];
}

@end
