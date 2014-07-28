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
		self.wantsLayer = NO;
		self.editable = NO;
		self.bordered = NO;
		self.wantCustomBorder = NO;
		self.drawsBackground = NO;
		self.backgroundColor = [NSColor clearColor];
		self.selectable = NO;
	}
	
	return self;
}

- (id)initWithText:(NSRect)frame : (NSString *) text : (NSColor *) color
{
    self = [self initWithFrame:frame];
    if (self)
	{
		self.wantsLayer = NO;
		self.editable = NO;
		self.bordered = NO;
		self.wantCustomBorder = NO;
		self.drawsBackground = NO;
		self.backgroundColor = [NSColor clearColor];
		
		if(color != nil)
			self.textColor = color;
		
		self.stringValue = text;
		self.selectable = NO;
    }
    return self;
}

+ (Class) cellClass
{
	return [RakCenteredTextFieldCell class];
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

- (void) dealloc
{
	if(self.superview != nil)
		[self removeFromSuperview];
	[super dealloc];
}

#pragma mark - Apply changes

- (void) underline : (BOOL) underline
{
	NSMutableAttributedString *string = [[[self attributedStringValue] mutableCopy] autorelease];
	
	if(underline)
		[string addAttribute:NSUnderlineStyleAttributeName value:[NSNumber numberWithInt:NSUnderlineStyleSingle] range:NSMakeRange(0, string.length)];
	else
		[string removeAttribute:NSUnderlineStyleAttributeName range:NSMakeRange(0, string.length)];
	
	[self setAttributedStringValue : string];
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

