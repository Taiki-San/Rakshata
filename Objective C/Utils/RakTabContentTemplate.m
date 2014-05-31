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
 ********************************************************************************************/

@implementation RakTabContentTemplate

- (void) setupInternal
{
	[Prefs getPref:PREFS_GET_MAIN_THREAD :&mainThread];
	[self setWantsLayer:YES];
	self.layer.backgroundColor = [self getBackgroundColor].CGColor;
	self.layer.cornerRadius = 5.0;
}

- (BOOL) isOpaque
{
	return YES;
}

- (NSString *) getContextToGTFO
{
	return STATE_EMPTY;
}

#pragma mark - Hooks to critical routines

- (void) setFrame:(NSRect)frameRect
{
	[super setFrame:frameRect];
	
	[self setFrameInternalViews:[self bounds]];
}

- (void) resizeAnimation : (NSRect)frameRect
{
	[self.animator setFrame:frameRect];
	
	frameRect.origin.x = frameRect.origin.y = 0;
	[self resizeAnimationInternalViews:frameRect];
}

- (id) retain
{
	[self retainInternalViews];
	return [super retain];
}

- (oneway void) release
{
	[self releaseInternalViews];
	[super release];
}

- (void) setFrameInternalViews : (NSRect) newBound
{
	
}

- (void) resizeAnimationInternalViews : (NSRect) newBound
{
	
}

- (void) retainInternalViews
{
	
}

- (void) releaseInternalViews
{
	
}

#pragma mark - Color

- (NSColor*) getBackgroundColor
{
	return [NSColor clearColor];
}

@end
