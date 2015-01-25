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
	
	winController = [[PrefsUI alloc] init];
	[winController setAnchor:preferenceButton];
	
	backButton = [[RakBackButton alloc] initWithFrame:[self backButtonFrame : self.bounds] : NO];
	[backButton setTarget:self];
	[backButton setAction:@selector(backButtonClicked)];
	[backButton setHidden:!_haveFocus];
	[self addSubview:backButton];
}

#pragma mark - Resizing

- (void) _resize : (NSRect) frameRect : (BOOL) animation
{
	frameRect = [self frameFromParent:frameRect];
	
	_height = frameRect.size.height;
	if(animation)
	{
		[self.animator setFrame:frameRect];
		[preferenceButton.animator setFrameOrigin : NSMakePoint(preferenceButton.frame.origin.x, RBB_TOP_BORDURE)];
		[backButton resizeAnimation : [self backButtonFrame:frameRect]];
	}
	else
	{
		[super setFrame:frameRect];
		[preferenceButton setFrameOrigin : NSMakePoint(preferenceButton.frame.origin.x, RBB_TOP_BORDURE)];
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

#pragma mark - Preference UI

- (void) gogoWindow
{
	_prefUIOpen = YES;
	[winController showPopover];
}

#pragma mark - Interface

- (void) updateFocus : (uint) mainThread
{
	[backButton setHidden: mainThread == TAB_SERIES];
	[displayType setHidden : mainThread != TAB_SERIES];
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
