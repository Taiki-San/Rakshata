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
	uint mainThread;
	NSMutableArray * validatedViews = [NSMutableArray array];
	
	[views enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
		if([obj isKindOfClass:[RakTabView class]])
			[validatedViews addObject:obj];
	}];
	
	[Prefs getPref:PREFS_GET_MAIN_THREAD :&mainThread];

	[NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
		
		[context setDuration : fastAnimation ? 0.1 : 0.2];
		
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
