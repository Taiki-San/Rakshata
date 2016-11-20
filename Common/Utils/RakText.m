/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at https://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
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

- (instancetype) initWithText:(NSRect)frame : (NSString *) text : (RakColor *) color
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

- (instancetype) initWithText : (NSString *) text : (RakColor *) color
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
	self.delegate = self;
	self.wantsLayer = NO;
	self.editable = NO;
	self.bordered = NO;
	_wantCustomBorder = NO;
	_forcedOffsetY = 0;
	self.drawsBackground = NO;
	self.backgroundColor = [RakColor clearColor];
	self.selectable = NO;

	if([self respondsToSelector:@selector(setAllowsDefaultTighteningForTruncation:)])
		[self setAllowsDefaultTighteningForTruncation:YES];

	CONFIGURE_APPEARANCE_DARK(self);

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

- (RakColor *) getBorderColor
{
	return _customBorderColor != nil ? _customBorderColor : [Prefs getSystemColor:COLOR_CLICKABLE_TEXT];
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
	
	NSRect rectOnScreen;
	CGFloat factor;
	
	//The text will be blurry if not on a round origin relative to the display, yeah, awesome...
	if(self.window == nil)
	{
		if([RakRealApp respondsToSelector:@selector(window)])
		{
			NSWindow * window = [RakRealApp performSelector:@selector(window)];
			if(window != nil)
			{
				rectOnScreen = [window convertRectToScreen: [self.superview convertRect:(NSRect) {newOrigin, NSZeroSize} toView:nil]];
				factor = [window backingScaleFactor];
			}
		}
	}
	else
	{
		rectOnScreen = [self.window convertRectToScreen: [self.superview convertRect:(NSRect) {newOrigin, NSZeroSize} toView:nil]];
		factor = [self.window backingScaleFactor];
	}
	
	if(factor == 0)	factor = 1;

	newOrigin.x += roundf(rectOnScreen.origin.x * factor) / factor - rectOnScreen.origin.x;
	newOrigin.y += roundf(rectOnScreen.origin.y * factor) / factor - rectOnScreen.origin.y;
	
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
#ifdef EXTENSIVE_LOGGING
		NSLog(@"Hum, tried to set a nil string in RakText...");
#endif
		aString = @"";
	}
	
	[super setStringValue : aString];
	
	if([self enableWraps])
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

- (void) setEnableWraps : (BOOL) canWrap
{
	[self.cell setWraps:canWrap];
}

- (BOOL) enableWraps
{
	return [self.cell wraps];
}

- (void) setFixedWidth : (CGFloat)fixedWidth
{
	haveFixedWidth = fixedWidth != 0;
	if(_fixedWidth != fixedWidth || _bounds.size.width != fixedWidth)
	{
		_fixedWidth = fixedWidth;
		
		if([self enableWraps] && ![self.stringValue isEqualToString:@""])
			[self setFrameSize:[self intrinsicContentSize]];
	}
}

- (void) sizeToFit
{
	if(haveFixedWidth)
		[self setFrameSize:[self intrinsicContentSize]];
	else
		[super sizeToFit];
}

- (NSSize) intrinsicContentSize
{
	if(![self enableWraps])
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
	else if ((commandSelector == @selector(noop:)) && [self isCommandEnterEvent:[RakRealApp currentEvent]])
	{
		[self.nextResponder keyDown:[RakRealApp currentEvent]];
		result = YES;
	}
	
	return result;
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


		NSScrollView * scrollview = (NSScrollView *) self.superview.superview;
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
	if(_callbackOnClic != nil)
	{
		_callbackOnClic(self);
	}
	else if(self.clicTarget != nil)
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

#pragma mark - Workarounds

- (BOOL) textShouldEndEditing:(NSText *)textObject
{
	BOOL initialRetValue = [super textShouldEndEditing:textObject];

	//In the case we override the formatter to support , AND ., we may be prevented to stop editing if the selection contains a ,/.
	if(!initialRetValue && self.formatter != nil)
	{
		if([self.formatter isKindOfClass:[NSNumberFormatter class]] && [self.formatter class] != [NSNumberFormatter class])
			return YES;
	}

	return initialRetValue;
}

#pragma mark - Delegate responder

- (void) textDidChange:(nonnull NSNotification *)obj
{
	BOOL didComplete;
	
	//Auto completion
	if(_wantCompletion && !autoCompleting)
	{
		autoCompleting = YES;
		[obj.object complete:nil];
		autoCompleting = NO;
		didComplete = YES;
	}
	else
		didComplete = NO;

	if(_enableMultiLine)
		[self updateMultilineHeight];

	if(self.callbackOnChange != nil)
	{
		//-updateMultilineHeight call this method that commit the text changes, if it wasn't called, we must do it ourselves
		if(!_enableMultiLine)
			[self validateEditing];

		self.callbackOnChange(obj, didComplete);
	}
}

#pragma mark - Cmd+Enter manager

- (void) noop : (id) yef {}

- (BOOL) isCommandEnterEvent : (NSEvent *) event
{
	return ([event modifierFlags] & NSEventModifierFlagCommand) != 0 && [[event charactersIgnoringModifiers] characterAtIndex:0] == NSCarriageReturnCharacter;
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
