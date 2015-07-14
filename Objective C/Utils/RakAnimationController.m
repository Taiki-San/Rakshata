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

@implementation RakAnimationController

- (instancetype) init
{
	self = [super init];
	
	if(self != nil)
	{
		self.animationDuration = 0.15f;
	}
	
	return self;
}

- (void) setAnimationDuration : (CGFloat) animationDuration
{
	_animationDuration = animationDuration;
	_animationFrame = floor(animationDuration * 60.0f);
}

#pragma mark - Context update

- (void) addAction : (id) target
{
	postAnimationTarget = target;
}

- (void) updateState : (NSInteger) initialPos : (CGFloat) diff
{
	_initialState = initialPos;
	_animationDiff = diff;
}

#pragma mark - Animation Control

- (void) startAnimation
{
	if(_animation != nil)
	{
		[self abortAnimation];
	}
	
	_stage = _animationFrame - _stage;
	
	CGFloat duration = _animationDuration - _stage / _animationFrame;
	uint steps = _animationFrame - _stage;
	
	_animation = [[NSAnimation alloc] initWithDuration:duration animationCurve:NSAnimationEaseInOut];
	if(_animation == nil)
		return;
	
	_animation.frameRate = 60;
	_animation.animationBlockingMode = NSAnimationNonblocking;
	_animation.delegate = self;
	
	NSAnimationProgress progress = 0;
	for(uint i = 0; i <= steps; i++)
	{
		[_animation addProgressMark:progress];
		progress += 1.0f / steps;
	}
	
	[self initiateCustomAnimation : steps];
	
	[_animation startAnimation];
}

- (void) initiateCustomAnimation : (CGFloat) stepsRemaining
{
	
}

- (void) abortAnimation
{
	[_animation stopAnimation];
	_animation = nil;
}

#pragma mark - Animation work

- (void) animation:(NSAnimation *)animation didReachProgressMark:(NSAnimationProgress)progress
{
	[_viewToRefresh display];
	_stage++;
}

- (void) animationDidEnd:(NSAnimation *)animation
{
	if(animation == _animation)
	{
		if(_stage >= _animationFrame)
		{
			_stage = _animationFrame;
			
			[self postProcessingBeforeAction];

			if([postAnimationTarget respondsToSelector:@selector(animationOver)])
				[postAnimationTarget performSelector:@selector(animationOver)];

			else if([postAnimationTarget isKindOfClass:[NSView class]])
				[postAnimationTarget display];
		}
		
		_animation = nil;
	}
}

- (void) postProcessingBeforeAction
{
	
}

@end
