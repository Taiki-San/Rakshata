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

@implementation RakCenteredTextFieldCell

- (NSRect)drawingRectForBounds:(NSRect)theRect
{
	NSRect newRect = [super drawingRectForBounds:theRect];	    // Get the parent's idea of where we should draw
	
	if(self.centered)
		newRect = [self centerCell : newRect];
	
	return newRect;
}

- (void) selectWithFrame:(NSRect)aRect inView:(NSView *)controlView editor:(NSText *)textObj delegate:(id)anObject start:(NSInteger)selStart length:(NSInteger)selLength
{
	if(self.centered)
		aRect = [self centerCell:aRect];
	
	[super selectWithFrame:aRect inView:controlView editor:textObj delegate:anObject start:selStart length:selLength];
}

- (void) editWithFrame:(NSRect)aRect inView:(NSView *)controlView editor:(NSText *)textObj delegate:(id)anObject event:(NSEvent *)theEvent
{
	if(self.centered)
		aRect = [self centerCell:aRect];
	
	[super editWithFrame:aRect inView:controlView editor:textObj delegate:anObject event:theEvent];
}

- (NSRect) centerCell : (NSRect) originalRect
{
	NSSize textSize = [self cellSizeForBounds:originalRect];		// Get our ideal size for current text
	
	double heightDelta = originalRect.size.height - textSize.height;		// Center that in the proposed rect
	if (heightDelta > 0)
	{
		originalRect.size.height -= heightDelta;
		originalRect.origin.y += (heightDelta / 2);
	}
	
	return originalRect;
}

@end

@implementation RakTextCell

- (id) init
{
	self = [super init];
	
	if(self != nil)
	{
		self.centered = YES;
		self.customizedInjectionPoint = NO;
	}
	
	return self;
}

- (id)initWithText : (NSString *) text : (NSColor *) color
{
	self = [self initTextCell:text];
    if (self)
	{
		self.editable = NO;
		self.bordered = NO;
		self.centered = YES;
		self.drawsBackground = NO;
		self.backgroundColor = [NSColor clearColor];
		self.alignment = NSCenterTextAlignment;
		
		if(color != nil)
			self.textColor = color;
		
		self.selectable = NO;
		self.customizedInjectionPoint = NO;
    }
    return self;
}

- (void) setBackgroundColor:(NSColor *)color
{
	[super setBackgroundColor:color];
	clearBackground = [color isEqualTo:[NSColor clearColor]];
}

- (void) highlight:(BOOL)flag withFrame:(NSRect)cellFrame inView:(NSView*)controlView
{
	if (flag && clearBackground)
		self.backgroundColor = [NSColor clearColor];
	
	[super highlight:flag withFrame:cellFrame inView:controlView];
}

- (NSText*) setUpFieldEditorAttributes : (NSText*) textObj
{
	NSTextView * output = (NSTextView*) [super setUpFieldEditorAttributes:textObj];
	
	if(self.customizedInjectionPoint)
		[output setInsertionPointColor:[Prefs getSystemColor:GET_COLOR_INSERTION_POINT :nil]];
	
	return output;
}

@end

/**		We have to duplicate the code from RakTextCell in order to use it with NSSecureTextFieldCell, must keep this duplicate as limited as possible		**/

@implementation RakPassFieldCell

- (NSRect)drawingRectForBounds:(NSRect)theRect
{
	return [self centerCell :[super drawingRectForBounds:theRect]];
}

- (void) selectWithFrame:(NSRect)aRect inView:(NSView *)controlView editor:(NSText *)textObj delegate:(id)anObject start:(NSInteger)selStart length:(NSInteger)selLength
{
	[super selectWithFrame:[self centerCell:aRect] inView:controlView editor:textObj delegate:anObject start:selStart length:selLength];
}

- (void) editWithFrame:(NSRect)aRect inView:(NSView *)controlView editor:(NSText *)textObj delegate:(id)anObject event:(NSEvent *)theEvent
{
	[super editWithFrame:[self centerCell:aRect] inView:controlView editor:textObj delegate:anObject event:theEvent];
}

- (NSRect) centerCell : (NSRect) originalRect
{
	NSSize textSize = [self cellSizeForBounds:originalRect];		// Get our ideal size for current text
	
	double heightDelta = originalRect.size.height - textSize.height;		// Center that in the proposed rect
	if (heightDelta > 0)
	{
		originalRect.size.height -= heightDelta;
		originalRect.origin.y += (heightDelta / 2);
	}
	
	return originalRect;
}

- (void) highlight:(BOOL)flag withFrame:(NSRect)cellFrame inView:(NSView*)controlView
{
	if (flag)
		self.backgroundColor = [NSColor clearColor];
	
	[super highlight:flag withFrame:cellFrame inView:controlView];
}

- (NSText*) setUpFieldEditorAttributes : (NSText*) textObj
{
	NSTextView * output = (NSTextView*) [super setUpFieldEditorAttributes:textObj];
	
	[output setInsertionPointColor:[Prefs getSystemColor:GET_COLOR_INSERTION_POINT :nil]];
	
	return output;
}

@end