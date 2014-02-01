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

#import "superHeader.h"

@implementation RakTabAnimationResize

- (id) init : (NSArray*)views : (RakTabAnimationResize*) instance
{
	self = [super init];
	if(self != nil)
	{
		_views = views;
		_instance = instance;
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
		if([currentView respondsToSelector:@selector(setUpViewForAnimation)])
			[currentView setUpViewForAnimation];
	}
}

- (void) perform
{
	RakTabView *subViewView;
	NSUInteger i, count = [_views count];
	
	[NSAnimationContext beginGrouping];
	
	[[NSAnimationContext currentContext] setDuration:0.25];
	
	for(i = 0; i < count; i++)
	{
		subViewView = [_views objectAtIndex:i];
		if([subViewView respondsToSelector:@selector(createFrame)])
			[subViewView.animator setFrame:[subViewView createFrame]];
	}
	
	[[NSAnimationContext currentContext] setCompletionHandler:^{
		[self performSelector:@selector(cleanUpAnimation) withObject:_views withObject:self];
	}];
	
	[NSAnimationContext endGrouping];
}

- (void) cleanUpAnimation
{
	[_instance release];
}

@end
