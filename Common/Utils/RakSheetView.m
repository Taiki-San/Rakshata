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

#define BORDER_OFFSET 0.5f

@implementation RakSheetView

- (void) drawRect:(NSRect)dirtyRect
{
	[_backgroundColor setFill];
	[[[RakColor whiteColor] colorWithAlphaComponent:0.175] setStroke];

	CGContextRef contextBorder = [[NSGraphicsContext currentContext] graphicsPort];

	NSSize currentSize = _bounds.size;
	const CGFloat radius = 2;

	//Background
	CGContextBeginPath(contextBorder);
	CGContextMoveToPoint(contextBorder, 0, currentSize.height);
	CGContextAddLineToPoint(contextBorder, currentSize.width, currentSize.height);
	CGContextAddLineToPoint(contextBorder, currentSize.width, radius);
	CGContextAddArc(contextBorder, currentSize.width - radius, radius, radius, 0, -M_PI_2, 1);
	CGContextAddLineToPoint(contextBorder, radius, 0);
	CGContextAddArc(contextBorder, radius, radius, radius, -M_PI_2, -M_PI, 1);
	CGContextAddLineToPoint(contextBorder, 0, currentSize.height);
	CGContextFillPath(contextBorder);

	//Border
	CGContextBeginPath(contextBorder);
	CGContextMoveToPoint(contextBorder, currentSize.width - BORDER_OFFSET, currentSize.height);
	CGContextAddLineToPoint(contextBorder, currentSize.width - BORDER_OFFSET, radius + BORDER_OFFSET);
	CGContextAddArc(contextBorder, currentSize.width - radius - BORDER_OFFSET, radius + BORDER_OFFSET, radius, 0, -M_PI_2, 1);
	CGContextAddLineToPoint(contextBorder, radius + BORDER_OFFSET, BORDER_OFFSET);
	CGContextAddArc(contextBorder, radius + BORDER_OFFSET, radius + BORDER_OFFSET, radius, -M_PI_2, -M_PI, 1);
	CGContextAddLineToPoint(contextBorder, BORDER_OFFSET, currentSize.height);
	CGContextStrokePath(contextBorder);
}

@end

@implementation RakSheetWindow

- (BOOL) canBecomeKeyWindow
{
	return YES;
}

@end