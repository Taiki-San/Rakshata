/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at https://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
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
