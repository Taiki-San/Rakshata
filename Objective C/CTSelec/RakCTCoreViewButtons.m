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
 ********************************************************************************************/

@implementation RakCTCoreViewButtons

- (id)initWithFrame:(NSRect)frame
{
	self = [super initWithFrame:[self getButtonFrame:frame]];
	
	if (self != nil)
	{
		uint widthButton;
		
		[self setSegmentCount:2];
		[self setLabel:@"Chapitres" forSegment:0];
		[self setEnabled:NO forSegment:0];
		[self sizeToFit];
		widthButton = self.frame.size.width - 20;
		[self setWidth:widthButton forSegment:0];
		
		[self setLabel:@"Tomes" forSegment:1];
		[self setEnabled:NO forSegment:1];
		[self sizeToFit];
		widthButton = self.frame.size.width - widthButton - 1;	//Hack to properly draw the buttons below
		[self setWidth:widthButton forSegment:1];
		
		[self setFrameOrigin:[self getButtonFrame:frame].origin];
	}
	
	return self;
}

- (NSRect) getButtonFrame : (NSRect) superViewFrame
{
	NSRect frame = self.frame;
	
	frame.size.height = CT_READERMODE_HEIGHT_CT_BUTTON;
	if(frame.size.width > superViewFrame.size.width)
		frame.size.width = superViewFrame.size.width;
	
	frame.origin.y = superViewFrame.size.height - frame.size.height;
	frame.origin.x = superViewFrame.size.width / 2 - frame.size.width / 2;
	
	return frame;
}

- (void) setLabel:(NSString *)label forSegment:(NSInteger)segment
{
	[super setLabel:label forSegment:segment];
	[self sizeToFit];
}

- (void) setFrame:(NSRect)frameRect
{
	NSRect newFrame = [self getButtonFrame:frameRect];
	[super setFrame: newFrame];

	if(newFrame.size.width == frameRect.size.width)
	{
		[self sizeToFit];
		newFrame = [self getButtonFrame:frameRect];
		[super setFrame: newFrame];
	}
}

- (void)resizeSubviewsWithOldSize:(NSSize)oldBoundsSize
{
	NSLog(@"[%@] - Weird constraints detected!", self);
}

- (void) resizeAnimation : (NSRect) frameRect
{
	[self.animator setFrame : [self getButtonFrame:frameRect]];
}

+ (Class)cellClass
{
	return [RakSegmentedButtonCell class];
}

#pragma mark - Animation

- (BOOL) setupTransitionAnimation : (NSNumber *) oldValue : (NSNumber *) newValue
{
	if(_animation != nil)
	{
		[_animation stopAnimation];
		_animation = nil;
	}
	
	_animation = [[[NSAnimation alloc] initWithDuration:0.15 animationCurve:NSAnimationEaseInOut] autorelease];
	[_animation setFrameRate:60];
	[_animation setAnimationBlockingMode:NSAnimationNonblocking];
	[_animation setDelegate:self];
	
	initialPos = [oldValue integerValue];
	animationDiff = [newValue integerValue] - initialPos;
	
	for (NSAnimationProgress i = 0; i < 1; i += 1 / 9.0f)
	{
		[_animation addProgressMark : i];
	}
	
	[_animation startAnimation];
	
	return YES;
}

- (void) animation:(NSAnimation *)animation didReachProgressMark:(NSAnimationProgress)progress
{
	[self.cell updateAnimationStatus: YES : initialPos + animationDiff * progress];
	[self setNeedsDisplay:YES];
}

- (void)animationDidEnd:(NSAnimation *)animation
{
	if(animation == _animation)
	{
		[self.cell updateAnimationStatus:NO :1];
		_animation = nil;
	}

	[self setNeedsDisplay:YES];
}

@end
