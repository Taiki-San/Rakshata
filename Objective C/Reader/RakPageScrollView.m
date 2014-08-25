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

@implementation RakPageScrollView

- (id) init
{
	self = [super init];
	
	if(self != nil)
	{
		self.borderType =		NSNoBorder;
		self.scrollerStyle =	NSScrollerStyleOverlay;
		self.drawsBackground =	NO;
		self.needsDisplay =		YES;
	}
	
	return self;
}

- (void) mouseDown:(NSEvent *)theEvent
{
	[self.nextResponder mouseDown:theEvent];
}

- (void) mouseUp:(NSEvent *)theEvent
{
	[self.nextResponder mouseUp:theEvent];
}

- (void) mouseDragged:(NSEvent *)theEvent
{
	[self.nextResponder mouseDragged:theEvent];
}

- (void) enforceScrollerPolicy
{
	self.hasVerticalScroller = self.pageTooHigh;
	self.verticalScroller.alphaValue =	0;
	self.hasHorizontalScroller = self.pageTooLarge;
	self.horizontalScroller.alphaValue = 0;
}

- (void) releaseData
{
	if(self.documentView != nil && [self.documentView class] == [NSImageView class])
	{
		((NSImageView*) self.documentView).image = nil;
	}
}

- (void) dealloc
{
	[CATransaction begin];
	[CATransaction setDisableActions:YES];

	[self releaseData];
	self.documentView = nil;
	[super dealloc];

	[CATransaction commit];
}

@end

@implementation RakImageView

- (void) setImage : (NSImage *) newImage
{
	[super setImage:newImage];
	
	[self setAnimates:NO];

	if(animationTimer != nil)
	{
		[animationTimer invalidate];
		animationTimer = nil;
	}
	
	[data release];
	data = nil;
}

- (void) startAnimation
{
	[data release];		data = [[self.image representations] objectAtIndex:0];
	currentFrame = 0;
	
	frameCount = [[data valueForProperty:NSImageFrameCount] intValue];
	
	if(frameCount > 1)
	{
		double frameDuration = [[data valueForProperty:NSImageCurrentFrameDuration] doubleValue];
		if(frameDuration < 0.01)
			frameDuration = 0.02;
		
		[data setProperty:NSImageCurrentFrame withValue:@(0)];
		
		animationTimer = [[NSTimer scheduledTimerWithTimeInterval:frameDuration target:self selector:@selector(nextFrame:) userInfo:nil repeats:YES] retain];
	}
}

- (void) dealloc
{
	if(animationTimer != nil)
	{
		[animationTimer invalidate];
		[animationTimer dealloc];
	}
	[super dealloc];
}

- (void)nextFrame:(NSTimer*)sender
{
	currentFrame = ++currentFrame % frameCount;
	
	[data setProperty:NSImageCurrentFrame withValue:@(currentFrame)];

	[self setNeedsDisplay];
}

@end