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

enum
{
	BORDER_BUTTON = 20
};

@implementation RakSegmentedControl

- (instancetype) initWithFrame : (NSRect) frame : (NSArray *) buttonMessages
{
	self = [super initWithFrame:[self getButtonFrame:frame]];
	
	if (self != nil)
	{
		__block uint oldWidthButton = 0;
		
		[self setSegmentCount:[buttonMessages count]];
		
		[buttonMessages enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {

			[self setLabel:obj forSegment:idx];
			[self setEnabled:NO forSegment:idx];
			[self sizeToFit];
			
			CGFloat widthButton;

			if(idx == 0)
				widthButton = self.frame.size.width - BORDER_BUTTON;
			else
				widthButton = self.frame.size.width - oldWidthButton - 1;	//Hack to properly draw the buttons below
			
			[self setWidth:widthButton forSegment:idx];
			
			oldWidthButton += widthButton;
			
		}];
		
		[self setFrameOrigin:[self getButtonFrame:frame].origin];
	}
	
	return self;
}

- (NSRect) getButtonFrame : (NSRect) superviewFrame
{
	return superviewFrame;
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

- (void) resizeAnimation : (NSRect) frameRect
{
	[self.animator setFrame : [self getButtonFrame:frameRect]];
}

+ (Class)cellClass
{
	return [RakSegmentedButtonCell class];
}

- (void) dealloc
{
	[animationController abortAnimation];
}

#pragma mark - Animation

- (BOOL) setupTransitionAnimation : (NSNumber *) oldValue : (NSNumber *) newValue
{
	NSInteger initialPos = [oldValue integerValue];
	
	if(animationController == nil)
	{
		animationController = [[RakCTAnimationController alloc] init: initialPos : [newValue integerValue] - initialPos : self.cell];
		
		if(animationController == nil)
			return NO;
	}
	else
		[animationController updateState : initialPos : [newValue integerValue] - initialPos];
	
	[(RakCTSelection*) self.superview feedAnimationController:animationController];
	[animationController startAnimation];
	
	return YES;
}

@end
