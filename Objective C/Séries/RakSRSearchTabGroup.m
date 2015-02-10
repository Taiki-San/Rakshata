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

#include "db.h"

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
			close = [RakButton allocWithText:@"Refermer ▲" :_bounds];
			if(close != nil)
			{
				[close sizeToFit];

				close.hasBorder = NO;
				[close setButtonType:NSOnOffButton];
				
				[close triggerBackground];
	
				[close setTarget:self];
				[close setAction:@selector(close)];

				[close.cell setHighlightAllowed:NO];
				[close setFrameOrigin:NSMakePoint(_bounds.size.width / 2 - close.bounds.size.width / 2, _bounds.size.height - 5 - close.bounds.size.height)];
				[close.cell setFont:[NSFont fontWithName:[Prefs getFontName:GET_FONT_RD_BUTTONS] size:11]];
				
				[self addSubview:close];
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
		return RDBS_TYPE_TYPE;
	
	NSLog(@"Not supported yet");
	return 255;
}

- (void) setFrame:(NSRect)frameRect
{
	[super setFrame:frameRect];
	
	if(_ID == SEARCH_BAR_ID_EXTRA)
	{
		[close setFrameOrigin:NSMakePoint(frameRect.size.width / 2 - close.bounds.size.width / 2, frameRect.size.height - 5 - close.bounds.size.height)];
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
		[close.animator setFrameOrigin:NSMakePoint(frameRect.size.width / 2 - close.bounds.size.width / 2, frameRect.size.height - 5 - close.bounds.size.height)];
	}
	else
	{
		[list resizeAnimation:[self getTableFrame:frameRect]];
		[searchBar resizeAnimation:[self getSearchFrame:frameRect]];
	}
}

#define BORDER_LIST 3
#define BORDER_SEARCH_LIST 4

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
}

@end
