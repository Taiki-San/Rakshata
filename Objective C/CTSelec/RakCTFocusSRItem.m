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

@implementation RakCTFocusSRItem

- (void) initContent
{
	NSImage * image = [self loadImage];
	if(image != nil)
	{
		thumbnail = [[NSImageView alloc] initWithFrame: (NSRect) {{0,0}, [self thumbSize]}];
		if(thumbnail != nil)
		{
			thumbnail.image = image;
			[self addSubview:thumbnail];
		}
	}
}

- (NSSize) defaultWorkingSize
{
	return NSMakeSize(RCVC_MINIMUM_WIDTH, RCVC_MINIMUM_HEIGHT);
}

@end
