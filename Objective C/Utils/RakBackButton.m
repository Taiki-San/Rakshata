/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite  **
 **                                                                                         **
 *********************************************************************************************/

@implementation RakBackButton

- (id)initWithFrame:(NSRect)frame : (int) numberReturnChar
{
	frame.origin.x = frame.size.width * RBB_BUTTON_POSX / 100.0f;
	frame.origin.y = frame.size.height - RBB_TOP_BORDURE - RBB_BUTTON_HEIGHT;
	frame.size.width *= RBB_BUTTON_WIDTH / 100.0f;
	frame.size.height = RBB_BUTTON_HEIGHT;
	
	
    self = [super initWithFrame:frame];
    if (self)
	{
		[self setAutoresizingMask:NSViewWidthSizable];
		[self setWantsLayer:true];
		[self setBordered:NO];
		[self.layer setCornerRadius:4];
		cursorOnMe = false;

		//On initialise la cellule
		[self.cell switchToNewContext: @"arrowBack" : RB_STATE_STANDARD];
	
		//Set tracking area
		tag = [self addTrackingRect:[self bounds] owner:self userData:NULL assumeInside:NO];
	}
    return self;
}

- (void) setFrame:(NSRect)frameRect
{
	frameRect.size.height = self.frame.size.height;
	frameRect.size.width = self.superview.frame.size.width * RBB_BUTTON_WIDTH / 100.0f;
	frameRect.origin.x = self.superview.frame.size.width * RBB_BUTTON_POSX / 100.0f;
	frameRect.origin.y = self.superview.frame.size.height - self.frame.size.height - RBB_TOP_BORDURE;
	
	[super setFrame:frameRect];
	
	[self removeTrackingRect:tag];
	tag = [self addTrackingRect:[self bounds] owner:self userData:NULL assumeInside:NO];
	
}

- (void) drawRect:(NSRect)dirtyRect
{
	[self.cell drawBezelWithFrame:dirtyRect inView:self];
	[self.cell drawImage:[self.cell image] withFrame:dirtyRect inView:self];
}

+ (Class) cellClass
{
	return [RakBackButtonCell class];
}

- (void) setHidden:(BOOL)flag
{
	if([self isHidden] != flag)
		[super setHidden:flag];
}

#pragma mark - Color

- (NSColor *) getColorBackground
{
	return [Prefs getSystemColor:GET_COLOR_BACKGROUD_BACK_BUTTONS];
}

- (NSColor *) getColorBackgroundSlider
{
	return [Prefs getSystemColor:GET_COLOR_BACKGROUD_BACK_BUTTONS_ANIMATING];
}

#pragma mark - Events

- (void) mouseEntered:(NSEvent *)theEvent
{
	if([self confirmMouseOnMe])
	{
		cursorOnMe = true;
		[self startAnimation];
	}
}

- (bool) confirmMouseOnMe
{
	return ![(RakTabView *) self.superview isStillCollapsedReaderTab];
}

- (void) mouseExited:(NSEvent *)theEvent
{
	cursorOnMe = false;
	[self.cell setAnimationInProgress:false];
	[_animation stopAnimation];
	[self setNeedsDisplay:YES];
}

//	Haaaaaccckkkyyyyyyyyy, in theory, nobody should call this function except before performing the click
- (SEL) action
{
	cursorOnMe = false;
	[self.cell setAnimationInProgress:false];
	return [super action];
}

#pragma mark - Animation

- (void) startAnimation
{
	_animation = [[NSAnimation alloc] initWithDuration:1.6 animationCurve:NSAnimationLinear];
	[_animation setFrameRate:60.0];
	[_animation setAnimationBlockingMode:NSAnimationNonblocking];
	[_animation setDelegate:self];
	
	for (NSAnimationProgress i = 0; i < 1; i+= 0.01)
	{
		[_animation addProgressMark:i];
	}
	
	[self.cell setAnimationInProgress:true];
	[_animation startAnimation];
}

- (void) animation:(NSAnimation *)animation didReachProgressMark:(NSAnimationProgress)progress
{
	if(cursorOnMe)
	{
		[self.cell setAnimationStatus: progress];
		[self setNeedsDisplay:YES];
	}
}

- (void)animationDidEnd:(NSAnimation *)animation
{
	if(cursorOnMe && animation)
	{
		[self.cell setAnimationInProgress:false];
		[self performClick:self];
		[animation release];
	}
}

@end

@implementation RakBackButtonCell

- (void) switchToNewContext : (NSString*) imageName : (short) state
{
	clicked		= [[RakResPath craftResNameFromContext:imageName : YES : YES : 1] retain];
	nonClicked	= [[RakResPath craftResNameFromContext:imageName : NO : YES : 1] retain];
	unAvailable = [[RakResPath craftResNameFromContext:imageName : NO : NO : 1] retain];
	
	notAvailable = false;
	
	if(state == RB_STATE_STANDARD && nonClicked != nil)
		[self setImage:nonClicked];
	else if(state == RB_STATE_HIGHLIGHTED && clicked != nil)
		[self setImage:clicked];
	else if(unAvailable != nil)
	{
		[self setImage:unAvailable];
		notAvailable = true;
	}
	else
	{
		NSLog(@"Failed at create button for icon: %@", imageName);
	}
}

- (void) setAnimationInProgress : (bool) start
{
	animationInProgress = start;
	animationStatus = 0;
}

- (void) setAnimationStatus:(CGFloat) status
{
	animationStatus = status;
}

- (void)drawBezelWithFrame:(NSRect)frame inView:(RakBackButton *)controlView
{
	NSGraphicsContext *ctx = [NSGraphicsContext currentContext];
	
	[ctx saveGraphicsState];
	
	if([self isHighlighted])
	{
		[[NSColor colorWithCalibratedWhite:0.0f alpha:0.35] setFill];
		NSRectFill(frame);
	}
	else if (animationInProgress)
	{
		NSRect drawingRect = frame;
		
		drawingRect.size.width *= animationStatus;
		if (animationStatus)
		{
			[[controlView getColorBackgroundSlider] setFill];
			NSRectFill(drawingRect);
		}
			
		if(animationStatus != 1)
		{
			drawingRect.origin.x = drawingRect.size.width;
			drawingRect.size.width = frame.size.width;
			[[controlView getColorBackground] setFill];
			NSRectFill(drawingRect);
		}
	}
	else
	{
		[[controlView getColorBackground] setFill];
		NSRectFill(frame);
	}
	[ctx restoreGraphicsState];
}

@end
