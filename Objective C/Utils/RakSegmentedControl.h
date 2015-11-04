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

@interface RakMinimalSegmentedControl : NSSegmentedControl

- (NSRect) getButtonFrame : (NSRect) superviewFrame;
- (void) resizeAnimation : (NSRect) frameRect;

@end

@interface RakSegmentedControl : RakMinimalSegmentedControl
{
	id animationController;
}

@property (unsafe_unretained) id postAnimationTarget;
@property SEL postAnimationAction;

- (instancetype) initWithFrame : (NSRect) frame : (NSArray *) buttonMessage;

- (void) updateSelectionWithoutAnimation : (NSInteger) newState;
- (BOOL) setupTransitionAnimation : (NSNumber*) oldValue : (NSNumber *) newValue;

@end
