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

@implementation RakSRSearchTab

- (instancetype) initWithFrame:(NSRect)frameRect
{
	self = [super initWithFrame:frameRect];
	
	if(self != nil)
	{
		_isVisible = NO;
		_height = SRSEARCHTAB_DEFAULT_HEIGHT;
		
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(searchWasTriggered:) name:SR_NOTIF_NAME_SEARCH_TRIGGERED object:nil];
	}
	
	return self;
}

- (void) searchWasTriggered : (NSNotification *) notification
{
	NSDictionary * dict = notification.userInfo;
	NSNumber * number;
	
	if(dict != nil && (number = [dict objectForKey:SR_NOTIF_KEY]) != nil && [number isKindOfClass:[NSNumber class]])
	{
		_isVisible = number.boolValue;
		
		if(_isVisible)
			_height = SR_SEARCH_TAB_INITIAL_HEIGHT;
		else
			_height = SRSEARCHTAB_DEFAULT_HEIGHT;
		
		if([self.superview class] == [Series class])
			[(Series *) self.superview resetFrameSize:YES];
	}
}

- (void)drawRect:(NSRect)dirtyRect
{
	[[[NSColor blackColor] colorWithAlphaComponent:0.5] setFill];
	NSRectFill(dirtyRect);
}

@end
