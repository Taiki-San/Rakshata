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

#import "superHeader.h"

@implementation Series

- (id)init:(NSWindow*)window
{
    self = [super init];
    if (self)
	{
		flag = GUI_THREAD_SERIES;
		[self setUpView:window.contentView];
		
		[self craftPrefButton:self.frame];
		
		/*Initialise la fenêtre de prefs*/
		winController = [[PrefsUI alloc] init];
		[winController setAnchor:button];
		
		/*Initialise le bandeau inférieur*/
		footer = [[RakFooter alloc] init:window];
		[self addSubview:footer];
	}
    return self;
}

- (void) craftPrefButton : (NSRect) tabSerieFrame
{
	NSRect frame;
	
	frame.origin.x = 25;			frame.origin.y = tabSerieFrame.size.height - 45;
	frame.size.width = 65;			frame.size.height = 25;
	button = [[NSButton alloc] initWithFrame:frame];
	
	[button setTitle:@"Prefs"];
	[button setButtonType:NSMomentaryLightButton];
	[button setBezelStyle:NSRoundedBezelStyle];
	
	[button setTarget:self];
	[button setAction:@selector(gogoWindow)];
	[self addSubview:button];
}

- (void) gogoWindow
{
	[self setFrame:NSMakeRect(0, 0, self.frame.size.width+50, self.frame.size.height+50)];
	[winController showPopover];
}

- (void) refreshViewSize
{
	[super refreshViewSize];
	[self refreshLevelViews: self];
}

- (void) drawContentView: (NSRect) frame
{
	//frame.origin.y = frame.size.height * 0.25;
	//frame.size.height *= 0.75;
	[[NSColor redColor] setFill];
	
	[super drawContentView:frame];
}

- (int) convertTypeToPrefArg : (bool) getX
{
	return PREFS_GET_TAB_SERIE_WIDTH + [super convertTypeToPrefArg:getX];
}

@end
