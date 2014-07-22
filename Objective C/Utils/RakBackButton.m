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

@implementation RakBackButton

- (id)initWithFrame : (NSRect) frame : (bool) isOneLevelBack
{
	frame.origin.x += frame.size.width * RBB_BUTTON_POSX / 100.0f;
	frame.origin.y = RBB_TOP_BORDURE;
	frame.size.width *= RBB_BUTTON_WIDTH / 100.0f;
	frame.size.height = RBB_BUTTON_HEIGHT;
	
    self = [super initWithFrame:frame];
    if (self != nil)
	{
		[self setAutoresizesSubviews:NO];
		[self setWantsLayer:true];
		[self setBordered:NO];
		[self.layer setCornerRadius:4];
		cursorOnMe = false;
		
		[Prefs getCurrentTheme:self];	//We register to theme change

		//On initialise la cellule
		[self.cell switchToNewContext: (isOneLevelBack ? @"back" : @"backback") : RB_STATE_STANDARD];
	
		//Set tracking area
		tag = [self addTrackingRect:[self bounds] owner:self userData:NULL assumeInside:NO];
	}
    return self;
}

- (NSRect) createFrameFromSuperFrame : (NSRect) superFrame
{
	superFrame.origin.y = RBB_TOP_BORDURE;
	superFrame.size.height = self.frame.size.height;
	superFrame.origin.x += superFrame.size.width * RBB_BUTTON_POSX / 100.0f;
	superFrame.size.width *= RBB_BUTTON_WIDTH / 100.0f;
	
	return superFrame;
}

- (void) setFrame:(NSRect)frameRect
{
	NSRect newFrame = [self createFrameFromSuperFrame:frameRect];
	[super setFrame: newFrame];
	
	newFrame.origin.x = newFrame.origin.y = 0;
	[self removeTrackingRect:tag];
	tag = [self addTrackingRect:newFrame owner:self userData:NULL assumeInside:NO];
}

- (void)resizeSubviewsWithOldSize:(NSSize)oldBoundsSize
{
	NSLog(@"[%@] - Weird constraints detected!", self);
}

- (void) resizeAnimation : (NSRect) frameRect
{
	NSRect newFrame = [self createFrameFromSuperFrame:frameRect];
	
	[self.animator setFrame: newFrame];
	
	newFrame.origin.x = newFrame.origin.y = 0;
	[self removeTrackingRect:tag];
	tag = [self addTrackingRect:newFrame owner:self userData:NULL assumeInside:NO];
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
	return [Prefs getSystemColor:GET_COLOR_BACKGROUD_BACK_BUTTONS : nil];
}

- (NSColor *) getColorBackgroundSlider
{
	return [Prefs getSystemColor:GET_COLOR_BACKGROUD_BACK_BUTTONS_ANIMATING : nil];
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return;
	
	[self setNeedsDisplay];
}

#pragma mark - Events

- (bool) confirmMouseOnMe
{
	if([self.superview superclass] == [RakTabView class])	//On vérifie que le tab est ouvert ET que la souris est bien sur nous
	{
		NSRect frame = [self frame];
		if([self.superview isFlipped])
			frame.origin.y = self.superview.frame.size.height - frame.size.height - frame.origin.y;
		
		return ![(RakTabView *) self.superview isStillCollapsedReaderTab] && [(RakTabView *) self.superview isCursorOnRect:frame];
	}
	else
		return YES;
}

- (void) mouseEntered:(NSEvent *)theEvent
{
	if([self confirmMouseOnMe])
	{
		cursorOnMe = true;
		[self startAnimation];
	}
}

- (void) mouseDown:(NSEvent *)theEvent
{
	cursorOnMe = false;
	[(RakWindow*) self.window stopDrag];
	[self.cell setAnimationInProgress:NO];
}

- (void) mouseUp:(NSEvent *)theEvent
{
	[self performClick:self];
}

- (void) mouseExited:(NSEvent *)theEvent
{
	cursorOnMe = false;
	[self.cell setAnimationInProgress:NO];
	[_animation stopAnimation];
	[self setNeedsDisplay:YES];
}

- (void) performClick : (id) sender
{
	if([self confirmMouseOnMe])
	{
		cursorOnMe = false;
		[self.cell setAnimationInProgress:NO];
		[super performClick:sender];
	}
}

#pragma mark - Animation

- (void) startAnimation
{
	_animation = [[[NSAnimation alloc] initWithDuration:1 animationCurve:NSAnimationLinear] autorelease];
	[_animation setFrameRate:60.0];
	[_animation setAnimationBlockingMode:NSAnimationNonblocking];
	[_animation setDelegate:self];
	
	for (NSAnimationProgress i = 0; i < 1; i+= 1/60.0f)
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
	if(cursorOnMe)
	{
		[self.cell setAnimationInProgress:NO];
		[self performClick:self];
	}

	_animation = nil;
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
