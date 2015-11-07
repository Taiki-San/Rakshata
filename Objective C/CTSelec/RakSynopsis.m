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

enum
{
	COMPACT_OFFSET = 3
};

@implementation RakSynopsis

- (instancetype) initWithSynopsis : (charType *) synopsis : (NSRect) frame : (BOOL) haveScroller
{
	_haveScroller = haveScroller;
	
	self = [self initWithFrame:[self frameFromParent:frame]];
	
	if(self != nil)
	{
		self.wantsLayer = YES;
		self.layer.cornerRadius = 4;
		[Prefs registerForChange:self forType:KVO_THEME];
		
		if([self setStringToSynopsis : getStringForWchar(synopsis)])
			[self updateFrame : frame : NO];
	}
	
	return self;
}

- (void) updateSynopsis : (charType *) synopsis
{
	if(placeholderString && synopsis[0] == 0)
		return;
	
	NSString * localSynopsis = getStringForWchar(synopsis);
	
	[NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
		
		[context setDuration:CT_HALF_TRANSITION_ANIMATION];
		[self.animator setAlphaValue:0.0];
		
	} completionHandler:^{
		
		[self setStringToSynopsis : localSynopsis];
		
		[NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
			
			[context setDuration:CT_HALF_TRANSITION_ANIMATION];
			[self.animator setAlphaValue:1.0];
			
		} completionHandler:^{
			
		}];
	}];
}

- (BOOL) setStringToSynopsis : (NSString *) synopsis
{
	BOOL needPostProcessing = NO;
	
	//Create _synopsis object
	if(_synopsis == nil)
	{
		needPostProcessing = YES;
		placeholderString = NO;
		
		_synopsis = [[RakText alloc] init];
		if(_synopsis == nil)
			return NO;
		
		_synopsis.fixedWidth = self.bounds.size.width - [RakScroller width] - (_haveScroller ? COMPACT_OFFSET : 0);
		[_synopsis setFrameOrigin : NSZeroPoint];
		
		[_synopsis setAlignment:NSTextAlignmentJustified];
		[_synopsis.cell setWraps:YES];
		
		[_synopsis setTextColor : [Prefs getSystemColor:COLOR_ACTIVE : nil]];
	}
	
	//Update text
	if(![synopsis isEqualTo:@""])
	{
		placeholderString = NO;
		
		[_synopsis setStringValue : synopsis];
		
		if(!_scrollview.hasVerticalScroller)
			_scrollview.hasVerticalScroller = YES;
	}
	else
	{
		placeholderString = YES;
		if(_placeholder == nil)
		{
			_placeholder = [[RakText alloc] initWithText:self.bounds : NSLocalizedString(@"CT-NO-SYNOPSIS", nil) : [Prefs getSystemColor:COLOR_ACTIVE : nil]];
			if(_placeholder != nil)
			{
#ifdef LARGE_FONT_FOR_PLACEHOLDERS
				[_placeholder setFont: [NSFont systemFontOfSize:15]];
#endif
				[_placeholder sizeToFit];
			}
		}
	}
	
	//Hide the unused view
	if(_synopsis != nil)
		[_synopsis setHidden : placeholderString];
	
	if(_placeholder != nil)
		[_placeholder setHidden: !placeholderString];
	
	//Postprocessing around the scrollview
	if(needPostProcessing && ![self postProcessScrollView])
		return NO;
	
	//Update the placeholder string
	if(placeholderString)
	{
		if(_placeholder.superview == nil)
			[_scrollview addSubview:_placeholder];
		
		[_placeholder setFrameOrigin : [self placeholderOrigin : _scrollview.bounds]];
	}
	
	[self updateScrollViewState];
	
	return YES;
}

- (BOOL) postProcessScrollView
{
	return [self generatedScrollView:[self frameForContent : _bounds]];
}

- (BOOL) generatedScrollView : (NSRect) frame
{
	_scrollview = [[RakListScrollView alloc] initWithFrame:frame];
	if(_scrollview == nil)
	{
		_synopsis = nil;
		return NO;
	}
	
	if(!_haveScroller)
		_scrollview.verticalScroller.alphaValue = 0;
	
	_scrollview.documentView = _synopsis;
	[self addSubview:_scrollview];
	
	return YES;
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	
	if(_synopsis != nil)
		[_synopsis setTextColor : [Prefs getSystemColor:COLOR_ACTIVE : nil]];
	
	if(_placeholder != nil)
		[_placeholder setTextColor : [Prefs getSystemColor:COLOR_ACTIVE : nil]];
	
	[self setNeedsDisplay:YES];
}

- (BOOL) isFlipped	{	return YES;	}

#pragma mark - Resize

- (void) setFrame : (NSRect) frameRect
{
	[self updateFrame : frameRect : NO];
}

- (void) resizeAnimation : (NSRect) frameRect
{
	[self updateFrame : frameRect : YES];
}

- (void) updateFrame : (NSRect) frame : (BOOL) animated
{
	NSRect mainFrame = [self frameFromParent:frame];
	
	[self _updateFrame:mainFrame :animated];
	
	mainFrame.origin = NSZeroPoint;
	
	NSRect scrollviewRect = [self frameForContent : mainFrame];
	
	if(animated)
	{
		[_scrollview.animator setFrame: scrollviewRect];
		if(placeholderString)
			[_placeholder.animator setFrameOrigin : [self placeholderOrigin : scrollviewRect]];
	}
	else
	{
		[_scrollview setFrame: scrollviewRect];
		if(placeholderString)
		{
			[_placeholder setFrameOrigin : [self placeholderOrigin : scrollviewRect]];
		}
	}
	
	[self updateScrollViewState];
}

- (void) _updateFrame : (NSRect) mainFrame : (BOOL) animated
{
	if(_synopsis != nil)
	{
		const CGFloat newSynopsisWidth = mainFrame.size.width - [RakScroller width] - (_haveScroller ? COMPACT_OFFSET : 0);
		
		if(_synopsis.fixedWidth != newSynopsisWidth)
			_synopsis.fixedWidth = newSynopsisWidth;
	}
	
	//Update main frame
	if(animated)
		[self.animator setFrame : mainFrame];
	else
		[super setFrame : mainFrame];
	
	//Set up new frames
	mainFrame.origin = NSZeroPoint;
}

- (void) updateScrollViewState
{
	if(!placeholderString && _synopsis.bounds.size.height > _scrollview.bounds.size.height)
		_scrollview.scrollingDisabled = NO;
	
	else
		_scrollview.scrollingDisabled = YES;
}

#pragma mark - Position routines

- (NSRect) frameFromParent : (NSRect) parentFrame
{
	if(!_haveScroller)
	{
		parentFrame.origin.x = SYNOPSIS_BORDER / 2;
		parentFrame.size.width -= SYNOPSIS_BORDER;
	}
	
	parentFrame.size.height -= SYNOPSIS_TOP_BORDER_WIDTH;
	
	return parentFrame;
}

- (NSPoint) placeholderOrigin : (NSRect) scrollviewBounds
{
	scrollviewBounds.origin = NSZeroPoint;
	
	NSPoint origin = NSCenteredRect(scrollviewBounds, _placeholder.bounds);
	
	if(!_haveScroller)
		origin.x -= SYNOPSIS_MAIN_TEXT_BORDER / 2 + 15;
	
	return origin;
}

- (NSRect) frameForContent : (NSRect) mainBounds
{
	mainBounds.origin.y = SYNOPSIS_SPACING;
	mainBounds.size.height -= SYNOPSIS_SPACING;
	
	if(_synopsis == nil)
		mainBounds.size.width -= _haveScroller ? 0 : SYNOPSIS_MAIN_TEXT_BORDER;
	else
		mainBounds.size.width = _synopsis.fixedWidth + (_haveScroller ? [RakScroller width] + COMPACT_OFFSET : 0);
	
	if(!_haveScroller)
	{
		mainBounds.origin.x = SYNOPSIS_MAIN_TEXT_BORDER / 2;
		mainBounds.size.width += [RakScroller width];	//Scroller width
	}
	else
	{
		mainBounds.origin.x = COMPACT_OFFSET;
		mainBounds.size.width -= COMPACT_OFFSET;
	}
	
	return mainBounds;
}

#pragma mark - Background

- (void) setHaveBackground:(BOOL)haveBackground
{
	_haveBackground = haveBackground;
	
	if(_haveBackground)
		self.layer.backgroundColor = [self backgroundColor].CGColor;
	else
		self.layer.backgroundColor = [NSColor clearColor].CGColor;
}

- (NSColor *) backgroundColor
{
	return [Prefs getSystemColor:COLOR_BACKGROUND_REPO_LIST :nil];
}

@end
