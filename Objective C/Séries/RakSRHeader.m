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

#define HEIGHT_ADDITIONNAL_ROW (TAG_BUTTON_HEIGHT + TAG_RAIL_INTER_RAIL_BORDER)

@implementation RakSRHeader

- (instancetype) initWithFrame : (NSRect) frameRect : (BOOL) haveFocus
{
	self = [super initWithFrame : [self frameFromParent:frameRect]];
	
	if(self != nil)
	{
		_prefUIOpen = NO;
		_haveFocus = haveFocus;
		_height = _bounds.size.height;
		
		[self initView];
		
		if(tagRail.nbRow != 1)
			[self setFrame:frameRect];
		
		_noAnimation = YES;
		[self updateFocus:haveFocus ? TAB_SERIES : TAB_CT];
		_noAnimation = NO;
		
		[Prefs registerForChange:self forType:KVO_THEME];
	}
	
	return self;
}

- (void) dealloc
{
	[Prefs deRegisterForChange:self forType:KVO_THEME];
}

- (void) initView
{
	NSRect frame = self.bounds;
	
	preferenceButton = [RakButton allocImageWithBackground: @"settings" : RB_STATE_STANDARD : self : @selector(gogoWindow)];
	if(preferenceButton != nil)
	{
		preferenceButton.hidden = [self shouldHidePrefButton];
		preferenceButton.alphaValue = !preferenceButton.isHidden;

		[preferenceButton setFrameSize:NSMakeSize(26, 26)];
		[preferenceButton.cell setActiveAllowed:NO];
		[preferenceButton setFrameOrigin: NSMakePoint(SR_PREF_BUTTON_BORDERS - ((RakButtonCell*)preferenceButton.cell).cellSize.width / 2, RBB_TOP_BORDURE)];
		
		[self addSubview:preferenceButton];
	}
	
#ifdef SEVERAL_VIEWS
	displayType = [RakButtonMorphic allocImages:@[@"grille", @"repo", @"list"] :RB_STATE_STANDARD :self :@selector(displayTypeSwitch)];
	if(displayType != nil)
	{
		[displayType setFrameOrigin: NSMakePoint([self preferenceButtonWidth], RBB_TOP_BORDURE)];
		[self addSubview:displayType];
	}
#endif
	
	storeSwitch = [RakButton allocWithText:NSLocalizedString(@"PROJ-STORE", nil)];
	if(storeSwitch != nil)
	{
		storeSwitch.target = self;
		storeSwitch.action = @selector(installOnlyToogle);
		
		[storeSwitch setFrameSize:NSMakeSize(storeSwitch.bounds.size.width, preferenceButton.bounds.size.height)];
		
		storeSwitch.hasBorder = NO;
		[storeSwitch setButtonType:NSOnOffButton];
		[storeSwitch.cell setActiveAllowed:YES];
		
		[storeSwitch triggerBackground];
#ifdef SEVERAL_VIEWS
		[storeSwitch setFrameOrigin: NSMakePoint(NSMaxX(displayType.frame) + SR_HEADER_INTERBUTTON_WIDTH, RBB_TOP_BORDURE)];
#else
		[storeSwitch setFrameOrigin: NSMakePoint([self preferenceButtonWidth], RBB_TOP_BORDURE)];
#endif
		
		if(!_haveFocus)
		{
			storeSwitch.hidden = YES;
			storeSwitch.alphaValue = 0;
		}
		
		[self addSubview:storeSwitch];
		
		_separatorX = SR_HEADER_INTERBUTTON_WIDTH + NSMaxX(storeSwitch.frame);
	}
#ifdef SEVERAL_VIEWS
	else if(displayType != nil)
		_separatorX = SR_HEADER_INTERBUTTON_WIDTH + NSMaxX(displayType.frame);
#endif
	else
		_separatorX = SR_HEADER_INTERBUTTON_WIDTH + NSMaxX(preferenceButton.frame);
	
	NSRect searchButtonFrame = [self searchButtonFrame : frame];
	
	tagRail = [[RakSRTagRail alloc] initWithFrame: [self railFrame:frame] : searchButtonFrame.origin.x];
	if(tagRail != nil)
	{
		if(!_haveFocus)
		{
			tagRail.hidden = YES;
			tagRail.alphaValue = 0;
		}
		
		[self addSubview:tagRail];
	}
	
	search = [[RakSRSearchBar alloc] initWithFrame:searchButtonFrame : SEARCH_BAR_ID_MAIN];
	if(search != nil)
	{
		if(!_haveFocus)
		{
			search.hidden = YES;
			search.alphaValue = 0;
		}
		[self addSubview:search];
	}
	
	winController = [[PrefsUI alloc] init];
	[winController setAnchor:preferenceButton];
	
	backButton = [[RakBackButton alloc] initWithFrame:[self backButtonFrame : frame] : NO];
	if(backButton != nil)
	{
		[backButton setTarget:self];
		[backButton setAction:@selector(backButtonClicked)];
		[backButton setHidden:_haveFocus];
		[self addSubview:backButton];
	}
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	
	[self setNeedsDisplay:YES];
}

- (void) drawRect:(NSRect)dirtyRect
{
	if(_haveFocus)
	{
		dirtyRect.size.height += SRSEARCHTAB_DEFAULT_HEIGHT;
		
		CGFloat border = dirtyRect.size.height / 5 ;
		
		[[self separatorColor] setFill];
		NSRectFill(NSMakeRect(_separatorX, border, 1, dirtyRect.size.height - 2 * border));
	}
}

- (BOOL) isFlipped
{
	return YES;
}

#pragma mark - Resizing

- (void) _resize : (NSRect) frameRect : (BOOL) animation
{
	NSRect searchFrame;
	frameRect = [self frameFromParent:frameRect];
	
	_height = frameRect.size.height;
	if(animation)
	{
		[self.animator setFrame:frameRect];
		frameRect.origin = NSZeroPoint;
		
		if(_haveFocus)
		{
			searchFrame = [self searchButtonFrame:frameRect];
			[search resizeAnimation : searchFrame];
			
			tagRail.baseSearchBar = searchFrame.origin.x;
			[tagRail resizeAnimation : [self railFrame:frameRect]];
		}
		else
		{
			[backButton resizeAnimation : [self backButtonFrame:frameRect]];
		}
	}
	else
	{
		[super setFrame:frameRect];
		
		if(_haveFocus)
		{
			searchFrame = [self searchButtonFrame:frameRect];
			[search setFrame : searchFrame];
			
			tagRail.baseSearchBar = searchFrame.origin.x;
			[tagRail setFrame: [self railFrame:frameRect]];
		}
		else
		{
			[backButton setFrame : [self backButtonFrame:frameRect]];
		}
	}
}

- (void) setFrame:(NSRect)frameRect
{
	[self _resize:frameRect :NO];
}

- (void) resizeAnimation : (NSRect) frameRect
{
	[self _resize:frameRect :YES];
}

- (NSRect) frameFromParent : (NSRect) parentFrame
{
	uint nbRow = tagRail != nil ? tagRail.nbRow - 1 : 0;
	
	parentFrame.origin = NSZeroPoint;
	parentFrame.size.height = SR_HEADER_HEIGHT_SINGLE_ROW + nbRow * HEIGHT_ADDITIONNAL_ROW - SRSEARCHTAB_DEFAULT_HEIGHT;
	
	return parentFrame;
}

- (NSRect) backButtonFrame : (NSRect) frame
{
	frame.origin.y = (frame.size.height + SRSEARCHTAB_DEFAULT_HEIGHT) / 2 - RBB_BUTTON_HEIGHT / 2;
	frame.size.height = RBB_BUTTON_HEIGHT;
	frame.origin.x = [self preferenceButtonWidth];
	frame.size.width -= frame.origin.x;
	
	return frame;
}

- (NSRect) searchButtonFrame : (NSRect) frame
{
	frame.origin.x = frame.size.width - SR_HEADER_INTERBUTTON_WIDTH;
	frame.origin.y = SR_HEADER_HEIGHT_SINGLE_ROW / 2;
	
	frame.size.height = SR_SEARCH_FIELD_HEIGHT;
	frame.origin.y -= frame.size.height / 2;
	
	frame.size.width = 250;
	frame.origin.x -= frame.size.width;
	
	return frame;
}

- (NSRect) railFrame : (NSRect) frame
{
	frame.size.height -= 2 * RBB_TOP_BORDURE;
	frame.origin.y = RBB_TOP_BORDURE;
	
	frame.origin.x = _separatorX + SR_HEADER_INTERBUTTON_WIDTH;
	frame.size.width -= frame.origin.x + 10;
	
	return frame;
}

- (void) nbRowRailsChanged
{
	[self setFrame:self.superview.bounds];
}

- (BOOL) shouldHidePrefButton
{
	return getMainThread() != TAB_READER;
}

- (CGFloat) preferenceButtonWidth
{
	if([self shouldHidePrefButton])
		return SR_HEADER_INTERBUTTON_WIDTH;
	
	return NSMaxX(preferenceButton.frame) + SR_HEADER_INTERBUTTON_WIDTH;
}

#pragma mark - Color

- (NSColor *) separatorColor
{
	return [Prefs getSystemColor:COLOR_INACTIVE :nil];
}

#pragma mark - Preference UI

- (void) gogoWindow
{
	_prefUIOpen = YES;
	[winController showPopover];
}

#pragma mark - Interface

- (void) updateFocus : (uint) mainThread
{
	_haveFocus = mainThread == TAB_SERIES;
	
	if(_haveFocus)
	{
#ifdef SEVERAL_VIEWS
		displayType.hidden = NO;
#endif
		tagRail.hidden = search.hidden = storeSwitch.hidden = NO;
	}
	else
		backButton.hidden = NO;
	
	preferenceButton.hidden = mainThread != TAB_READER;
	
	if(_noAnimation)
	{
		backButton.alphaValue = !_haveFocus;
		preferenceButton.alphaValue = mainThread == TAB_READER;
#ifdef SEVERAL_VIEWS
		displayType.alphaValue = _haveFocus;
#endif
		storeSwitch.alphaValue = _haveFocus;
		tagRail.alphaValue = _haveFocus;
		search.alphaValue = _haveFocus;
	}
	else
	{
		backButton.animator.alphaValue = !_haveFocus;
		preferenceButton.animator.alphaValue = mainThread == TAB_READER;
#ifdef SEVERAL_VIEWS
		displayType.animator.alphaValue = _haveFocus;
#endif
		storeSwitch.animator.alphaValue = _haveFocus;
		tagRail.animator.alphaValue = _haveFocus;
		search.animator.alphaValue = _haveFocus;
	}
}

- (void) cleanupAfterFocusChange
{
#ifdef SEVERAL_VIEWS
	for(NSView * view in @[backButton, displayType, tagRail, search, preferenceButton])
#else
		for(NSView * view in @[backButton, tagRail, search, preferenceButton])
#endif
		{
			if(view.alphaValue == 0)
				view.hidden = YES;
		}
}

- (void) backButtonClicked
{
	[RakTabView broadcastUpdateFocus : TAB_SERIES];
}

- (void) installOnlyToogle
{
	[[NSNotificationCenter defaultCenter] postNotificationName:NOTIFICATION_INSTALLED_ONLY_STAB object:@(storeSwitch.state == NSOnState)];
}

- (void) focusSearchField
{
	if(self.window.firstResponder != search)
		[self.window makeFirstResponder:search];
}

#ifdef SEVERAL_VIEWS
- (void) displayTypeSwitch
{
	uint activeCell = displayType.activeCell = (displayType.activeCell + 1) % 3;
	
	if(activeCell == SR_CELLTYPE_GRID)
		NSLog(@"Would update to grid");
	else if(activeCell == SR_CELLTYPE_REPO)
		NSLog(@"Would update to repo view");
	else if(activeCell == SR_CELLTYPE_LIST)
		NSLog(@"Would update to list");
	else
		NSLog(@"Would fail to update");
}
#endif

@end
