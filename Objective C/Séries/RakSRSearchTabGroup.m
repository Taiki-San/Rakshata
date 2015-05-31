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
	BORDER_X_FREE_BUTTON = 10,
	BORDER_FREE_BUTTON_TEXT = 5,
	BORDER_Y_FREE = 8,
	
	BORDER_LIST = 3,
	BORDER_SEARCH_LIST = 4
};

@implementation RakSRSearchTabGroup

- (instancetype) initWithFrame:(NSRect)frameRect : (byte) ID
{
	self = [super initWithFrame:frameRect];
	
	if(self != nil)
	{
		_ID = ID;
		
		if(_ID != SEARCH_BAR_ID_EXTRA)
		{
			list = [[RakSRSearchList alloc] init:[self getTableFrame : _bounds] :[self getRDBSCodeForID]];
			if(list != nil)
				[self addSubview:[list getContent]];
			
			searchBar = [[RakSRSearchBar alloc] initWithFrame:[self getSearchFrame:_bounds] :_ID];
			if(searchBar != nil)
			{
				[self addSubview:searchBar];
			}
		}
		else
		{
			close = [RakButton allocWithText:NSLocalizedString(@"PROJ-CLOSE-FILTER-ARRAY", nil)];
			if(close != nil)
			{
				[close setTarget:self];
				[close setAction:@selector(close)];
				
				[close.cell setActiveAllowed:NO];
				[close setFrameOrigin:NSMakePoint(_bounds.size.width / 2 - close.bounds.size.width / 2, _bounds.size.height - 5 - close.bounds.size.height)];
				[close.cell setFont:[NSFont fontWithName:[Prefs getFontName:GET_FONT_RD_BUTTONS] size:11]];
				
				[self addSubview:close];
			}
			
			freeSwitch = [[RakSwitchButton alloc] init];
			if(freeSwitch != nil)
			{
				freeText = [[RakText alloc] initWithText:NSLocalizedString(@"PROJ-FILTER-FREE-ONLY", nil) :[self textColor]];
				if(freeText != nil)
				{
					freeSwitch.state = NSOffState;
					freeSwitch.target = self;
					freeSwitch.action = @selector(triggerFree);
					
					[self addSubview:freeSwitch];
					
					freeText.clicTarget = self;
					freeText.clicAction = @selector(switchTriggerFree);
					
					[self addSubview:freeText];
				}
				else
					freeSwitch = nil;
			}
		}
		
		self.wantsLayer = YES;
		self.layer.cornerRadius = 3;
		self.layer.backgroundColor = [Prefs getSystemColor:GET_COLOR_SEARCHBAR_BACKGROUND :nil].CGColor;
	}
	
	return self;
}

- (byte) getRDBSCodeForID
{
	if(_ID == SEARCH_BAR_ID_AUTHOR)
		return RDBS_TYPE_AUTHOR;
	else if(_ID == SEARCH_BAR_ID_SOURCE)
		return RDBS_TYPE_SOURCE;
	else if(_ID == SEARCH_BAR_ID_TAG)
		return RDBS_TYPE_TAG;
	else if(_ID == SEARCH_BAR_ID_TYPE)
		return RDBS_TYPE_CAT;
	
	NSLog(@"Not supported yet");
	return 255;
}

- (void) setFrame:(NSRect)frameRect
{
	[super setFrame:frameRect];
	
	if(_ID == SEARCH_BAR_ID_EXTRA)
	{
		NSRect freeSwitchFrame = freeSwitch.frame, freeTextFrame = freeText.frame;
		
		NSPoint newOrigin = NSMakePoint(frameRect.size.width / 2 - close.bounds.size.width / 2, frameRect.size.height - 5 - close.bounds.size.height);
		[close setFrameOrigin:newOrigin];
		
		const CGFloat cumulatedWidth = freeSwitchFrame.size.width + BORDER_FREE_BUTTON_TEXT + freeTextFrame.size.width;
		newOrigin = NSMakePoint(frameRect.size.width / 2 - cumulatedWidth / 2, newOrigin.y - BORDER_Y_FREE - freeSwitchFrame.size.height);
		[freeSwitch setFrameOrigin:newOrigin];
		
		newOrigin = NSMakePoint(newOrigin.x + freeSwitchFrame.size.width + BORDER_FREE_BUTTON_TEXT, newOrigin.y + (freeSwitchFrame.size.height / 2 - freeTextFrame.size.height / 2));
		[freeText setFrameOrigin:newOrigin];
	}
	else
	{
		[list setFrame:[self getTableFrame:frameRect]];
		[searchBar setFrame:[self getSearchFrame:frameRect]];
	}
}

- (void) resizeAnimation:(NSRect)frameRect
{
	[self.animator setFrame:frameRect];
	
	if(_ID == SEARCH_BAR_ID_EXTRA)
	{
		NSRect freeSwitchFrame = freeSwitch.frame, freeTextFrame = freeText.frame;
		
		NSPoint newOrigin = NSMakePoint(frameRect.size.width / 2 - close.bounds.size.width / 2, frameRect.size.height - 5 - close.bounds.size.height);
		[close.animator setFrameOrigin:newOrigin];
		
		const CGFloat cumulatedWidth = freeSwitchFrame.size.width + BORDER_FREE_BUTTON_TEXT + freeTextFrame.size.width;
		newOrigin = NSMakePoint(frameRect.size.width / 2 - cumulatedWidth / 2, newOrigin.y - BORDER_Y_FREE - freeSwitchFrame.size.height);
		[freeSwitch setFrameOrigin:newOrigin];
		
		newOrigin = NSMakePoint(newOrigin.x + freeSwitchFrame.size.width + BORDER_FREE_BUTTON_TEXT, newOrigin.y + (freeSwitchFrame.size.height / 2 - freeTextFrame.size.height / 2));
		[freeText setFrameOrigin:newOrigin];
	}
	else
	{
		[list resizeAnimation:[self getTableFrame:frameRect]];
		[searchBar resizeAnimation:[self getSearchFrame:frameRect]];
	}
}

- (NSColor *) textColor
{
	return [Prefs getSystemColor:GET_COLOR_SURVOL :nil];
}

- (NSRect) getSearchFrame : (NSRect) frame
{
	frame.origin.y = frame.size.height - SR_SEARCH_FIELD_HEIGHT;
	frame.size.height = SR_SEARCH_FIELD_HEIGHT;
	
	frame.origin.x = 0;
	
	return frame;
}

- (NSRect) getTableFrame : (NSRect) frame
{
	frame.size.height -= SR_SEARCH_FIELD_HEIGHT + BORDER_SEARCH_LIST;
	
	frame.size.width -= 2 * BORDER_LIST;
	
	frame.origin.x = BORDER_LIST;
	frame.origin.y = 0;
	
	return frame;
}

#pragma mark - Responder

- (void) close
{
	[[NSNotificationCenter defaultCenter] postNotificationName: SR_NOTIF_NAME_SEARCH_TRIGGERED
														object:@(SEARCH_BAR_ID_FORCE_CLOSE) userInfo: @{SR_NOTIF_NEW_STATE:@(NO)}];
	[self.window makeFirstResponder:nil];
	close.state = NSOffState;
}

- (void) triggerFree
{
	[[NSNotificationCenter defaultCenter] postNotificationName:NOTIFICATION_FREE_ONLY object:@(freeSwitch.state == NSOnState)];
}

- (void) switchTriggerFree
{
	[freeSwitch performClick:self];
}

@end
