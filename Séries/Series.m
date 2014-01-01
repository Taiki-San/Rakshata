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
		NSView *superview = window.contentView;
		NSRect frame = NSMakeRect(0, 0, [self getRequestedViewWidth:superview], superview.frame.size.height);
		
		tabSerie = [[NSView alloc] initWithFrame:frame];
		[superview addSubview:tabSerie];
		
		NSColorWell *background = [[NSColorWell alloc] initWithFrame:frame];
		[background setColor:[NSColor redColor]];
		[background setBordered:NO];
		[tabSerie addSubview:background];
		[background release];
		
		frame.origin.x = 25;			frame.origin.y = frame.size.height - 45;
		frame.size.width = 65;			frame.size.height = 25;
		button = [[NSButton alloc] initWithFrame:frame];
		
		/*Initialise la fenêtre de prefs*/
		winController = [[PrefsUI alloc] init];
		[winController setAnchor:button];
		
		[self drawView];
	}
    return self;
}

- (int) getRequestedViewWidth:(NSView*)contentView
{
	return contentView.frame.size.width * (int) [Prefs getPref:PREFS_GET_TAB_SERIE_WIDTH] / 100;
}

- (void) gogoWindow
{
	NSRect frame = self.window.frame;
	frame.size.height *= 2;
	[self.window setFrame:frame display:YES];
	[winController showPopover];
}

- (void) resizeView:(NSRect)frame
{

}

- (void) drawView
{
	//	NSRect frame = self->tabSerie.frame;

	[button setTitle:@"Prefs"];
	[button setButtonType:NSMomentaryLightButton];
	[button setBezelStyle:NSRoundedBezelStyle];
	
	[button setTarget:self];
	[button setAction:@selector(gogoWindow)];
	[tabSerie addSubview:button];
}

- (void)drawRect:(NSRect)dirtyRect
{
	[[NSColor whiteColor] setFill];
	NSRectFill(dirtyRect);
	[super drawRect:dirtyRect];
	
    // Drawing code here.
}

@end
