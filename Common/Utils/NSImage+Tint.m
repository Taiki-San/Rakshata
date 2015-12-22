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
 ********************************************************************************************/

@implementation RakImage (Tint)

//Come from http://stackoverflow.com/questions/1413135/tinting-a-grayscale-RakImage-or-ciimage
- (void) tintWithColor : (RakColor *) color
{
	NSSize size = self.size;
	
	if (color != nil && !NSEqualSizes(size, NSZeroSize))
	{
		NSRect imageRect = {NSZeroPoint, size};
		
		[self lockFocus];
		
		[color set];
		NSRectFillUsingOperation(imageRect, NSCompositeSourceAtop);
		
		[self unlockFocus];
	}
#ifdef EXTENSIVE_LOGGING
	else if(color != nil)
	{
		NSLog(@"Hey, trying to tint an empty image, WTF?");
	}
#endif
}

@end
