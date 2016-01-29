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

@implementation RakCenteredTextFieldCell

- (NSRect)drawingRectForBounds:(NSRect)theRect
{
	NSRect newRect = [super drawingRectForBounds:theRect];	    // Get the parent's idea of where we should draw
	
	if(self.centered)
		newRect = [self centerCell : newRect];
	
	return newRect;
}

- (void) selectWithFrame:(NSRect)aRect inView:(RakView *)controlView editor:(NSText *)textObj delegate:(id)anObject start:(NSInteger)selStart length:(NSInteger)selLength
{
	if(self.centered)
		aRect = [self centerCell:aRect];
	
	[super selectWithFrame:aRect inView:controlView editor:textObj delegate:anObject start:selStart length:selLength];
}

- (void) editWithFrame:(NSRect)aRect inView:(RakView *)controlView editor:(NSText *)textObj delegate:(id)anObject event:(NSEvent *)theEvent
{
	if(self.centered)
		aRect = [self centerCell:aRect];
	
	[super editWithFrame:aRect inView:controlView editor:textObj delegate:anObject event:theEvent];
}

- (NSRect) centerCell : (NSRect) originalRect
{
	NSSize textSize = [self cellSizeForBounds:originalRect];		// Get our ideal size for current text
	
	double heightDelta = originalRect.size.height - textSize.height;		// Center that in the proposed rect
	if(heightDelta > 0)
	{
		originalRect.size.height -= heightDelta;
		originalRect.origin.y += (heightDelta / 2);
	}
	
	return originalRect;
}

@end

@implementation RakTextCell

- (instancetype) init
{
	self = [super init];
	
	if(self != nil)
	{
		self.centered = YES;
		self.customizedInjectionPoint = NO;
	}
	
	return self;
}

- (instancetype) initWithText : (NSString *) text : (RakColor *) color
{
	self = [self initTextCell:text];
	if(self != nil)
	{
		self.editable = NO;
		self.bordered = NO;
		self.centered = YES;
		self.drawsBackground = NO;
		self.backgroundColor = [RakColor clearColor];
		self.alignment = NSTextAlignmentCenter;
		
		if(color != nil)
			self.textColor = color;
		
		self.selectable = NO;
		self.customizedInjectionPoint = NO;
	}
	return self;
}

- (void) setBackgroundColor:(RakColor *)color
{
	[super setBackgroundColor:color];
	clearBackground = [color isEqualTo:[RakColor clearColor]];
}

- (void) highlight:(BOOL)flag withFrame:(NSRect)cellFrame inView:(RakView*)controlView
{
	if(flag && clearBackground)
		self.backgroundColor = [RakColor clearColor];
	
	[super highlight:flag withFrame:cellFrame inView:controlView];
}

- (NSText*) setUpFieldEditorAttributes : (NSText*) textObj
{
	NSTextView * output = (NSTextView*) [super setUpFieldEditorAttributes:textObj];
	
	if(self.customizedInjectionPoint)
		[output setInsertionPointColor:[Prefs getSystemColor:COLOR_INSERTION_POINT]];
	
	return output;
}

@end

/**		We have to duplicate the code from RakTextCell in order to use it with NSSecureTextFieldCell, must keep this duplicate as limited as possible		**/

@implementation RakPassFieldCell

- (NSRect)drawingRectForBounds:(NSRect)theRect
{
	return [self centerCell :[super drawingRectForBounds:theRect]];
}

- (void) selectWithFrame:(NSRect)aRect inView:(RakView *)controlView editor:(NSText *)textObj delegate:(id)anObject start:(NSInteger)selStart length:(NSInteger)selLength
{
	[super selectWithFrame:[self centerCell:aRect] inView:controlView editor:textObj delegate:anObject start:selStart length:selLength];
}

- (void) editWithFrame:(NSRect)aRect inView:(RakView *)controlView editor:(NSText *)textObj delegate:(id)anObject event:(NSEvent *)theEvent
{
	[super editWithFrame:[self centerCell:aRect] inView:controlView editor:textObj delegate:anObject event:theEvent];
}

- (NSRect) centerCell : (NSRect) originalRect
{
	NSSize textSize = [self cellSizeForBounds:originalRect];		// Get our ideal size for current text
	
	double heightDelta = originalRect.size.height - textSize.height;		// Center that in the proposed rect
	if(heightDelta > 0)
	{
		originalRect.size.height -= heightDelta;
		originalRect.origin.y += (heightDelta / 2);
	}
	
	return originalRect;
}

- (void) highlight:(BOOL)flag withFrame:(NSRect)cellFrame inView:(RakView*)controlView
{
	if(flag)
		self.backgroundColor = [RakColor clearColor];
	
	[super highlight:flag withFrame:cellFrame inView:controlView];
}

- (NSText*) setUpFieldEditorAttributes : (NSText*) textObj
{
	NSTextView * output = (NSTextView*) [super setUpFieldEditorAttributes:textObj];
	
	[output setInsertionPointColor:[Prefs getSystemColor:COLOR_INSERTION_POINT]];
	
	return output;
}

@end