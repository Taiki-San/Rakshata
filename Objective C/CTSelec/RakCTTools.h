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

@interface RakCTAnimationController : NSObject <NSAnimationDelegate>
{
	RakSegmentedButtonCell * _cell;
	RakCTSelectionListContainer * _chapter;
	RakCTSelectionListContainer * _volume;
	
	id postAnimationTarget;
	SEL postAnimationAction;

	
	NSAnimation * _animation;
	uint state;
	
	CGFloat _animationDiff;
	NSInteger _initialState;
	
	CGFloat distanceToCoverPerMark;
	NSPoint chapOrigin;
	NSPoint volOrigin;
}

- (instancetype) init : (NSInteger) initialPos : (CGFloat) diff : (RakSegmentedButtonCell*) cell;

- (void) addCTContent : (RakCTSelectionListContainer*) chapter : (RakCTSelectionListContainer*) volume;
- (void) addAction : (id) target : (SEL) action;
- (void) updateState : (NSInteger) initialPos : (CGFloat) diff;

- (void) startAnimation;
- (void) abortAnimation;

@end

@interface RakCTProjectImageView : NSImageView

- (id) initWithImageName : (char*) URLRepo : (NSString *) imageName : (NSRect) superviewFrame;
- (void) updateProject : (NSString *) imageName;

- (NSRect) getProjectImageSize : (NSRect) superviewFrame : (NSSize) imageSize;

- (void) resizeAnimation : (NSRect) frameRect;

@end

NSString * priceString(uint price);
