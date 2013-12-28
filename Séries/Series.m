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

@implementation Series

- (id)initWithFrame:(NSRect)frame
{
	frame.origin.x = frame.origin.y = 0;
    self = [super initWithFrame:frame];
    if (self)
	{
		flag = GUI_THREAD_SERIES;
        // Initialization code here.
		
		NSView *superview = self.window.contentView;
		NSRect frame2 = NSMakeRect(0, 0, 200, 100);
		NSButton *button = [[NSButton alloc] initWithFrame:frame2];
		[button setTitle:@"Click me!"];
		
		[button setButtonType:NSMomentaryLightButton]; //Set what type button You want
		[button setBezelStyle:NSRoundedBezelStyle]; //Set what style You want
		
		
		[button setTarget:self];
		[button setAction:@selector(logTest)];
		[superview addSubview:button];
		
		//		[button release];
	}
    return self;
}

- (void) logTest
{
	NSLog(@"Coin!!!");
}

- (void)drawRect:(NSRect)dirtyRect
{
	[super drawRect:dirtyRect];
	
    // Drawing code here.
}

@end
