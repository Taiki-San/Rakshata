/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite  **
 **                                                                                         **
 *********************************************************************************************/

#define ANIMATION_DURATION 0.2

@implementation RakTabAnimationResize

- (id) init : (NSArray*)views
{
	self = [super init];
	if(self != nil)
	{
		_views = views;
		[Prefs getPref:PREFS_GET_IS_READER_MT :&readerMode];
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
	int i, count = [_views count];		//	i doit être un int pour récupérer -1 si indexOfObjectPassingTest fail, et count ne devrait pas causer d'overflow
	RakTabView *currentView[count];
	haveBasePos = (basePosition != nil && [basePosition count] == count);
	
	[NSAnimationContext beginGrouping];
	
	[[NSAnimationContext currentContext] setDuration:ANIMATION_DURATION];
	
	[[NSAnimationContext currentContext] setCompletionHandler:^{
		[self cleanUpAnimation];
	}];
	
	for(i = 0; i < count; i++)
		currentView[i] = [_views objectAtIndex:i];
	
	//Nous devions mettre à jour la frame du MDL en premier
	//Maj seulement MDL créait un bug vraiment tordu, possiblement lié à Cocoa lui même.
	for(i = 0; i < count; [currentView[i++] createFrame]);

	for(i = 0; i < count; i++)
		[self resizeView:currentView[i] : haveBasePos ? [basePosition objectAtIndex:i] : nil];
	
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
		if(currentView->resizeAnimationCount == 1)
		{
			if(haveBasePos)	//si on a qqchose à libérer
				[[currentView.animations objectForKey:@"frame"] release];
		}

		if([currentView respondsToSelector:@selector(refreshDataAfterAnimation)])
			[currentView refreshDataAfterAnimation];
		
		currentView->resizeAnimationCount--;
	}
}

@end
