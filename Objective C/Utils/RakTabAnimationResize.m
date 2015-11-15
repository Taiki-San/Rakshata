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

@implementation RakTabAnimationResize

+ (void) animateTabs : (NSArray *) views : (BOOL) fastAnimation
{
	uint mainThread = getMainThread();
	NSMutableArray * validatedViews = [NSMutableArray array];
	
	[views enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
		if([obj isKindOfClass:[RakTabView class]])
			[validatedViews addObject:obj];
	}];
	
	[NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
		
		[context setDuration : fastAnimation ? ANIMATION_DURATION_SHORT : ANIMATION_DURATION_LONG];
		
		for(RakTabView * currentView in validatedViews)
			[currentView setUpViewForAnimation : mainThread];
		
		//MDL frame is used is some contexts because of where is appear
		MDL * tabMDL = [(RakAppDelegate*) [NSApp delegate] MDL];
		[tabMDL createFrame];
		tabMDL.needUpdateMainViews = NO;	//We're going to update the frame anyway
		
		for(RakTabView *currentView in validatedViews)
			[currentView resizeAnimation];
		
	} completionHandler:^{
		
		for(RakTabView * currentView in validatedViews)
			[currentView refreshDataAfterAnimation];
	}];
}

@end
