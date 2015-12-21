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

@implementation RakTabForegroundView

- (instancetype) initWithFrame: (NSRect) frameRect : (RakTabView *) father : (NSString *) detail
{
	self = [super initWithFrame:frameRect];
	
	if(self != nil)
	{
		tabView = father;
		
		RakColor * textColor = [Prefs getSystemColor:COLOR_CLICKABLE_TEXT];
		
		head = [[RakText alloc] initWithText:frameRect :NSLocalizedString(@"AUTH-REQUIRED", nil) : textColor];
		[head setFont:[NSFont boldSystemFontOfSize:[NSFont systemFontSize]]];
		[head sizeToFit];
		
		main = [[RakText alloc] initWithText:frameRect : detail : textColor];
		[main setAlignment:NSTextAlignmentCenter];
		[main sizeToFit];
		[main setFrameOrigin:NSMakePoint(frameRect.size.width / 2 - main.bounds.size.width / 2, frameRect.size.height / 2 - main.bounds.size.height / 2 -  head.bounds.size.height * 0.75)];
		[self addSubview:main];
		
		[head setFrameOrigin:NSMakePoint(frameRect.size.width / 2 - head.bounds.size.width / 2, NSMaxY(main.frame) + head.bounds.size.height / 2)];
		[self addSubview:head];
	}
	
	return self;
}

- (void) dealloc
{
	[head removeFromSuperview];
	[main removeFromSuperview];
}

- (void) setFrame : (NSRect)frameRect
{
	frameRect.origin = NSZeroPoint;
	
	[super setFrame:frameRect];
	[main setFrameOrigin:NSMakePoint(frameRect.size.width / 2 - main.bounds.size.width / 2, frameRect.size.height / 2 - main.bounds.size.height / 2 - head.bounds.size.height * 0.75)];
	[head setFrameOrigin:NSMakePoint(frameRect.size.width / 2 - head.bounds.size.width / 2, NSMaxY(main.frame) + head.bounds.size.height / 2)];
}

- (void) resizeAnimation : (NSRect) frameRect
{
	frameRect.origin = NSZeroPoint;
	[self.animator setFrame:frameRect];
	[main.animator setFrameOrigin:NSMakePoint(frameRect.size.width / 2 - main.bounds.size.width / 2, frameRect.size.height / 2 - main.bounds.size.height / 2 - head.bounds.size.height * 0.75)];
	[head.animator setFrameOrigin:NSMakePoint(frameRect.size.width / 2 - head.bounds.size.width / 2, NSMaxY(main.frame) + head.bounds.size.height / 2)];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	
	RakColor * textColor = [Prefs getSystemColor:COLOR_CLICKABLE_TEXT];
	
	for(RakText * view in self.subviews)
	{
		[view setTextColor:textColor];
		[view setNeedsDisplay:YES];
	}
}

- (RakColor*) getBackgroundColor
{
	return [[super getBackgroundColor] colorWithAlphaComponent:0.6f];
}

- (void) mouseDown : (NSEvent*) theEvent
{
	[(RakAppDelegate*) [NSApp delegate]openLoginPrompt];
}

@end
