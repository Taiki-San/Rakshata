/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at https://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

@interface RakAnimation : NSAnimation

@property BOOL noPing;

@end

@implementation RakAnimation

//Because of a bug in _stopAnimation:withDisplayLink: ¯\_(ツ)_/¯
- (void) dealloc
{
	if(!self.noPing && self.delegate != nil && [self.delegate respondsToSelector:@selector(workaroundFlushAnimation)])
	{
		[(id) self.delegate workaroundFlushAnimation];
	}
}

@end

@implementation RakAnimationController

//The loop need to be initialized very early
- (instancetype) initAsLoop
{
	self = [self init];
	
	if(self != nil)
	{
		_loop = YES;
	}
	
	return self;
}

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
	_animationFrame = floor(animationDuration * (_loop ? 30.0f : 60.0f));
}

#pragma mark - Context update

- (void) addAction : (id) target
{
	postAnimationTarget = target;
}

- (void) setSelectorToPing:(SEL)selectorToPing
{
	if([postAnimationTarget respondsToSelector:selectorToPing])
		haveSelectorToPing = YES;

	_selectorToPing = selectorToPing;
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

	_stage = _animationFrame - MIN(_stage, _animationFrame);
	
	CGFloat duration = _animationDuration - _stage / _animationFrame;
	uint steps = _animationFrame - _stage;
	
	RakAnimation * animation = [[RakAnimation alloc] initWithDuration:duration animationCurve:NSAnimationEaseInOut];
	if(animation == nil)
		return;
	
	_animation = animation;
	_animation.frameRate = _loop ? 30 : 60;
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
	aborting = YES;

	[_animation stopAnimation];
	[self animationDidEnd:_animation];
	
	aborting = NO;
}

#pragma mark - Animation work

- (void) animation:(NSAnimation *)animation didReachProgressMark:(NSAnimationProgress)progress
{
	[_viewToRefresh display];

	if(haveSelectorToPing)
	{
		IMP imp = [postAnimationTarget methodForSelector:_selectorToPing];
		void (*func)(id, SEL, id) = (void *)imp;
		func(postAnimationTarget, _selectorToPing, nil);
	}

	_stage++;
}

- (void) animationDidEnd:(NSAnimation *)animation
{
	if(animation != _animation)
		return;
	
	if(!aborting && _loop)
	{
		_loopingBack = !_loopingBack;
		return [self startAnimation];
	}

	if(_stage >= _animationFrame)
	{
		_stage = _animationFrame;
		
		[self postProcessingBeforeAction];

		if([postAnimationTarget respondsToSelector:@selector(animationOver:)])
			[postAnimationTarget performSelector:@selector(animationOver:) withObject:self];

		else if([postAnimationTarget isKindOfClass:[RakView class]])
			[postAnimationTarget display];
	}
		
	//Signal dealloc
	if(_animation != nil)
	{
		((RakAnimation *) _animation).noPing = YES;
		_animation = nil;
	}
}

- (void) workaroundFlushAnimation
{
	_animation = nil;
}

- (void) dealloc
{
	if(_animation != nil)
	{
		((RakAnimation *) _animation).noPing = YES;
		_animation = nil;
	}
}

- (void) postProcessingBeforeAction
{
	
}

@end
