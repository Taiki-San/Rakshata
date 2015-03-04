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
	WINDOW_HEIGHT = 400,
	WINDOW_WIDTH = 600,
	
	BUTTON_BAR_HEIGHT = 65
};

@implementation RakPrefsWindow

+ (NSSize) defaultWindowSize
{
	return NSMakeSize(WINDOW_WIDTH, WINDOW_HEIGHT);
}

- (NSRect) contentFrame : (NSView *) content
{
	NSRect frame = [super contentFrame:content];
	
	CGFloat offset = frame.origin.x;
	
	frame.origin.x -= offset - 2;
	frame.origin.y -= offset;
	frame.size.height += 2 * offset - 2;
	frame.size.width += 2 * offset - 4;
	
	return frame;
}

- (void) fillWindow
{
	[super fillWindow];

	header = [[RakPrefsButtons alloc] initWithFrame:NSMakeRect(0, WINDOW_HEIGHT - BUTTON_BAR_HEIGHT, WINDOW_WIDTH, BUTTON_BAR_HEIGHT) :self];
	if(header != nil)
	{
		[contentView addSubview:header];
	}
}

- (void) loadButtons
{
}

#pragma mark - Color

- (NSColor *) textColor
{
	return [Prefs getSystemColor:GET_COLOR_CLICKABLE_TEXT :nil];
}

#pragma mark - Buttons responder

- (void) focusChanged : (byte) newTab
{
	
}

@end
