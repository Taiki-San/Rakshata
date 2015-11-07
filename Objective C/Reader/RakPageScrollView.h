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

@interface RakPageScrollView : NSScrollView

@property NSRect scrollViewFrame;
@property NSRect contentFrame;
@property (nonatomic) BOOL pageTooLarge;
@property (nonatomic) BOOL pageTooHigh;
@property uint page;
@property (nonatomic) BOOL isPDF;

- (void) scrollToBeginningOfDocument;
- (void) scrollToEndOfDocument;

- (NSRect) documentViewFrame;

- (void) scrollToPoint : (NSPoint) origin;

@end

@interface RakImageView : NSImageView
{
	NSBitmapImageRep *data;
	NSTimer *animationTimer;

	int currentFrame;
	int frameCount;
}

@property uint page;

- (void) startAnimation;
- (void) stopAnimation;

@end