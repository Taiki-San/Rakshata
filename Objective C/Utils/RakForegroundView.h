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

@class RakForegroundView;

@interface RakForegroundViewBackgroundView : NSView
{
	RakForegroundView * _father;
}

- (id) initWithFrame: (NSRect) frameRect : (RakForegroundView *) father;
- (NSColor *) getBackgroundColor;

- (void) mouseEnteredForced : (NSEvent *) theEvent;
- (void) mouseExitedForced:(NSEvent *)theEvent;

@end

@interface RakForegroundViewContentView : NSView

@end

@interface RakForegroundView : NSObject
{
	RakForegroundViewBackgroundView * background;
	RakForegroundViewContentView * _coreView;
	
	BOOL animationInProgress;
}

@property (assign) id delegate;

- (id) init : (NSView *) contentView : (NSView *) coreView;
- (RakForegroundViewContentView *) initCoreView : (NSRect) contentViewFrame;

- (void) switchState;
- (BOOL) isVisible;

- (void) switchOver : (BOOL) isDisplayed;

@end

