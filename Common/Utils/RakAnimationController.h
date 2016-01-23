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

@interface RakAnimationController : NSObject <NSAnimationDelegate>
{
	id postAnimationTarget;
	BOOL haveSelectorToPing, aborting, _loop;
	
	NSAnimation * __weak _animation;
	
	CGFloat _animationDiff;
	NSInteger _initialState;
}

@property (nonatomic) CGFloat animationDuration;
@property uint stage;
@property uint animationFrame;

@property BOOL loopingBack;

@property (nonatomic) SEL selectorToPing;
@property RakView * viewToRefresh;

- (instancetype) initAsLoop;

- (void) addAction : (id) target;
- (void) updateState : (NSInteger) initialPos : (CGFloat) diff;

- (void) startAnimation;
- (void) abortAnimation;

- (void) initiateCustomAnimation : (CGFloat) stepsRemaining;

- (void) animation:(NSAnimation *)animation didReachProgressMark:(NSAnimationProgress)progress;
- (void) animationDidEnd:(NSAnimation *)animation;
- (void) postProcessingBeforeAction;

- (void) workaroundFlushAnimation;

@end
