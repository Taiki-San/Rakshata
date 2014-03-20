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

@implementation RakReaderBottomBar

#define RADIUS_BORDERS 13.0

- (id)init: (BOOL) displayed : (id) parent
{
	self = [self initView:parent];
	
	[self.layer setCornerRadius:RADIUS_BORDERS];
		
	if(!displayed)
		[self setHidden:![self isHidden]];
	
	return self;
}

- (void) setupPath
{
	NSSize selfSize = self.frame.size;
	
	contextBorder = [[NSGraphicsContext currentContext] graphicsPort];
	
	CGContextBeginPath(contextBorder);
	CGContextAddArc(contextBorder, RADIUS_BORDERS, selfSize.height / 2, RADIUS_BORDERS, -M_PI, M_PI_2, 1);
	CGContextAddLineToPoint(contextBorder, selfSize.width - RADIUS_BORDERS, selfSize.height);
	CGContextAddArc(contextBorder, selfSize.width - RADIUS_BORDERS, selfSize.height/2, RADIUS_BORDERS, M_PI_2, 0, 1);
}

- (NSColor*) getMainColor
{
	return [NSColor colorWithSRGBRed:20/255.0f green:20/255.0 blue:20/255.0 alpha:0.8];
}

- (NSColor*) getColorFront
{
	return [NSColor colorWithSRGBRed:75/255.0f green:75/255.0 blue:75/255.0 alpha:0.8];
}

- (void)drawRect:(NSRect)dirtyRect
{
	[super drawRect:dirtyRect];
	
	[self setupPath];
	[[self getColorFront] setStroke];
	CGContextStrokePath(contextBorder);
}

/*	Routines à overwrite	*/

- (void) setFrame:(NSRect)frameRect
{
	[super setFrame:frameRect];
}

/* Gestion des évènements*/
- (void)mouseDown:(NSEvent *)theEvent
{
	
}

- (void) readerIsOpening
{
	
}

- (void) applyRefreshSizeReaderChecks
{
	
}

/*Constraints routines*/

- (CGFloat) getRequestedViewPosX:(CGFloat) widthWindow
{
	return widthWindow / 2 - [self getRequestedViewWidth : widthWindow] / 2;
}

- (CGFloat) getRequestedViewPosY:(CGFloat) heightWindow
{
	return RD_CONTROLBAR_POSY;
}

- (CGFloat) getRequestedViewWidth:(CGFloat) widthWindow
{
	CGFloat output = widthWindow * RD_CONTROLBAR_WIDHT_PERC / 100;
	
	output = MIN(output, RD_CONTROLBAR_WIDHT_MAX);
	output = MAX(output, RD_CONTROLBAR_WIDHT_MIN);
	
	return output;
}

- (CGFloat) getRequestedViewHeight:(CGFloat) heightWindow
{
	return RD_CONTROLBAR_HEIGHT;
}

@end
