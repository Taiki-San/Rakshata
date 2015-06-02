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

@interface RakAnimationController : NSObject <NSAnimationDelegate>
{
	id postAnimationTarget;
	
	NSAnimation * _animation;
	
	CGFloat _animationDiff;
	NSInteger _initialState;
}

@property (nonatomic) CGFloat animationDuration;
@property uint stage;
@property uint animationFrame;

- (void) addAction : (id) target;
- (void) updateState : (NSInteger) initialPos : (CGFloat) diff;

- (void) startAnimation;
- (void) abortAnimation;

- (void) initiateCustomAnimation : (CGFloat) stepsRemaining;

- (void) animation:(NSAnimation *)animation didReachProgressMark:(NSAnimationProgress)progress;
- (void) animationDidEnd:(NSAnimation *)animation;
- (void) postProcessingBeforeAction;

@end
