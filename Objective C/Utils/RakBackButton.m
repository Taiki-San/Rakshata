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

@implementation RakBackButton

- (id)initWithFrame:(NSRect)frame
{
	frame.origin.x = frame.size.width / 8;
	frame.origin.y = frame.size.height - 40;
	frame.size.width *= 0.75;
	frame.size.height = 25;
	
    self = [super initWithFrame:frame];
    if (self)
	{
		[self setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
		[self setAutoresizesSubviews:YES];
		[self setNeedsDisplay:YES];
		[self setWantsLayer:true];
		[self.layer setCornerRadius:5];
    }
    return self;
}

- (void)drawRect:(NSRect)dirtyRect
{
	[[self getColorBackground] setFill];
	NSRectFill(dirtyRect);
}

#pragma mark - Color

- (NSColor *) getColorBackground
{
	return [Prefs getSystemColor:GET_COLOR_BACKGROUD_BACK_BUTTONS];
}

- (NSColor *) getColorTextStandBy
{
	return [Prefs getSystemColor:GET_COLOR_INACTIVE];
}

- (NSColor *) getColorTextActivating
{
	return [Prefs getSystemColor:GET_COLOR_SURVOL];
}

- (NSColor *) getColorClick
{
	return [Prefs getSystemColor:GET_COLOR_ACTIVE];
}

@end
