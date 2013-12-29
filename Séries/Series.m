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

#import "Series.h"

#define PREFS_GETTABSERIEWIDTH 1

@implementation Series

- (id)init:(NSWindow*)window
{
    self = [super init];
    if (self)
	{
		flag = GUI_THREAD_SERIES;
		
		NSView *superview = window.contentView;
		NSRect frame = NSMakeRect(0, 0, (int) [Prefs getPref:PREFS_GETTABSERIEWIDTH], window.frame.size.height);
		
		tabSerie = [[NSView alloc] initWithFrame:frame];
		[superview addSubview:tabSerie];
		
		NSColorWell *background = [[NSColorWell alloc] initWithFrame:frame];
		[background setColor:[NSColor redColor]];
		[background setBordered:NO];

		frame.origin.x = 25;			frame.origin.y = frame.size.height - 60;
		frame.size.width = 65;			frame.size.height = 25;
		
		NSButton *button = [[NSButton alloc] initWithFrame:frame];

		[button setTitle:@"Prefs"];
		[button setButtonType:NSMomentaryLightButton];
		[button setBezelStyle:NSRoundedBezelStyle];

		[button setTarget:self];
		[button setAction:@selector(logTest)];
		[tabSerie addSubview:background];
		[tabSerie addSubview:button];
		[button release];
	}
    return self;
}

- (void) logTest
{
	NSLog(@"Coin!!!");
	[self drawRect:self.frame];
}

- (void)drawRect:(NSRect)dirtyRect
{
	[[NSColor whiteColor] setFill];
	NSRectFill(dirtyRect);
	[super drawRect:dirtyRect];
	
    // Drawing code here.
}

@end
