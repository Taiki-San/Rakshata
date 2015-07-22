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
	if(self != nil)
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
	
	if(self != nil)
	{
		[self internalInit];

		if(text != nil)
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
	_wantCustomBorder = NO;
	_forcedOffsetY = 0;
	self.drawsBackground = NO;
	self.backgroundColor = [NSColor clearColor];
	self.selectable = NO;
	
	[self.cell setWraps : NO];
}

- (void) viewDidChangeBackingProperties
{
	NSPoint origin = self.frame.origin;
	
	origin.y += _forcedOffsetY;
	
	[self setFrameOrigin:origin];
}

+ (Class) cellClass
{
	return [RakTextCell class];
}

- (NSColor *) getBorderColor
{
	return [Prefs getSystemColor:COLOR_CLICKABLE_TEXT :nil];
}

- (void) additionalDrawing
{
	if(self.wantCustomBorder)
	{
		NSBezierPath * path = [NSBezierPath bezierPathWithRect : _bounds];
		[path setLineWidth:1];
		[[self getBorderColor] setStroke];
		[path stroke];
	}
}

- (void) setFrameSize:(NSSize)newSize
{
	//Fix an issue when resizing with an animation
	if(_forcedOffsetY && newSize.height != _bounds.size.height)
		newSize.height += _forcedOffsetY;
	
	_suggestedWidth = newSize.width;
	_currentFrame.size = _discardHeight ? [self intrinsicContentSize] : newSize;
	
	[super setFrameSize: _currentFrame.size];
}

- (void) setFrameOrigin:(NSPoint)newOrigin
{
	if(_forcedOffsetY)
		newOrigin.y -= _forcedOffsetY;
	
	//On non-retina display, the text will be blurry if not on a round origin relative to the window, yeah, awesome...
	if(self.window == nil)
	{
		if([[(RakAppDelegate *) [NSApp delegate] window] backingScaleFactor] == 1)
		{
			NSRect rectOnScreen = [[(RakAppDelegate *) [NSApp delegate] window] convertRectToScreen: [self.superview convertRect:(NSRect) {newOrigin, NSZeroSize} toView:nil]];
			
			newOrigin.x += roundf(rectOnScreen.origin.x) - rectOnScreen.origin.x;
			newOrigin.y += roundf(rectOnScreen.origin.y) - rectOnScreen.origin.y;
		}
	}
	else if([self.window backingScaleFactor] == 1)
	{
		NSRect rectOnScreen = [self.window convertRectToScreen: [self.superview convertRect:(NSRect) {newOrigin, NSZeroSize} toView:nil]];
		
		newOrigin.x += roundf(rectOnScreen.origin.x) - rectOnScreen.origin.x;
		newOrigin.y += roundf(rectOnScreen.origin.y) - rectOnScreen.origin.y;
	}
	
	[super setFrameOrigin: (_currentFrame.origin = newOrigin)];
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
	
	if([self.cell wraps])
		[self setFrameSize: _discardHeight ? NSZeroSize : [self intrinsicContentSize]];
}

- (void) setObjectValue:(id<NSCopying>)obj
{
	if([(NSObject *) obj isKindOfClass:[NSString class]])
		[self setStringValue: (NSString *) obj];
	else
		[super setObjectValue:obj];
}

- (void) dealloc
{
	if(self.superview != nil)
		[self removeFromSuperview];

	[[NSNotificationCenter defaultCenter] removeObserver:self];
}

#pragma mark - Handle wrap lines properly

- (void) setFixedWidth:(CGFloat)fixedWidth
{
	haveFixedWidth = fixedWidth != 0;
	if(_fixedWidth != fixedWidth)
	{
		_fixedWidth = fixedWidth;
		
		if([self.cell wraps] && ![self.stringValue isEqualToString:@""])
			[self setFrameSize:[self intrinsicContentSize]];
	}
}

- (CGFloat) fixedWidth
{
	return _fixedWidth;
}

- (NSSize) intrinsicContentSize
{
	if(![self.cell wraps])
		return [super intrinsicContentSize];
	
	NSRect frame = NSZeroRect;
	
	if(haveFixedWidth)
		frame.size.width = _fixedWidth;
	else if(_bounds.size.width != 0)
		frame.size.width = _bounds.size.width;
	else
		frame.size.width = _suggestedWidth;
	
	// Make the frame very high, while keeping the width
	frame.size.height = CGFLOAT_MAX;

	// Calculate new height within the frame with practically infinite height.
	return NSMakeSize(frame.size.width, [self.cell cellSizeForBounds: frame].height);
}

- (void) setEnableMultiLine:(BOOL)enableMultiLine
{
	_enableMultiLine = enableMultiLine;

	if(!self.cell.wraps)
		NSLog(@"Eh, I need a max width :/");

	self.delegate = self;
}

- (BOOL)control:(NSControl*)control textView:(NSTextView*)textView doCommandBySelector:(SEL)commandSelector;
{
	BOOL result = NO;

	if(_enableMultiLine)
	{
		if (commandSelector == @selector(insertNewline:))
		{
			// new line action:
			// always insert a line-break character and don’t cause the receiver to end editing
			[textView insertNewlineIgnoringFieldEditor:self];
			result = YES;
		}
		else if (commandSelector == @selector(insertTab:))
		{
			// tab action:
			// always insert a tab character and don’t cause the receiver to end editing
			[textView insertTabIgnoringFieldEditor:self];
			result = YES;
		}

		if(result)
			[self updateMultilineHeight];
	}

	return result;
}

- (void)controlTextDidChange:(NSNotification *)notification
{
	if(_enableMultiLine)
		[self updateMultilineHeight];
}

- (void) updateMultilineHeight
{
	[self validateEditing];

	NSSize size = [self intrinsicContentSize];
	NSRect frame = _frame;

	if(!NSEqualSizes(size, frame.size))
	{
		CGFloat delta = size.height - frame.size.height;
		frame.size = size;

		[self setFrameSize:size];


		NSScrollView * scrollview = (id) self.superview.superview;
		if([scrollview isKindOfClass:[NSScrollView class]])
		{
			frame.origin = [[scrollview contentView] bounds].origin;

			if(self.superview.isFlipped)
				frame.origin.y += delta;
			else
				frame.origin.y -= delta;

			[[scrollview documentView] scrollPoint:frame.origin];
		}
		else
		{
			if(self.superview.isFlipped)
				frame.origin.y += delta;
			else
				frame.origin.y -= delta;

			[self setFrameOrigin:frame.origin];
		}
	}
}

#pragma mark - Handle clic

- (BOOL) acceptsFirstMouse:(nullable NSEvent *)theEvent
{
	return YES;
}

- (BOOL) acceptsFirstResponder
{
	return YES;
}

- (void) overrideMouseDown : (NSEvent *) theEvent
{
	[super mouseDown:theEvent];
}

- (void) mouseDown:(NSEvent *)theEvent
{
	if(self.clicTarget == nil)
		[super mouseDown:theEvent];
}

- (void) overrideMouseUp : (NSEvent *) theEvent
{
	[super mouseUp:theEvent];
}

- (void) mouseUp : (NSEvent *) theEvent
{
	if(self.clicTarget != nil)
	{
		if([self.clicTarget respondsToSelector:self.clicAction])
		{
			IMP imp = [self.clicTarget methodForSelector:self.clicAction];
			void (*func)(id, SEL, id) = (void *)imp;
			func(self.clicTarget, self.clicAction, self);
		}
	}
	else
		[super mouseUp:theEvent];
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
