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
	self = [super init];
	
	if(self != nil)
	{
		self.editable = NO;
		self.stringValue = text;
		self.textColor = color;
		
		self.font = [NSFont fontWithName:[Prefs getFontName:GET_FONT_STANDARD] size:13];
		[self sizeToFit];
		
		self.backgroundColor = [NSColor clearColor];
		[self.cell setDrawsBackground: NO];
		
		self.target = responder;
		self.action = @selector(respondTo:);
	}
	
	return self;
}

- (void) setupArea
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
	self.textColor = [self focusTextColor];
}

- (void) mouseExited : (NSEvent *) theEvent
{
	self.textColor = classicalTextColor;
}

- (void) mouseUp : (NSEvent *) theEvent
{
	if(_clicTarget != nil)
	{
		if([_clicTarget respondsToSelector:_clicAction])
		{
			IMP imp = [_clicTarget methodForSelector:_clicAction];
			void (*func)(id, SEL, id) = (void *)imp;
			func(_clicTarget, _clicAction, self);

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

@implementation RakAboutIcon

- (instancetype) initWithFrame : (NSRect) frame
{
	self = [super initWithFrame : frame];
	
	if(self != nil)
	{
		NSImage* image = [[NSWorkspace sharedWorkspace] iconForFile:[[NSBundle mainBundle] bundlePath]];
		
		if(image == nil)
			return nil;
		
		self.image = image;
		self.imageScaling = NSImageScaleAxesIndependently;
	}
	
	return self;
}

- (void) mouseDown:(NSEvent *)theEvent
{
	if(_clicResponder != nil)
		[_clicResponder clicIcon];
}

@end