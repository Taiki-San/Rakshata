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

- (instancetype) init
{
	self = [super init];
	
	if(self != nil)
	{
		[self internalInit];
	}
	
	return self;
}

- (instancetype) initWithText:(NSRect)frame : (NSString *) text : (NSColor *) color
{
	self = [self initWithFrame : frame];
	if (self != nil)
	{
		[self internalInit];
		self.stringValue = text;
		
		if(color != nil)
			self.textColor = color;
	}
	return self;
}

- (instancetype) initWithText : (NSString *) text : (NSColor *) color
{
	self = [super init];
	if (self != nil)
	{
		[self internalInit];
		self.stringValue = text;
		[self sizeToFit];
		
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
	
	[self.cell setWraps : NO];
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
	//Fix an issue when resizing with an animation
	if(self.forcedOffset && newSize.height != self.bounds.size.height)
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

- (void) setStringValue : (NSString *)aString
{
	if(aString == nil)
	{
#ifdef DEV_VERSION
		NSLog(@"Hum, tried to set a nil string in RakText...");
#endif
		aString = @"";
	}
	[super setStringValue : aString];
	
	if ([self.cell wraps])
		[self setFrameSize:[self intrinsicContentSize]];
}

- (void) dealloc
{
	if(self.superview != nil)
		[self removeFromSuperview];
}

#pragma mark - Handle wrap lines properly

- (void) setFixedWidth:(CGFloat)fixedWidth
{
	haveFixedWidth = fixedWidth != 0;
	if(_fixedWidth != fixedWidth)
	{
		_fixedWidth = fixedWidth;
		
		if ([self.cell wraps] && ![self.stringValue isEqualToString:@""])
			[self setFrameSize:[self intrinsicContentSize]];
	}
}

- (CGFloat) fixedWidth
{
	return _fixedWidth;
}

- (NSSize) intrinsicContentSize
{
	if (![self.cell wraps])
		return [super intrinsicContentSize];
	
	NSRect frame = [self frame];
	
	if(haveFixedWidth)
		frame.size.width = _fixedWidth;
	
	// Make the frame very high, while keeping the width
	frame.size.height = CGFLOAT_MAX;
	
	// Calculate new height within the frame with practically infinite height.
	CGFloat height = [self.cell cellSizeForBounds: frame].height;
	
	return NSMakeSize(frame.size.width, height);
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
