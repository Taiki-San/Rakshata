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

int launchApp(int argc, char *argv[])
{
	@autoreleasepool
	{
		return NSApplicationMain(argc, (const char **) argv);
	}
}

@implementation RakView (BackgroundColor)

- (RakColor *) backgroundColor
{
	return [NSColor colorWithCGColor:self.layer.backgroundColor];
}

- (void) setBackgroundColor : (RakColor *) color
{
	if(self.layer == nil)
		self.wantsLayer = YES;
	
	self.layer.backgroundColor = color.CGColor;
}

@end