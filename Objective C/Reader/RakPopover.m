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

@implementation RakPopoverWrapper

- (instancetype) init : (NSView *) contentView
{
	self = [super init];
	
	if(self != nil)
	{
		self.anchor = nil;
		self.anchorFrame = NSZeroRect;
		self.direction = INPopoverArrowDirectionLeft;
		
		NSViewController * controller = [[NSViewController alloc] init];
		controller.view = contentView;
		_popover = [[INPopoverController alloc] initWithContentViewController:controller];
		_popover.closesWhenApplicationBecomesInactive = NO;
		_popover.closesWhenPopoverResignsKey = NO;
	}
	
	return self;
}

- (void) togglePopover : (NSRect) frame
{
	if(_popover.popoverIsVisible)
		[_popover closePopover:nil];
	else
	{
		if(NSIsEmptyRect(self.anchorFrame))
			self.anchorFrame = [self.anchor bounds];
		
		[_popover presentPopoverFromRect:(NSEqualRects(frame, NSZeroRect) ? self.anchorFrame : frame) inView:self.anchor preferredArrowDirection:self.direction anchorsToPositionView:YES];
	}
}

- (void) additionalConfiguration : (id) target
{
	[target performSelector:@selector(configurePopover:) withObject:_popover];
}

- (void) updatePosition : (NSPoint) origin : (BOOL) animated
{
	if(_popover != nil && _popover.popoverWindow != nil)
	{
		NSSize size = _popover.popoverWindow.frame.size;
		origin.x -= size.width / 2;
		
		[_popover.popoverWindow setFrame:(NSRect) {origin, size} display:YES animate:animated];
	}
}

- (void)setDelegate:(id <INPopoverControllerDelegate>) delegate
{
	[_popover setDelegate:delegate];
}

- (void) closePopover
{
	[_popover closePopover:self];
}

- (void) clearMemory
{
	_popover.contentViewController.view = nil;
	_popover.contentViewController = nil;
}

@end

@implementation RakPopoverView

- (void) internalInit : (id) anchor : (NSRect) baseFrame : (BOOL) wantAdditionalConfig
{
	[Prefs getCurrentTheme:self];
	prefsRegistered = YES;
	
	[self setWantsLayer:YES];
	[self.layer setCornerRadius:4];
	[self.layer setBorderWidth:1];
	
	NSColor * color;
	if((color = [self backgroundColor]) != nil)
		[self.layer setBackgroundColor : color.CGColor];
	
	if((color = [self borderColor]) != nil)
		[self.layer setBorderColor : color.CGColor];
	
	[self setupView];
	
	popover = [[RakPopoverWrapper alloc] init : self];
	popover.anchor = anchor;
	popover.direction = [self arrowDirection];
	
	if(wantAdditionalConfig)
	{
		additionalConfigRequired = YES;
		[popover additionalConfiguration : self];
	}
	
	[popover togglePopover : baseFrame];
	[popover setDelegate : self];
}

- (void) dealloc
{
	if(prefsRegistered)
		[Prefs deRegisterForChanges:self];
}

#pragma mark - Drawing

- (INPopoverArrowDirection) arrowDirection
{
	return INPopoverArrowDirectionDown;
}

- (void) setupView
{
	
}

- (void) configurePopover : (INPopoverController*) internalPopover
{
	internalPopover.borderColor = [[self popoverBorderColor] colorWithAlphaComponent:0.8];
	internalPopover.color = [self popoverArrowColor];
	internalPopover.borderWidth = 4;
}

#pragma mark - Colors

- (NSColor *) popoverBorderColor
{
	return [Prefs getSystemColor:GET_COLOR_INACTIVE:nil];
}

- (NSColor *) popoverArrowColor
{
	return [Prefs getSystemColor:GET_COLOR_BACKGROUND_TABS:nil];
}

- (NSColor *) borderColor
{
	return [Prefs getSystemColor:GET_COLOR_BORDER_TABS:nil];
}

- (NSColor *) backgroundColor
{
	return 	[Prefs getSystemColor:GET_COLOR_BACKGROUND_TABS:nil];
}

#pragma mark - Toolbox

- (void) updateOrigin : (NSPoint) origin : (BOOL) animated
{
	[popover updatePosition:origin :animated];
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return;
	
	[self.layer setBorderColor:[self popoverBorderColor].CGColor];
	[self.layer setBackgroundColor:[self popoverArrowColor].CGColor];
	
	[self additionalUpdateOnThemeChange];
	
	[self setNeedsDisplay:YES];
	
	if(additionalConfigRequired)
		[popover additionalConfiguration:self];
}

- (void) additionalUpdateOnThemeChange
{
	
}

- (void) closePopover
{
	[popover closePopover];
}

- (void) popoverWillClose:(INPopoverController *)popover
{
	if([_anchor respondsToSelector : @selector(stopUsePopover)])
		[_anchor performSelector : @selector(stopUsePopover) withObject : nil];
	
	if([_anchor respondsToSelector : @selector(removePopover)])
		[_anchor performSelectorOnMainThread : @selector(removePopover) withObject : nil waitUntilDone : NO];		//Let the event chain end before signaling a new popover can appear
}

- (void)popoverDidClose:(INPopoverController *)discarded;
{
	[popover clearMemory];
}

@end
