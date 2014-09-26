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
	[Prefs getCurrentTheme:self];
	[self updateGradient];
}

- (void) dealloc
{
	[_startColor release];
	[_endColor release];
	[gradient release];
	
	[super dealloc];
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
	CGFloat width = self.bounds.size.width, drawnWidth = MIN(60, width * 0.05f);
	[gradient drawInRect : NSMakeRect(width - drawnWidth, 0, drawnWidth, self.bounds.size.height) angle:self.angle];
}

#pragma mark - UI utilities

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
	[_startColor release];
	[_endColor release];
	[gradient release];
	
	_startColor = [self startColor];
	_endColor = [self endColor: _startColor];
	gradient = [[NSGradient alloc] initWithStartingColor : _startColor endingColor : _endColor];
}

@end
