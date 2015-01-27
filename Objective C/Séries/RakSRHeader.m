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

@implementation RakSRHeader

- (instancetype) initWithFrame : (NSRect) frameRect : (BOOL) haveFocus
{
	self = [super initWithFrame : [self frameFromParent:frameRect]];
	
	if(self != nil)
	{
		_responder = nil;
		_prefUIOpen = NO;
		_haveFocus = haveFocus;
		_height = self.bounds.size.height;
		
		[self initView];
	}
	
	return self;
}

- (void) initView
{
	preferenceButton = [RakButton allocImageWithBackground: @"parametre" : RB_STATE_STANDARD : self : @selector(gogoWindow)];
	if(preferenceButton != nil)
	{
		[preferenceButton.cell setHighlightAllowed:NO];
		[preferenceButton setFrameOrigin: NSMakePoint(SR_PREF_BUTTON_BORDERS - ((RakButtonCell*)preferenceButton.cell).cellSize.width / 2, RBB_TOP_BORDURE)];
		
		[self addSubview:preferenceButton];
	}
	
	displayType = [RakButtonMorphic allocImages:@[@"grille", @"repo", @"list"] :RB_STATE_STANDARD :self :@selector(displayTypeSwitch)];
	if(displayType != nil)
	{
		[displayType setFrameOrigin: NSMakePoint(NSMaxX(preferenceButton.frame) + SR_HEADER_INTERBUTTON_WIDTH, RBB_TOP_BORDURE)];
		[self addSubview:displayType];
	}
	
	storeSwitch = [RakButton allocWithText:@"Magasin" :self.bounds];
	if(storeSwitch != nil)
	{
		[storeSwitch sizeToFit];
		[storeSwitch setFrameSize:NSMakeSize(storeSwitch.bounds.size.width, displayType.bounds.size.height)];
		
		storeSwitch.hasBorder = NO;
		[storeSwitch setButtonType:NSOnOffButton];

		[storeSwitch triggerBackground];
		[storeSwitch setFrameOrigin: NSMakePoint(NSMaxX(displayType.frame) + SR_HEADER_INTERBUTTON_WIDTH, RBB_TOP_BORDURE)];

		[self addSubview:storeSwitch];
		
		_separatorX = SR_HEADER_INTERBUTTON_WIDTH + NSMaxX(storeSwitch.frame);
	}
	else if(displayType != nil)
		_separatorX = SR_HEADER_INTERBUTTON_WIDTH + NSMaxX(displayType.frame);
	else
		_separatorX = SR_HEADER_INTERBUTTON_WIDTH + NSMaxX(preferenceButton.frame);
	
	search = [[RakSRSearchBar alloc] initWithFrame:[self searchButtonFrame : self.bounds]];
	if(search != nil)
	{
		[self addSubview:search];
	}
	
	winController = [[PrefsUI alloc] init];
	[winController setAnchor:preferenceButton];
	
	backButton = [[RakBackButton alloc] initWithFrame:[self backButtonFrame : self.bounds] : NO];
	if(backButton != nil)
	{
		[backButton setTarget:self];
		[backButton setAction:@selector(backButtonClicked)];
		[backButton setHidden:_haveFocus];
		[self addSubview:backButton];
	}
}

- (void) drawRect:(NSRect)dirtyRect
{
	if(_haveFocus)
	{
		CGFloat border = dirtyRect.size.height / 8;
		
		[[self separatorColor] setFill];
		NSRectFill(NSMakeRect(_separatorX, border, 1, dirtyRect.size.height - 2 * border));
	}
}

#pragma mark - Resizing

- (void) _resize : (NSRect) frameRect : (BOOL) animation
{
	frameRect = [self frameFromParent:frameRect];
	
	_height = frameRect.size.height;
	if(animation)
	{
		[self.animator setFrame:frameRect];
		frameRect.origin = NSZeroPoint;
		
		[search resizeAnimation : [self searchButtonFrame:frameRect]];
		[backButton resizeAnimation : [self backButtonFrame:frameRect]];
	}
	else
	{
		[super setFrame:frameRect];

		[search setFrame : [self searchButtonFrame:frameRect]];
		[backButton setFrame : [self backButtonFrame:frameRect]];
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
	parentFrame.origin = NSZeroPoint;
	parentFrame.size.height = RBB_TOP_BORDURE + RBB_BUTTON_HEIGHT + RBB_TOP_BORDURE;
	
	return parentFrame;
}

- (NSRect) backButtonFrame : (NSRect) frame
{
	frame.origin.y = RBB_TOP_BORDURE;

	if(preferenceButton != nil)
	{
		frame.origin.x = preferenceButton.frame.origin.x + preferenceButton.frame.size.width;
		frame.size.width -= frame.origin.x;
		
		return frame;
	}
	else
	{
		frame.origin.x = RBB_BUTTON_POSX;
		return frame;
	}
}

- (NSRect) searchButtonFrame : (NSRect) frame
{
	frame.origin.y = frame.size.height / 2;
	frame.origin.x = SR_HEADER_INTERBUTTON_WIDTH;

	if(displayType != nil)
		frame.origin.x += NSMaxX(displayType.frame);
	else if(preferenceButton != nil)
		frame.origin.x += NSMaxX(preferenceButton.frame);
	
	frame.size.height = 22;
	frame.origin.y -= frame.size.height / 2;
	frame.origin.x += frame.size.width / 2 - 75;
	frame.size.width = 150;
	
	return frame;
}

#pragma mark - Color

- (NSColor *) separatorColor
{
	return [Prefs getSystemColor:GET_COLOR_INACTIVE :nil];
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
	[backButton setHidden: _haveFocus];
	[displayType setHidden : !_haveFocus];
}

- (void) backButtonClicked
{
	if(_responder != nil)
	{
		[_responder mouseDown:nil];
		[_responder mouseUp:nil];
	}
}

- (void) displayTypeSwitch
{
	uint activeCell = displayType.activeCell = (displayType.activeCell + 1) % 3;
	
	if(_responder != nil)
	{
		[_responder displayTypeUpdate:activeCell];
	}
}

@end
