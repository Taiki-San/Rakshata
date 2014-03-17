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

- (id)init: (BOOL) displayed : (id) parent
{
	self = [self setUpView:parent];
	
	if(!displayed)
		[self setHidden:![self isHidden]];
	
	return self;
}

- (NSColor*) getMainColor
{
	return [NSColor colorWithSRGBRed:62/255.0f green:62/255.0 blue:62/255.0 alpha:1.0];
}

- (void)drawRect:(NSRect)dirtyRect
{
	NSRect rect = NSMakeRect([self bounds].origin.x + 3, [self bounds].origin.y + 3, [self bounds].size.width - 6, [self bounds].size.height - 6);
	
	roundBorders = [NSBezierPath bezierPathWithRoundedRect:rect xRadius:10.0 yRadius:10.0];
	[roundBorders addClip];
	[super drawRect:dirtyRect];
}

/*	Routines à overwrite	*/

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
