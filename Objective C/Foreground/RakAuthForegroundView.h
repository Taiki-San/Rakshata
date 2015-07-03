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

#import "RakForegroundBackgroundView.h"

@interface RakForegroundViewContentView : NSView

@end

@interface RakAuthForegroundView : NSObject
{
	RakForegroundBackgroundView * background;
	RakForegroundViewContentView * _coreView;
}

@property (unsafe_unretained) id delegate;

- (instancetype) init : (NSView *) coreView;
- (RakForegroundViewContentView *) craftCoreView : (NSRect) contentViewFrame;

- (void) switchState;
- (BOOL) isVisible;

- (void) switchOver : (BOOL) isDisplayed;

@end
