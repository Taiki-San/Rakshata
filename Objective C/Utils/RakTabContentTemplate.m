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
	
	self.autoresizesSubviews = NO;
	self.wantsLayer = YES;
	self.layer.backgroundColor = [self getBackgroundColor].CGColor;
	self.layer.cornerRadius = 5.0;
	
	[Prefs getCurrentTheme:self];		//register to changes
	isFocusDrop = NO;
}

- (NSString *) getContextToGTFO
{
	return STATE_EMPTY;
}

- (BOOL) mouseDownCanMoveWindow
{
	return YES;
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return;
	
	self.layer.backgroundColor = [self getBackgroundColor].CGColor;
	[self setNeedsDisplay:YES];
}


#pragma mark - Focus drop

- (void) setFocusDrop : (BOOL) isFocus
{
	if(isFocusDrop == isFocus)
		return;
	
	isFocusDrop = isFocus;
	[self updateViewForFocusDrop];
	[self setNeedsDisplay:YES];
}

- (BOOL) isFocusDrop	{	return isFocusDrop;		}

- (void) updateViewForFocusDrop
{
	
}

#pragma mark - Hooks to critical routines

- (void) setFrame:(NSRect)frameRect
{
	[super setFrame:frameRect];
	
	[self setFrameInternalViews:_bounds];
}

- (void) resizeAnimation : (NSRect)frameRect
{
	[self.animator setFrame:frameRect];
	
	frameRect.origin = NSZeroPoint;
	[self resizeAnimationInternalViews:frameRect];
}

- (void) setFrameInternalViews : (NSRect) newBound
{
	
}

- (void) resizeAnimationInternalViews : (NSRect) newBound
{
	
}

- (void) drawRect:(NSRect)dirtyRect
{
	[super drawRect:dirtyRect];
	
	if (isFocusDrop)
		[self drawFocusRing];
}

- (void) drawFocusRing
{
	
}

#pragma mark - Color

- (NSColor*) getBackgroundColor
{
	return [NSColor clearColor];
}

- (NSColor*) getBorderColor
{
	return [Prefs getSystemColor:GET_COLOR_BORDERS_COREVIEWS:nil];
}

#pragma mark - Behavior when leaving a focus

- (void) focusViewChanged : (uint) newMainThread
{
	if(mainThread == newMainThread)
	{
		_ignoredPreviousContextChange = YES;
		return;
	}
	
	mainThread = newMainThread;
	_ignoredPreviousContextChange = NO;
	_animatedContextChange = YES;
	
	self.serieViewHidden	= (newMainThread & TAB_SERIES) == 0;
	self.CTViewHidden		= (newMainThread & TAB_CT) == 0;
	self.readerViewHidden	= (newMainThread & TAB_READER) == 0;
	self.layer.backgroundColor = [self getBackgroundColor].CGColor;
	
	_animatedContextChange = NO;
}

- (void) cleanupFocusViewChange
{
	if(_ignoredPreviousContextChange)
		return;
}

- (void) setCTViewHidden : (BOOL) CTViewHidden	{	_CTViewHidden = CTViewHidden;	}
- (BOOL) CTViewHidden	{	return _CTViewHidden;	}

- (void) setReaderViewHidden : (BOOL) readerViewHidden	{	_readerViewHidden = readerViewHidden;	}
- (BOOL) readerViewHidden	{	return _readerViewHidden;	}

@end
