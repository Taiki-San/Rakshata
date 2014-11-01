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

@implementation RakGradientView

- (void) initGradient
{
	self.gradientWidth = 0.1f;
	self.gradientMaxWidth = 60;
	
	[Prefs getCurrentTheme:self];
	[self updateGradient];
}

- (BOOL) mouseDownCanMoveWindow
{
	return YES;
}

#pragma mark - UI routines

- (void) updateTheme
{
	
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return;
	
	[self updateGradient];
	[self updateTheme];
}

- (void) drawRect:(NSRect)dirtyRect
{
	[super drawRect:dirtyRect];
	
	//We add a gradient at the extreme right
	[gradient drawInRect : [self grandientBounds] angle : self.angle];
}

#pragma mark - UI utilities

- (NSRect) grandientBounds
{
	CGFloat width = self.bounds.size.width, drawnWidth;
	
	if(self.gradientMaxWidth == 0)
		drawnWidth = width * self.gradientWidth;
	else
		drawnWidth = MIN(self.gradientMaxWidth, width * self.gradientWidth);
	
	return NSMakeRect(width - drawnWidth, 0, drawnWidth, self.bounds.size.height);
}

- (NSColor *) startColor
{
	return nil;
}

- (NSColor *) endColor : (NSColor *) startColor
{
	return nil;
}

- (void) updateGradient
{
	_startColor = [self startColor];
	_endColor = [self endColor: _startColor];
	gradient = [[NSGradient alloc] initWithStartingColor : _startColor endingColor : _endColor];
}

@end
