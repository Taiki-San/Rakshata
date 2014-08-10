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

- (id) init : (NSArray*) views : (BOOL) fastAnimation
{
	self = [super init];
	if(self != nil)
	{
		NSMutableArray * validatedViews = [NSMutableArray array];
		
		[views enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
			if([obj isKindOfClass:[RakTabView class]])
				[validatedViews addObject:obj];
		}];
		
		_views = [[NSArray arrayWithArray:validatedViews] retain];
		[Prefs getPref:PREFS_GET_IS_READER_MT :&readerMode];
		animationDuration = fastAnimation ? 0.1 : 0.2;
	}
	return self;
}

- (void) setUpViews
{
	RakTabView * currentView;
	NSUInteger i, count = [_views count];
	for(i = 0; i < count; i++)
	{
		currentView = [_views objectAtIndex:i];
		if([currentView respondsToSelector:@selector(setUpViewForAnimation:)])
			[currentView setUpViewForAnimation : readerMode];
	}
}

- (void) performTo
{
	[self performFromTo:NULL];
}

- (void) performFromTo : (NSArray*) basePosition
{
	int count = [_views count];		//	i doit être un int pour récupérer -1 si indexOfObjectPassingTest fail, et count ne devrait pas causer d'overflow
	RakTabView *currentView;
	haveBasePos = (basePosition != nil && [basePosition count] == count);
	
	[NSAnimationContext beginGrouping];
	
	[[NSAnimationContext currentContext] setDuration:animationDuration];
	
	[[NSAnimationContext currentContext] setCompletionHandler:^{
		[self cleanUpAnimation];
	}];
	
	[[[NSApp delegate] MDL] createFrame];

	byte pos = 0;
	for(currentView in _views)
	{
		[self resizeView:currentView : haveBasePos ? [basePosition objectAtIndex:pos++] : nil];
	}
	
	[NSAnimationContext endGrouping];
}

- (void) resizeView : (RakTabView *) view : (id) basePos
{
	if(haveBasePos)
	{
		CABasicAnimation *animation = [CABasicAnimation animation];
		animation.fromValue = basePos;
		[view.animations setValue:animation forKey:@"frame"];
	}
	
	if([view respondsToSelector:@selector(resizeAnimation)])
	{
		[view resizeAnimation];
		
		if([view isKindOfClass:[RakTabView class]])
			view->resizeAnimationCount++;
	}
}

- (void) cleanUpAnimation
{
	NSUInteger count = [_views count];
	RakTabView * currentView;
	for(NSUInteger i = 0; i < count; i++)
	{
		currentView = [_views objectAtIndex:i];
		
		//resizeAnimationCount == 1 => dernière animation en cours
		//Post mortem: no idea why, de mémoire un crash mais repose sur un contexte qu'on utilise plus pour l'instant, donc on laisse en place pour l'instant
		if([currentView isKindOfClass:[RakTabView class]] && currentView->resizeAnimationCount == 1)
		{
			if(haveBasePos)	//si on a qqchose à libérer
				[[currentView.animations objectForKey:@"frame"] release];
		}

		if([currentView respondsToSelector:@selector(refreshDataAfterAnimation)])
			[currentView refreshDataAfterAnimation];
		
		if([currentView isKindOfClass:[RakTabView class]])
			currentView->resizeAnimationCount--;
	}
	
	[_views release];
}

@end
