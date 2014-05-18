/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite  **
 **                                                                                         **
 *********************************************************************************************/

@interface RakBackButton : NSButton <NSAnimationDelegate>
{
	bool cursorOnMe;
	NSTrackingRectTag tag;

	NSAnimation * _animation;
}

- (id)initWithFrame : (NSRect) frame : (bool) isOneLevelBack;

- (NSRect) createFrameFromSuperFrame : (NSRect) superFrame;
- (void) resizeAnimation : (NSRect) frameRect;

- (NSColor *) getColorBackground;
- (NSColor *) getColorBackgroundSlider;

@end

@interface RakBackButtonAnimation : NSAnimation

@end

@interface RakBackButtonCell : RakButtonCell
{
	bool animationInProgress;
	CGFloat animationStatus;
}

- (void) switchToNewContext : (NSString*) imageName : (short) state;

- (void) setAnimationInProgress : (bool) start;
- (void) setAnimationStatus:(CGFloat) status;

@end