/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

@implementation RakGradientView

- (void) initGradient
{
	_gradientWidth = 0.1f;
	_gradientMaxWidth = 60;
	
	[Prefs registerForChange:self forType:KVO_THEME];
	[self updateGradient];
}

- (void) dealloc
{
	[Prefs deRegisterForChange:self forType:KVO_THEME];
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
		return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	
	[self updateGradient];
	[self updateTheme];
	[self setNeedsDisplay:YES];
}

- (void) drawRect:(NSRect)dirtyRect
{
	[super drawRect:dirtyRect];
	
	//We add a gradient at the extreme right
	[gradient drawInRect : [self grandientBounds] angle : _angle];
}

#pragma mark - UI utilities

- (NSRect) grandientBounds
{
	CGFloat width = self.bounds.size.width, drawnWidth;
	
	if(_gradientMaxWidth == 0)
		drawnWidth = width * _gradientWidth;
	else
		drawnWidth = MIN(_gradientMaxWidth, width * _gradientWidth);
	
	return NSMakeRect(width - drawnWidth, 0, drawnWidth, self.bounds.size.height);
}

- (RakColor *) startColor
{
	return [RakColor blackColor];
}

- (RakColor *) endColor : (RakColor *) startColor
{
	return [RakColor clearColor];
}

- (void) updateGradient
{
	_startColor = [self startColor];
	_endColor = [self endColor: _startColor];
	gradient = [[NSGradient alloc] initWithStartingColor : _startColor endingColor : _endColor];
}

@end
