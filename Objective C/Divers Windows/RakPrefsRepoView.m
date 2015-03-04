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

@implementation RakPrefsRepoView

- (instancetype) init
{
	self = [self initWithFrame:[self frame]];
	
	if(self != nil)
	{
		
	}
	
	return self;
}

- (NSRect) frame
{
	return NSMakeRect(0, PREF_BUTTON_BAR_HEIGHT, PREF_WINDOW_REAL_WIDTH, 500);
}

- (void) drawRect:(NSRect)dirtyRect
{
	[[NSColor grayColor] setFill];
	NSRectFill(dirtyRect);
}

@end
