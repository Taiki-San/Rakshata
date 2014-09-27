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

@implementation RakText

- (id) init
{
	self = [super init];
	
	if(self != nil)
	{
		[self internalInit];
	}
	
	return self;
}

- (id)initWithText:(NSRect)frame : (NSString *) text : (NSColor *) color
{
    self = [self initWithFrame:frame];
    if (self)
	{
		[self internalInit];
		self.stringValue = text;
		
		if(color != nil)
			self.textColor = color;
    }
    return self;
}

- (void) internalInit
{
	self.wantsLayer = NO;
	self.editable = NO;
	self.bordered = NO;
	self.wantCustomBorder = NO;
	self.forcedOffset = 0;
	self.drawsBackground = NO;
	self.backgroundColor = [NSColor clearColor];
	self.selectable = NO;
}

+ (Class) cellClass
{
	return [RakTextCell class];
}

- (NSColor *) getBorderColor
{
	return [Prefs getSystemColor:GET_COLOR_CLICKABLE_TEXT :nil];
}

- (void) additionalDrawing
{
	if(self.wantCustomBorder)
	{
		NSBezierPath * path = [NSBezierPath bezierPathWithRect : self.bounds];
		[path setLineWidth:1];
		[[self getBorderColor] setStroke];
		[path stroke];
	}	
}

- (void) setFrameSize:(NSSize)newSize
{
	if(self.forcedOffset)
	{
		newSize.height += self.forcedOffset;
	}
	
	[super setFrameSize:newSize];
}

- (void) setFrameOrigin:(NSPoint)newOrigin
{
	if(self.forcedOffset)
	{
		newOrigin.y -= self.forcedOffset;
	}
	
	[super setFrameOrigin:newOrigin];
}

- (void) drawRect:(NSRect)dirtyRect
{
	if(self.drawsBackground)
	{
		[self.backgroundColor setFill];
		NSRectFill(dirtyRect);
	}
	
	[super drawRect:dirtyRect];

	[self additionalDrawing];
}

- (BOOL) isFlipped
{
	return NO;
}

- (void) dealloc
{
	if(self.superview != nil)
		[self removeFromSuperview];
}

@end

@implementation RakFormatterLength

- (instancetype) init
{
	return [self init : 0];
}

- (instancetype) init : (uint) length
{
	self = [super init];
	
	if(self != nil)
		self.maxLength = length;
	
	return self;
}

- (NSString *) stringForObjectValue : (id) object {	return (NSString *)object;	}
- (BOOL) getObjectValue : (id *) object forString : (NSString *) string errorDescription : (NSString **) error {	*object = string;	return YES;		}
- (NSAttributedString *) attributedStringForObjectValue : (id) anObject withDefaultAttributes : (NSDictionary *) attributes {	return nil;		}

- (BOOL)isPartialStringValid:(NSString **)partialStringPtr	proposedSelectedRange:(NSRangePointer)proposedSelRangePtr	originalString:(NSString *)origString	originalSelectedRange:(NSRange)origSelRange	errorDescription:(NSString **)error
{
	return self.maxLength == 0 || [*partialStringPtr length] <= self.maxLength;
}

@end

#define RADIUS 2

@implementation RakTextClickable

- (id) initWithText : (NSRect)frame : (NSString *) text : (NSColor *) color;
{
	self = [self initWithFrame:frame];
	
	if(self != nil)
	{
		RakText * content = [[RakText alloc] initWithText : frame : text : color];
		[content sizeToFit];
		
		[self setFrameSize:NSMakeSize(content.bounds.size.width + 2 * RADIUS, content.bounds.size.height + 2 * RADIUS)];
		[content setFrameOrigin:NSMakePoint(RADIUS, RADIUS)];

		[self addSubview:content];
	}
	
	return self;
}

- (void) setFrame:(NSRect)frameRect
{
	[super setFrame:frameRect];
	[self.subviews[0] setFrame:NSInsetRect(frameRect, RADIUS, RADIUS)];
}

- (void) drawRect:(NSRect)dirtyRect
{
	if(backgroundColor == nil)
		backgroundColor = [Prefs getSystemColor:GET_COLOR_BACKGROUND_BUTTON_UNSELECTED : self];
	
	[backgroundColor setFill];
	[[NSBezierPath bezierPathWithRoundedRect:self.bounds xRadius:RADIUS yRadius:RADIUS] fill];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] == [Prefs class])
	{
		backgroundColor = [Prefs getSystemColor:GET_COLOR_BACKGROUND_BUTTON_UNSELECTED : nil];
		[super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	}
}

- (void) mouseUp:(NSEvent *)theEvent
{
	if(self.URL != nil)
		[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:self.URL]];
}

@end
