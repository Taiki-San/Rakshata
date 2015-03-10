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

@implementation RakClickableText

- (instancetype) initWithText:(NSString *) text : (NSColor *)color responder : (NSObject *) responder
{
	self = [super initWithText:text :color];
	
	if(self != nil)
	{
		self.font = [NSFont fontWithName:[Prefs getFontName:GET_FONT_STANDARD] size:13];
		[self sizeToFit];
		
		self.clicTarget = responder;
		self.clicAction = @selector(respondTo:);
	}
	
	return self;
}

- (void) viewDidMoveToSuperview
{
	[self updateTrackingAreas];
}

- (void) updateTrackingAreas
{
	[super updateTrackingAreas];
	
	if(tracking == 0)
		classicalTextColor = self.textColor;
	else
		[self removeTrackingRect:tracking];
	
	BOOL mouseInside = NSPointInRect([self convertPoint:[self.window mouseLocationOutsideOfEventStream] fromView:nil], _bounds);
	tracking = [self addTrackingRect:_bounds owner:self userData:NULL assumeInside:mouseInside];
	
	if(mouseInside)
		[self mouseEntered:nil];
	
	else if(self.textColor != classicalTextColor)
		[self mouseExited:nil];
}

- (void) mouseEntered : (NSEvent *) theEvent
{
	if(_URL != nil)
		self.textColor = [self focusTextColor];
}

- (void) mouseExited : (NSEvent *) theEvent
{
	if(_URL != nil)
		self.textColor = classicalTextColor;
}

- (void) mouseDown:(NSEvent *)theEvent
{
	if(_URL != nil)
		[super mouseDown:theEvent];
	else
		[self overrideMouseUp:theEvent];
}

- (void) mouseUp : (NSEvent *) theEvent
{
	if(_URL != nil)
		[super mouseUp:theEvent];
	else
		[self overrideMouseUp:theEvent];
}

#pragma mark - Color

- (NSColor *) focusTextColor
{
	return [Prefs getSystemColor:GET_COLOR_ACTIVE :nil];
}

@end
