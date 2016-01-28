/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 ********************************************************************************************/

@implementation RakImage (Tint)

//Come from http://stackoverflow.com/questions/1413135/tinting-a-grayscale-RakImage-or-ciimage
- (void) tintWithColor : (RakColor *) color
{
	if(color == nil)
	{
#ifdef EXTENSIVE_LOGGING
		NSLog(@"Hey, trying to tint an empty image, WTF?");
#endif
		return;
	}

	NSSize size = self.size;
	
	if (!NSEqualSizes(size, NSZeroSize))
	{
		NSRect imageRect = {NSZeroPoint, size};
		
		[self lockFocus];
		
		[color set];
		NSRectFillUsingOperation(imageRect, NSCompositeSourceAtop);
		
		[self unlockFocus];
	}
}

@end
