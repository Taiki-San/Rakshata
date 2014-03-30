/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                          **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite   **
 **                                                                                          **
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
	RakTabView *currentView;
	NSUInteger i, count = [_views count];
	haveBasePos = (basePosition != NULL || [basePosition count] != count);
	
	[NSAnimationContext beginGrouping];
	
	[[NSAnimationContext currentContext] setDuration:ANIMATION_DURATION];
	
	[[NSAnimationContext currentContext] setCompletionHandler:^{
		[self cleanUpAnimation];
	}];
	
	for(i = 0; i < count; i++)
	{
		currentView = [_views objectAtIndex:i];
		if(haveBasePos)
		{
			CABasicAnimation *animation = [CABasicAnimation animation];
			animation.fromValue = [basePosition objectAtIndex:i];
			[currentView.animations setValue:animation forKey:@"frame"];
		}
		
		if([currentView respondsToSelector:@selector(createFrame)])
		{
			[currentView.animator setFrame:[currentView createFrame]];
			currentView->resizeAnimationCount++;
		}
	}
	
	[NSAnimationContext endGrouping];
}

- (void) cleanUpAnimation
{
	NSUInteger count = [_views count];
	RakTabView * currentView;
	for(NSUInteger i = 0; i < count; i++)
	{
		currentView = [_views objectAtIndex:i];
		if([currentView respondsToSelector:@selector(applyRefreshSizeReaderChecks)])
		{
			[currentView applyRefreshSizeReaderChecks];
			if(haveBasePos)
			{
				[[currentView.animations objectForKey:@"frame"] release];
			}
		}
		currentView->resizeAnimationCount--;
	}
}

@end
