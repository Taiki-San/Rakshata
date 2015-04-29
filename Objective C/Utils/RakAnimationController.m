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
	animationFrame = animationDuration / 60.0f;
}

#pragma mark - Context update

- (void) addAction : (id) target : (SEL) action
{
	postAnimationTarget = target;
	postAnimationAction = action;
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
	
	state = animationFrame - state;
	
	CGFloat duration = _animationDuration - state / animationFrame, steps = animationFrame - state;
	
	_animation = [[NSAnimation alloc] initWithDuration:duration animationCurve:NSAnimationEaseInOut];
	[_animation setFrameRate:60];
	[_animation setAnimationBlockingMode:NSAnimationNonblocking];
	[_animation setDelegate:self];
	
	NSAnimationProgress progress = 0;
	for(uint i = 0; i < steps; i++)
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
	state++;
}

- (void) animationDidEnd:(NSAnimation *)animation
{
	if(animation == _animation)
	{
		if(state >= animationFrame)
		{
			state = animationFrame;
			
			[self postProcessingBeforeAction];
			
			IMP imp = [postAnimationTarget methodForSelector:postAnimationAction];
			void (*func)(id, SEL, id) = (void *)imp;
			func(postAnimationTarget, postAnimationAction, nil);
		}
		
		_animation = nil;
	}
}

- (void) postProcessingBeforeAction
{
	
}

@end
