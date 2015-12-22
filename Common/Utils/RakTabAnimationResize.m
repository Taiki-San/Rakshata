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

@implementation RakTabAnimationResize

+ (void) animateTabs : (NSArray *) views : (BOOL) fastAnimation
{
	uint mainThread = getMainThread();
	NSMutableArray * validatedViews = [NSMutableArray array];
	
	[views enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
		if([obj isKindOfClass:[RakTabView class]])
			[validatedViews addObject:obj];
	}];

#if TARGET_OS_IPHONE
	[CATransaction begin];
	[CATransaction setAnimationDuration:fastAnimation ? ANIMATION_DURATION_SHORT : ANIMATION_DURATION_LONG];
#else
	
	[NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
		
		[context setDuration : fastAnimation ? ANIMATION_DURATION_SHORT : ANIMATION_DURATION_LONG];
#endif
		
		for(RakTabView * currentView in validatedViews)
			[currentView setUpViewForAnimation : mainThread];
		
		//MDL frame is used is some contexts because of where is appear
		MDL * tabMDL = RakApp.MDL;
		[tabMDL createFrame];
		tabMDL.needUpdateMainViews = NO;	//We're going to update the frame anyway
		
		for(RakTabView *currentView in validatedViews)
			[currentView resizeAnimation];

#if TARGET_OS_IPHONE
	[CATransaction setCompletionBlock:^{
#else
	} completionHandler:^{
#endif
		
		for(RakTabView * currentView in validatedViews)
			[currentView refreshDataAfterAnimation];
	}];
}

@end
