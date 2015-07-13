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

@implementation RakSheetView

- (void) drawRect:(NSRect)dirtyRect
{
	[_backgroundColor setFill];

	CGContextRef contextBorder = [[NSGraphicsContext currentContext] graphicsPort];

	NSSize currentSize = _bounds.size;
	const CGFloat radius = 2;

	CGContextBeginPath(contextBorder);
	CGContextMoveToPoint(contextBorder, 0, currentSize.height);
	CGContextAddLineToPoint(contextBorder, currentSize.width, currentSize.height);
	CGContextAddLineToPoint(contextBorder, currentSize.width, radius);
	CGContextAddArc(contextBorder, currentSize.width - radius, radius, radius, 0, -M_PI_2, 1);
	CGContextAddLineToPoint(contextBorder, radius, 0);
	CGContextAddArc(contextBorder, radius, radius, radius, M_PI_2, M_PI, 1);
	CGContextAddLineToPoint(contextBorder, 0, currentSize.height);

	CGContextFillPath(contextBorder);
}

@end
