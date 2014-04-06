/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite  **
 **                                                                                         **
 ********************************************************************************************/

@implementation RakPageCounter

- (id)init: (NSView*) superView : (CGFloat) posX : (uint) currentPageArg : (uint) pageMaxArg;
{
    self = [super initWithText:[superView bounds] :[NSString stringWithFormat:@"%d/%d", currentPageArg, pageMaxArg] :[self getColorBackground]];
    if (self)
	{
		[self setFont:[NSFont boldSystemFontOfSize:13]];
		[self updateSize:[superView bounds].size.height : posX];
		currentPage = currentPageArg;
		pageMax = pageMaxArg;
		[self setTextColor:[self getFontColor]];
    }
    return self;
}

- (void) updateContext
{
	[self setStringValue:[NSString stringWithFormat:@"%d/%d", currentPage, pageMax]];
	
	[self updateSize: self.superview.bounds.size.height : -1];

	[self setNeedsDisplay:YES];
}

#pragma mark - Size related

- (void) updateSize : (CGFloat) heightSuperView : (CGFloat) posX
{
	NSPoint origin;
	
	if(posX == -1)
	{
		CGFloat previousWidth = self.bounds.size.width, newWidth;
		
		[self sizeToFit];
		
		newWidth = self.bounds.size.width;
		origin.x = self.frame.origin.x - (newWidth - previousWidth) / 2;
	}
	else
	{
		[self sizeToFit];
		origin.x = posX - self.bounds.size.width / 2;
	}
	
	origin.y = heightSuperView / 2 - self.bounds.size.height / 2;
	
	[self setFrameOrigin:origin];
}

+ (Class)cellClass
{
	return [RakCenteredTextFieldCell class];
}

#pragma mark - Color stuffs

- (NSColor *) getColorBackground
{
	return [Prefs getSystemColor:GET_COLOR_READER_BAR_PAGE_COUNTER];
}

- (NSColor *) getBorderColor
{
	return [NSColor blackColor];
}

- (NSColor *) getFontColor
{
	return [Prefs getSystemColor:GET_COLOR_INACTIVE];//[NSColor whiteColor];
}

- (void) drawBorder
{
	CGContextRef contextBorder = [[NSGraphicsContext currentContext] graphicsPort];
	CGContextBeginPath(contextBorder);

	CGContextSetLineWidth(contextBorder, 2);
	[[self getBorderColor] setStroke];

	CGContextMoveToPoint(contextBorder, 0, self.bounds.size.height);
	CGContextAddLineToPoint(contextBorder, 0, 0);
	CGContextAddLineToPoint(contextBorder, self.bounds.size.width, 0);
	
	CGContextStrokePath(contextBorder);
}

- (void)drawRect:(NSRect)dirtyRect
{
    [super drawRect:dirtyRect];

	[self drawBorder];
}

#pragma mark - Setters/Getters

- (void) updatePage : (uint) newCurrentPage : (uint) newPageMax
{
	currentPage = newCurrentPage;
	pageMax = newPageMax;
	
	[self updateContext];
}

@end
