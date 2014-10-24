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
{
	BOOL _pageTooLarge;
	BOOL _pageTooHigh;
}

@property NSRect scrollViewFrame;
@property NSRect contentFrame;
@property BOOL pageTooLarge;
@property BOOL pageTooHigh;
@property uint page;

- (void) scrollToBeginningOfDocument;
- (void) scrollToEndOfDocument;

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