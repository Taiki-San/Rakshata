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
	{
		NSSize textSize = [self cellSizeForBounds:theRect];		// Get our ideal size for current text
		
		double heightDelta = newRect.size.height - textSize.height;		// Center that in the proposed rect
		if (heightDelta > 0)
		{
			newRect.size.height -= heightDelta;
			newRect.origin.y += (heightDelta / 2);
		}
	}
	
	return newRect;
}

@end

@implementation RakTextCell

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
		
		self.font = [NSFont systemFontOfSize:13];
		self.alignment = NSCenterTextAlignment;
		
		if(color != nil)
			self.textColor = color;
		
		self.selectable = NO;
		self.customizedInjectionPoint = NO;
    }
    return self;
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
	
	if(self.customizedInjectionPoint)
		[output setInsertionPointColor:[Prefs getSystemColor:GET_COLOR_INSERTION_POINT :nil]];
	
	return output;
}

@end