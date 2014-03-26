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
 *********************************************************************************************/

@implementation RakScroller

- (id) initWithFrame:(NSRect)frameRect
{
	self = [super initWithFrame:frameRect];
	
	if(self != nil)
	{
		passive = [[Prefs getSystemColor:GET_COLOR_INACTIVE] retain];
		active = [[Prefs getSystemColor:GET_COLOR_ACTIVE] retain];
		color = passive;
		[self setKnobStyle:NSScrollerKnobStyleLight];
	}
	
	return self;
}

- (void)drawKnobSlotInRect:(NSRect)slotRect highlight:(BOOL)flag
{
	slotRect.origin.x += slotRect.size.width / 4;
	slotRect.size.width /= 2;
	
	[[NSColor blackColor] set];
	NSRectFill(slotRect);
}

#define RADIUS_BORDERS	6.5f
#define BAR_WIDTH		(2 * RADIUS_BORDERS)

- (void) setupPath : (NSRect) selfRect
{
	contextBorder = [[NSGraphicsContext currentContext] graphicsPort];
	
	CGContextBeginPath(contextBorder);
	CGContextAddArc(contextBorder, selfRect.size.width / 2, selfRect.origin.y + selfRect.size.height - RADIUS_BORDERS, RADIUS_BORDERS, M_PI, 0, 1);
	CGContextAddLineToPoint(contextBorder, selfRect.size.width / 2 + BAR_WIDTH / 2, RADIUS_BORDERS);
	CGContextAddArc(contextBorder, selfRect.size.width / 2, selfRect.origin.y + RADIUS_BORDERS, RADIUS_BORDERS, 0, M_PI, 1);
	CGContextAddLineToPoint(contextBorder, selfRect.size.width / 2 - BAR_WIDTH / 2, selfRect.origin.y + selfRect.size.height - RADIUS_BORDERS);
}

- (NSColor*) getColorBar
{
	return color;
}

- (void)drawKnob
{
	NSRect knobRect = [self rectForPart:NSScrollerKnob];
	[self setupPath:knobRect];
	[[self getColorBar] setFill];
	CGContextFillPath(contextBorder);
}

- (void) mouseDown:(NSEvent *)theEvent
{
	color = active;
	[self setNeedsDisplay:YES];
	[super mouseDown:theEvent];
	color = passive;
	[self setNeedsDisplay:YES];
}

+ (void) updateScrollers : (NSScrollView *) view
{
	NSRect horizontalScrollerFrame = [[view horizontalScroller] frame];
	NSRect verticalScrollerFrame = [[view verticalScroller] frame];
	
	if ([view verticalScroller] != nil && ![[view verticalScroller] isKindOfClass:[RakScroller class]])
		[view setVerticalScroller:[[[RakScroller alloc] initWithFrame:verticalScrollerFrame] autorelease]];
	
	if ([view horizontalScroller] != nil && ![[view horizontalScroller] isKindOfClass:[RakScroller class]])
		[view setHorizontalScroller:[[[RakScroller alloc] initWithFrame:horizontalScrollerFrame] autorelease]];
}

/*
 Code to move sliders according to an internal view move:
 
	selfFrame = la taille de l'élément complet
	readerView = la taille de la view
 
 if(pageTooLarge)
	self.horizontalScroller.floatValue = 1 - sliderStart.x / (selfFrame.size.width + 2 * READER_BORDURE_VERT_PAGE - frameReader.size.width);
 if(pageTooHigh)
	self.verticalScroller.floatValue = 1 - sliderStart.y / (selfFrame.size.height + READER_PAGE_BORDERS_HIGH - frameReader.size.height);
 */

@end