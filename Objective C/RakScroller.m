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

- (void)highlight:(BOOL)flag
{
	if(flag)
		color = [NSColor greenColor];
	else
		color = [NSColor redColor];
}

- (NSScrollerKnobStyle) knobStyle
{
	return [super knobStyle];
}

- (void)drawKnob
{
	[super drawKnob];
/*	NSRect knobRect = [self rectForPart:NSScrollerKnob];
	[[NSColor greenColor] set];
	[NSBezierPath fillRect:knobRect];*/
}

- (void)drawRect:(NSRect)dirtyRect
{
    [[NSColor clearColor] set];
    NSRectFill(dirtyRect);
	
	[self setKnobStyle:NSScrollerKnobStyleDark];
	
    [self drawKnob];
}

/*
 - (void) updateScrollers
 {
 NSRect horizontalScrollerFrame = [[self horizontalScroller] frame];
 NSRect verticalScrollerFrame = [[self verticalScroller] frame];
 
 if ([self verticalScroller] != nil && ![[self verticalScroller] isKindOfClass:[RakScroller class]])
 [self setVerticalScroller:[[[RakScroller alloc] initWithFrame:verticalScrollerFrame] autorelease]];
 
 if ([self horizontalScroller] != nil && ![[self horizontalScroller] isKindOfClass:[RakScroller class]])
 [self setHorizontalScroller:[[[RakScroller alloc] initWithFrame:horizontalScrollerFrame] autorelease]];
 }
 
*/

@end