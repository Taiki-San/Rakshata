/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
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

@implementation RakListScrollView

- (instancetype) initWithFrame:(NSRect)frameRect
{
	self = [super initWithFrame:frameRect];
	
	if(self != nil)
	{
		self.verticalScroller.alphaValue =	0;
		self.hasVerticalScroller =		YES;
		self.horizontalScroller.alphaValue = 0;
		self.hasHorizontalScroller =	NO;
		self.borderType =				NSNoBorder;
		self.scrollerStyle =			NSScrollerStyleOverlay;
		self.drawsBackground =			NO;
		self.needsDisplay =				YES;
		self.translatesAutoresizingMaskIntoConstraints = NO;
		
		[RakScroller updateScrollers:self];
	}
	
	return self;
}

- (void) setFrame:(NSRect)frameRect
{
	if(self.isHidden && !forceUpdate)
	{
		_cachedFrame = frameRect;
	}
	else
	{
		[super setFrame:frameRect];
		[self updateScrollerState:frameRect];
	}
}

- (void) resizeAnimation:(NSRect)frameRect
{
	if(self.isHidden && !forceUpdate)
	{
		_cachedFrame = frameRect;
	}
	else
	{
		[self setFrameAnimated:frameRect];
		[self updateScrollerState:frameRect];
	}
}

- (void) setHidden : (BOOL) flag
{
	if(!flag && self.isHidden)
	{
		forceUpdate = YES;
		[self setFrame:_cachedFrame];
		forceUpdate = NO;
	}
	else if(flag && !self.isHidden)
	{
		_cachedFrame = [super frame];
	}
	
	[super setHidden:flag];
}

- (NSRect) frame
{
	return self.isHidden ? _cachedFrame : [super frame];
}

- (void) updateScrollerState : (NSRect) frame
{
	self.hasVerticalScroller = ((RakView *) self.documentView).bounds.size.height > frame.size.height;
}

- (void) scrollWheel:(NSEvent *)event
{
	//The second part bloc moves when there is no scroller on this direction
	if(self.scrollingDisabled || ((event.scrollingDeltaX == 0 || !self.hasHorizontalScroller) && (event.scrollingDeltaY == 0 || !self.hasVerticalScroller)))
		[self.nextResponder scrollWheel:event];
	
#if 0
	else if(self.horizontalScrollingEnabled || ![event scrollingDeltaX])
		[super scrollWheel:event];
	
	else if([event scrollingDeltaY] >= 1)
	{
		BOOL type = [event hasPreciseScrollingDeltas] ? kCGScrollEventUnitPixel : kCGScrollEventUnitLine;
		CGEventRef cgEvent = CGEventCreateScrollWheelEvent(NULL, type, 1, [event scrollingDeltaY], 0);
		event = [NSEvent eventWithCGEvent:cgEvent];
		CFRelease(cgEvent);
		[super scrollWheel:event];
	}
#endif
	else
		[super scrollWheel:event];
}

- (void) setBackgroundColor:(RakColor *)backgroundColor
{
	[super setBackgroundColor:backgroundColor];

	((RakScroller *) self.verticalScroller).backgroundColorToReplicate = backgroundColor;
	((RakScroller *) self.horizontalScroller).backgroundColorToReplicate = backgroundColor;
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		[super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
}

#pragma mark - Redirecting clic

- (void) mouseDown:(nonnull NSEvent *)theEvent
{
	if(!_redirectClicToContentView || self.documentView == nil || redirectingClic)
		return [super mouseDown:theEvent];

	redirectingClic = YES;
	[self.documentView mouseDown:theEvent];
	redirectingClic = NO;
}

- (void) mouseUp:(nonnull NSEvent *)theEvent
{
	if(!_redirectClicToContentView || self.documentView == nil || redirectingClic)
		return [super mouseUp:theEvent];

	redirectingClic = YES;
	[self.documentView mouseUp:theEvent];
	redirectingClic = NO;
}

@end