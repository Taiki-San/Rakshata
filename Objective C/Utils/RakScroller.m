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

- (instancetype) initWithFrame:(NSRect)frameRect
{
	self = [super initWithFrame:frameRect];
	
	if(self != nil)
	{
		incompleteDrawing = NO;
		
		passive = [Prefs getSystemColor:COLOR_INACTIVE:self];
		active = [Prefs getSystemColor:COLOR_HIGHLIGHT:nil];
		color = passive;
		[self setKnobStyle:NSScrollerKnobStyleLight];
		[Prefs getPref : PREFS_GET_SCROLLER_STYLE : &scrollerStyle];
		
		if(scrollerStyle == SCROLLER_STYLE_THIN)
			_radiusBorders = 5.0;
		else
			_radiusBorders = 6.5;
	}
	
	return self;
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	
	BOOL isActive = color == active;
	
	passive = [Prefs getSystemColor:COLOR_INACTIVE:nil];
	active = [Prefs getSystemColor:COLOR_HIGHLIGHT:nil];
	
	if(isActive)	color = active;
	else			color = passive;
	
	[self setNeedsDisplay:YES];
}

- (void)drawKnobSlotInRect:(NSRect)slotRect highlight:(BOOL)flag
{
	if(scrollerStyle == SCROLLER_STYLE_THIN)
	{
		slotRect.origin.x += slotRect.size.width / 3;
		slotRect.size.width /= 3;
	}
	else
	{
		slotRect.origin.x += slotRect.size.width / 4;
		slotRect.size.width /= 2;
	}
	
	slotRect.size.height -= 20;
	slotRect.origin.y += 10;
	
	[self setupPath:slotRect : slotRect.size.width : slotRect.size.width / 2.0f];
	
	[[NSColor blackColor] set];
	
	CGContextFillPath(contextBorder);
	
	incompleteDrawing = !incompleteDrawing;
}

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
	if(!_hideScroller)
	{
		NSRect knobRect = [self rectForPart:NSScrollerKnob];
		[self setupPath : knobRect : 2 * _radiusBorders : _radiusBorders];
		[[self getColorBar] setFill];
		CGContextFillPath(contextBorder);
	}

	incompleteDrawing = !incompleteDrawing;
}

- (void) drawRect:(NSRect)dirtyRect
{
	if(_backgroundColorToReplicate && ![_backgroundColorToReplicate isEqual:[NSColor clearColor]])
	{
		[_backgroundColorToReplicate setFill];
		NSRectFill(dirtyRect);
	}

	if(_hideScroller)
		return;

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
	
	if([view verticalScroller] != nil && ![[view verticalScroller] isKindOfClass:[RakScroller class]])
		[view setVerticalScroller:[[RakScroller alloc] initWithFrame:verticalScrollerFrame]];
	
	if([view horizontalScroller] != nil && ![[view horizontalScroller] isKindOfClass:[RakScroller class]])
		[view setHorizontalScroller:[[RakScroller alloc] initWithFrame:horizontalScrollerFrame]];
}

+ (CGFloat) width
{
	byte scrollerStyle;
	[Prefs getPref : PREFS_GET_SCROLLER_STYLE : &scrollerStyle];
	
	if(scrollerStyle == SCROLLER_STYLE_THIN)
		return 2 * 5.0;
	
	return 2 * 6.5;
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