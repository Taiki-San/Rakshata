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

@implementation RakForegroundBackgroundView

- (instancetype) init
{
	return [self initWithFrame:[RakApp getContentView].bounds];
}

- (instancetype) initWithFrame:(NSRect)frameRect
{
	self = [super initWithFrame:frameRect];
	
	if(self != nil)
	{
		[self setWantsLayer:YES];
		[self setAutoresizesSubviews:NO];
		[self.layer setBackgroundColor : [self getBackgroundColor].CGColor];
		[self setAlphaValue:0];
		[Prefs registerForChange:self forType:KVO_THEME];
	}
	
	return self;
}

- (void) dealloc
{
	[Prefs deRegisterForChange:self forType:KVO_THEME];
}

- (void) attachToView
{
	[[RakApp getContentView] addSubview:self];
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	
	[self.layer setBackgroundColor : [self getBackgroundColor].CGColor];
	[self setNeedsDisplay:YES];
}

- (RakColor *) getBackgroundColor
{
	return [Prefs getSystemColor:COLOR_FILTER_FOREGROUND];
}

- (void) mouseDown:(NSEvent *)theEvent
{
	if([_father isVisible])
	{
		[_father switchState];
	}
}

- (void) mouseUp:(NSEvent *)theEvent		{	}
- (void) mouseEntered:(NSEvent *)theEvent	{	}
- (void) mouseEnteredForced : (NSEvent *) theEvent	{	[super mouseEntered: theEvent];	}

- (void) mouseExited:(NSEvent *)theEvent	{	}
- (void) mouseExitedForced:(NSEvent *)theEvent		{	[super mouseExited: theEvent];	}

- (void) mouseMoved:(NSEvent *)theEvent		{	}
- (void) swipeWithEvent:(NSEvent *)event	{	}
- (void) scrollWheel:(NSEvent *)theEvent	{	}

@end
