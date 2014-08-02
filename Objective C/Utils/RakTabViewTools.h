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

@class RakTabView;

@interface RakTabForegroundView : RakForegroundViewBackgroundView
{
	RakText * head;
	RakText * main;
	
	RakTabView * tabView;
}

- (id) initWithFrame: (NSRect) frameRect : (RakTabView *) father : (NSString *) detail;
- (void) resizeAnimation : (NSRect) frameRect;

@end

@interface RakTabAnimationResize : NSObject
{
	BOOL readerMode;
	BOOL haveBasePos;
	NSArray* _views;
}
- (id) init : (NSArray*)views;
- (void) setUpViews;
- (void) performTo;
- (void) performFromTo : (NSArray*) basePosition;
- (void) resizeView : (RakTabView *) view : (id) basePos;

@end
