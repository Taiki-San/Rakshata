/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

@implementation RakCustomButton

- (instancetype) init
{
	self = [super init];
	
	if(self != nil)
	{
		[self setButtonType:NSSwitchButton];
		[self setImagePosition:NSImageOnly];
		[self sizeToFit];
	}
	
	return self;
}

@end

@implementation RakCustomButtonCell

- (instancetype) init
{
	self = [super init];
	
	if(self != nil)
	{
		[self initColors];
		[Prefs registerForChange:self forType:KVO_THEME];
		
		initialized = YES;
	}
	
	return self;
}

- (void) dealloc
{
	[Prefs deRegisterForChange:self forType:KVO_THEME];
}

#pragma mark - Color management

- (void) initColors
{
	
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	
	[self initColors];
	[self.controlView setNeedsDisplay:YES];
}

#pragma mark - Animation management

- (void) setState:(NSInteger)value
{
	if(initialized && self.state != value)
	{
		if(_animation == nil)
		{
			_animation = [[RakAnimationController alloc] init];
			if(_animation != nil)
			{
				_animation.viewToRefresh = self.controlView;
				[_animation addAction:self];
				_animation.stage = _animation.animationFrame;
			}
		}
		else if(animationIsOver)
			_animation.stage = _animation.animationFrame;
		else
			canceledAnimation = YES;		

		[_animation startAnimation];
		canceledAnimation = NO;
		animationIsOver = NO;
	}
	
	[super setState:value];
}

- (void) animationOver : (RakAnimationController *) controller
{
	animationIsOver = YES;
	
	if(canceledAnimation)
		return;

	[self.controlView display];
}

@end
