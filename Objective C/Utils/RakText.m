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

	if([self respondsToSelector:@selector(setAllowsDefaultTighteningForTruncation:)])
		[self setAllowsDefaultTighteningForTruncation:YES];

//	if(floor(NSAppKitVersionNumber) >= NSAppKitVersionNumber10_10)
//		self.appearance = [NSAppearance appearanceNamed:NSAppearanceNameVibrantDark];

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

- (void) dealloc
{
	if(self.superview != nil)
		[self removeFromSuperview];

	[[NSNotificationCenter defaultCenter] removeObserver:self];
}

#pragma mark - Setters Hook

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

- (void) setFont:(NSFont * _Nullable)font
{
	_cachedMinHeight = 0;
	return [super setFont:font];
}

- (void) setMaxLength:(CGFloat)maxLength
{
	[self setFormatter:[[RakFormatterLength alloc] init : maxLength]];
	_maxLength = maxLength;
}

#pragma mark - Handle wrap lines properly

- (void) setFixedWidth : (CGFloat)fixedWidth
{
	haveFixedWidth = fixedWidth != 0;
	if(_fixedWidth != fixedWidth)
	{
		_fixedWidth = fixedWidth;
		
		if([self.cell wraps] && ![self.stringValue isEqualToString:@""])
			[self setFrameSize:[self intrinsicContentSize]];
	}
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
	frame.size = NSMakeSize(frame.size.width, [self.cell cellSizeForBounds: frame].height);

	if(frame.size.height < [self getMinHeight])
	{
		//We need to compute the height we would need
		frame.size.height = [self getMinHeight];
	}

	return frame.size;
}

- (CGFloat) getMinHeight
{
	if(_cachedMinHeight != 0)
		return _cachedMinHeight;

	NSTextStorage * textStorage = [[NSTextStorage alloc] initWithString:@"Gravitation"];
	NSTextContainer * textContainer = [[NSTextContainer alloc] initWithSize:NSMakeSize(CGFLOAT_MAX, CGFLOAT_MAX)];
	NSLayoutManager * layoutManager = [[NSLayoutManager alloc] init];
	[layoutManager addTextContainer: textContainer];

	[textStorage addLayoutManager:layoutManager];

	if(self.attributedStringValue != nil && [self.attributedStringValue length] > 0)
		[textStorage addAttributes:[self.attributedStringValue attributesAtIndex:0 effectiveRange:NULL] range:NSMakeRange(0, [textStorage length])];
	else
		[textStorage addAttribute:NSFontAttributeName value:self.font range:NSMakeRange(0, [textStorage length])];

	[textContainer setLineFragmentPadding:0];

	[layoutManager glyphRangeForTextContainer:textContainer];

	return _cachedMinHeight = [layoutManager usedRectForTextContainer:textContainer].size.height;
}

- (void) setEnableMultiLine:(BOOL)enableMultiLine
{
	_enableMultiLine = enableMultiLine;

	if(!self.cell.wraps)
		NSLog(@"Eh, I need a max width :/");

	self.delegate = self;
}

- (BOOL) textView:(nonnull NSTextView *)textView doCommandBySelector:(nonnull SEL)commandSelector
{
	return [self control:self textView:textView doCommandBySelector:commandSelector];
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

		if(result)
			[self updateMultilineHeight];
	}
	else if(_wantCompletion)
	{
		//Prevent the auto completion from messing with deleting chars
		if(commandSelector == @selector(deleteBackward:))
		{
			autoCompleting = YES;
			[textView deleteBackward:self];
			autoCompleting = NO;
			result = YES;
		}

		else if(commandSelector == @selector(deleteForward:))
		{
			autoCompleting = YES;
			[textView deleteForward:self];
			autoCompleting = NO;
			result = YES;
		}
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

#pragma mark - Completion is required

- (void) textDidChange:(nonnull NSNotification *)obj
{
	if(_wantCompletion && !autoCompleting)
	{
		autoCompleting = YES;
		[obj.object complete:nil];
		autoCompleting = NO;
	}
}

//By default, we complete with project names
- (NSArray<NSString *> *)textView:(NSTextView *)textView completions:(NSArray<NSString *> *)words forPartialWordRange:(NSRange)charRange indexOfSelectedItem:(NSInteger *)index
{
	if(!_wantCompletion)
		return nil;

	else if(_completionCallback != nil)
	{
		IMP imp = [_completionCallback methodForSelector:_completionSelector];
		NSArray * (*func)(id, SEL, id) = (void *)imp;
		return func(_completionCallback, _completionSelector, nil);
	}

	uint nbElem;
	char ** output = getProjectNameStartingWith([textView.string UTF8String], &nbElem);

	if(output == NULL || nbElem == 0)
	{
		free(output);
		return @[];
	}

	NSMutableArray * array = [NSMutableArray array];

	for(uint i = 0; i < nbElem; i++)
	{
		[array addObject:[NSString stringWithUTF8String:output[i]]];
		free(output[i]);
	}
	free(output);

	return array;
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
