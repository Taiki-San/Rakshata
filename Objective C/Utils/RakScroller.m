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

@implementation RakScroller

- (id) initWithFrame:(NSRect)frameRect
{
	self = [super initWithFrame:frameRect];
	
	if(self != nil)
	{
		incompleteDrawing = NO;
		
		passive = [[Prefs getSystemColor:GET_COLOR_INACTIVE:self] retain];
		active = [[Prefs getSystemColor:GET_COLOR_ACTIVE:nil] retain];
		color = passive;
		[self setKnobStyle:NSScrollerKnobStyleLight];
	}
	
	return self;
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return;

	bool isActive = color == active;
	
	[passive release];		passive = [[Prefs getSystemColor:GET_COLOR_INACTIVE:nil] retain];
	[active release];		active = [[Prefs getSystemColor:GET_COLOR_ACTIVE:nil] retain];

	if(isActive)	color = active;
	else			color = passive;
	
	[self setNeedsDisplay:YES];
}

- (void)drawKnobSlotInRect:(NSRect)slotRect highlight:(BOOL)flag
{
	slotRect.origin.x += slotRect.size.width / 4;
	slotRect.size.width /= 2;
	
	slotRect.size.height -= 20;
	slotRect.origin.y += 10;
	
	[self setupPath:slotRect : slotRect.size.width : slotRect.size.width / 2.0f];

	[[NSColor blackColor] set];
	
	CGContextFillPath(contextBorder);
	
	incompleteDrawing = !incompleteDrawing;
}

#define RADIUS_BORDERS	6.5f
#define BAR_WIDTH		(2 * RADIUS_BORDERS)

- (void) setupPath : (NSRect) selfRect : (CGFloat) barWidth : (CGFloat) radius
{
	contextBorder = [[NSGraphicsContext currentContext] graphicsPort];
	
	CGContextBeginPath(contextBorder);
	CGContextAddArc(contextBorder, selfRect.origin.x + selfRect.size.width / 2, selfRect.origin.y + selfRect.size.height - radius, radius, M_PI, 0, 1);
	CGContextAddLineToPoint(contextBorder, selfRect.origin.x + selfRect.size.width / 2 + barWidth / 2, radius);
	CGContextAddArc(contextBorder, selfRect.origin.x + selfRect.size.width / 2, selfRect.origin.y + radius, radius, 0, M_PI, 1);
	CGContextAddLineToPoint(contextBorder, selfRect.origin.x + selfRect.size.width / 2 - barWidth / 2, selfRect.origin.y + selfRect.size.height - radius);
}

- (NSColor*) getColorBar
{
	return color;
}

- (void)drawKnob
{
	NSRect knobRect = [self rectForPart:NSScrollerKnob];
	[self setupPath:knobRect : BAR_WIDTH : RADIUS_BORDERS];
	[[self getColorBar] setFill];
	CGContextFillPath(contextBorder);
	
	incompleteDrawing = !incompleteDrawing;
}

- (void) drawRect:(NSRect)dirtyRect
{
	[super drawRect:dirtyRect];
	
	//Fix a weird bug, when the slot would be drawn, but not the knob
	if(incompleteDrawing)
	{
		incompleteDrawing = NO;

		if([self.superview class] == [RakListScrollView class])
		{
			RakListScrollView * view = (RakListScrollView*) self.superview;

			if(self == [view horizontalScroller])
				[view setHasHorizontalScroller:NO];
			else if(self == [view verticalScroller])
				[view setHasVerticalScroller:NO];
		}
	}
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