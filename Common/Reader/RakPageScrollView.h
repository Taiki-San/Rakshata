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
 ********************************************************************************************/

#if TARGET_OS_IPHONE
@interface RakPageScrollView : UIScrollView <UIScrollViewDelegate>
#else
@interface RakPageScrollView : NSScrollView
#endif
{
	NSPoint cachedBounds;
}

#if TARGET_OS_IPHONE
@property CGFloat magnification;
#endif

@property NSRect scrollViewFrame;
@property NSRect contentFrame;
@property (nonatomic) BOOL pageTooLarge;
@property (nonatomic) BOOL pageTooHigh;
@property uint page;
@property (nonatomic) BOOL isPDF;

- (void) scrollToTopOfDocument : (BOOL) animated;
- (void) scrollToBottomOfDocument : (BOOL) animated;

- (void) scrollToBeginningOfDocument;
- (void) scrollToEndOfDocument;

- (NSRect) documentViewFrame;
- (NSPoint) scrollerPosition;

- (void) scrollToPoint : (NSPoint) origin;
- (void) scrollWithAnimationToPoint : (NSPoint) origin;

@end

#if !TARGET_OS_IPHONE
@interface RakImageView : NSImageView
{
	NSBitmapImageRep *data;
	NSTimer *animationTimer;

	int currentFrame;
	int frameCount;
}

- (void) startAnimation;
- (void) stopAnimation;

#else
@interface RakImageView : UIImageView
#endif

@property uint page;

@end
