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
		[Prefs getCurrentTheme:self];
		
		initialized = YES;
	}
	
	return self;
}

- (void) dealloc
{
	[Prefs deRegisterForChanges:self];
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
			_animation = [[RakButtonAnimationController alloc] initWithCell:self];
			if(_animation != nil)
			{
				[_animation addAction:self];
				_animation.stage = _animation.animationFrame;
			}
		}
		
		[_animation startAnimation];
	}
	
	[super setState:value];
}

- (void) animationOver
{
	_animation = nil;
	[self.controlView display];
}

@end

@implementation RakButtonAnimationController

- (instancetype) initWithCell : (NSButtonCell *) cell
{
	self = [super init];
	
	if(self != nil)
	{
		_cell = cell;
	}
	
	return self;
}

- (void) animation : (NSAnimation *) animation didReachProgressMark : (NSAnimationProgress) progress
{
	[_cell.controlView display];
	[super animation:animation didReachProgressMark:progress];
}

@end