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

@implementation RakAboutContent

- (void)drawRect:(NSRect)dirtyRect
{
	if(_haveAdditionalDrawing)
		_additionalDrawing(_bounds.size);
}

@end

@implementation RakAboutText

- (instancetype) initWithText:(NSString *) text : (NSColor *)color responder : (RakAboutWindow *) responder
{
	self = [super initWithText:text :color];
	
	if(self != nil)
	{
		self.font = [NSFont fontWithName:[Prefs getFontName:GET_FONT_STANDARD] size:13];
		[self sizeToFit];
		
		self.target = responder;
		self.action = @selector(respondTo:);
	}
	
	return self;
}

- (void) setupArea
{
	classicalTextColor = self.textColor;
	
	BOOL mouseInside = NSPointInRect([self convertPoint:[self.window mouseLocationOutsideOfEventStream] fromView:nil], _bounds);
	tracking = [self addTrackingRect:_bounds owner:self userData:nil assumeInside:mouseInside];
	
	if(mouseInside)
		[self mouseEntered:nil];
}

- (void) mouseEntered : (NSEvent *) theEvent
{
	self.textColor = [self focusTextColor];
}

- (void) mouseExited : (NSEvent *) theEvent
{
	self.textColor = classicalTextColor;
}

- (void) mouseUp : (NSEvent *) theEvent
{
	if(_target != nil)
	{
		if([_target respondsToSelector:_action])
		{
			IMP imp = [_target methodForSelector:_action];
			void (*func)(id, SEL, id) = (void *)imp;
			func(_target, _action, self);

		}
	}
	else
		[super mouseUp:theEvent];
}

#pragma mark - Color

- (NSColor *) focusTextColor
{
	return [Prefs getSystemColor:GET_COLOR_ACTIVE :nil];
}

@end