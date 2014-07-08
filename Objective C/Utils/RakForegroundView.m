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

@implementation RakForegroundView

- (id) init : (NSView *) contentView
{
	self = [super init];
	
	if(self != nil)
	{
		animationInProgress = NO;
		background = [[RakForegroundViewBackgroundView alloc] initWithFrame:contentView.bounds : self];
		if(background != nil)
		{
			[contentView addSubview:background];
		}
		
		coreView = [self initCoreView : contentView.bounds];
		if(coreView != nil)
		{
			[coreView setFrameOrigin:NSMakePoint(contentView.frame.size.width / 2 - coreView.frame.size.width / 2, -coreView.frame.size.height)];
			[contentView addSubview:coreView];
		}
	}
	
	return self;
}

- (RakForegroundViewContentView *) initCoreView : (NSRect) contentViewFrame
{
	RakForegroundViewContentView * view = [[RakForegroundViewContentView alloc] initWithFrame:NSMakeRect(0, 0, contentViewFrame.size.width * 3 / 4, contentViewFrame.size.height * 4 / 5)];
	
	if(view != nil)
	{
		[view setWantsLayer:YES];
		[view setAutoresizesSubviews:NO];
		[view.layer setBackgroundColor : NSColor.redColor.CGColor];
	}
	
	return view;
}

- (id) retain
{
	[background retain];
	[coreView retain];
	return [super retain];
}

- (oneway void) release
{
	[background release];
	[coreView release];
	[super release];
}

- (void) dealloc
{
	[background removeFromSuperview];	background = nil;
	[coreView removeFromSuperview];		coreView = nil;
	[super dealloc];
}

#pragma mark - Animations

- (void) switchState
{
	if(animationInProgress)
		return;
	
	[NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
		[context setDuration:0.2];
		
		animationInProgress = YES;
		
		if(background != nil)
			[background.animator setAlphaValue: [background alphaValue] ? 0 : 1.0f];
		
		if(coreView != nil)
		{
			NSRect frame = coreView.frame;
			
			if(frame.origin.y)
				frame.origin.y = 0;
			else
				frame.origin.y = -coreView.frame.size.height;
			
			[coreView.animator setFrame:frame];
		}
		
	} completionHandler:^{
		animationInProgress = NO;
		[self switchOver];
	}];
}

- (BOOL) isVisible
{
	if(background != nil)
		return background.alphaValue != 0;
	
	else if(coreView != nil)
		return coreView.frame.origin.y > 0;
	
	return NO;
}

#pragma mark - Content control

- (void) switchOver
{
	
}
	 
@end

@implementation RakForegroundViewBackgroundView

- (id) initWithFrame: (NSRect) frameRect : (RakForegroundView *) father
{
	self = [super initWithFrame:frameRect];
	
	if(self != nil)
	{
		_father = father;
		[self setWantsLayer:YES];
		[self setAutoresizesSubviews:NO];
		[self.layer setBackgroundColor : [NSColor colorWithDeviceWhite:15/255.0f alpha:0.7f].CGColor];
		[self setAlphaValue:0];
	}
	
	return self;
}

- (void) mouseDown:(NSEvent *)theEvent
{
	if([_father isVisible])
	{
		[_father switchState];
		[_father release];
	}
}

@end

@implementation RakForegroundViewContentView

- (void) setFrame:(NSRect)frameRect
{
	[super setFrameSize:NSMakeSize(frameRect.size.width * 3 / 4, frameRect.size.height * 4 / 5)];
	
	CGFloat y = self.frame.origin.y == 0 ? 0 : -self.frame.size.height;
	[super setFrameOrigin:NSMakePoint(self.superview.frame.size.width / 2 - self.frame.size.width / 2, y)];
}

@end