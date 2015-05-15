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

@interface RakBackButton : NSButton <NSAnimationDelegate>
{
	byte ID;
	BOOL cursorOnMe;
	NSTrackingRectTag tag;

	NSAnimation * _animation;
}

- (instancetype) initWithFrame : (NSRect) frame : (BOOL) isOneLevelBack;

- (void) resizeAnimation : (NSRect) frameRect;

- (NSColor *) getColorBackground;
- (NSColor *) getColorBackgroundSlider;

@end

@interface RakBackButtonAnimation : NSAnimation

@end

@interface RakBackButtonCell : RakButtonCell
{
	BOOL animationInProgress;
	CGFloat animationStatus;
}

- (void) switchToNewContext : (NSString*) imageName : (short) state;

- (void) setAnimationInProgress : (BOOL) start;
- (void) setAnimationStatus:(CGFloat) status;

@end

#define RBB_TOP_BORDURE 10
#define RBB_BUTTON_HEIGHT	25
#define RBB_BUTTON_WIDTH	75		//	%
#define RBB_BUTTON_POSX		12.5f	//	%