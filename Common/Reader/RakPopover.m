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

- (instancetype) init : (RakView *) contentView
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
	[target configurePopover:_popover];
}

- (void) justCallSuperConfigure : (RakPopoverView *) view
{
	[view directConfiguration:_popover];
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
	_popover.contentViewController = nil;
}

@end

@implementation RakPopoverView

- (void) internalInit : (id) anchor : (NSRect) baseFrame : (BOOL) wantAdditionalConfig
{
	[Prefs registerForChange:self forType:KVO_THEME];
	prefsRegistered = YES;
	
	[self setWantsLayer:YES];
	[self.layer setCornerRadius:4];
	[self.layer setBorderWidth:1];
	
	RakColor * color;
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
	else
		[popover justCallSuperConfigure : self];

	[popover togglePopover : baseFrame];
	[popover setDelegate : self];
}

- (void) setFrame:(NSRect)frame
{
	[self setFrameOrigin:frame.origin];
}

- (void) dealloc
{
	if(prefsRegistered)
		[Prefs deRegisterForChange:self forType:KVO_THEME];
}

#pragma mark - Drawing

- (INPopoverArrowDirection) arrowDirection
{
	return INPopoverArrowDirectionDown;
}

- (void) setupView
{
	
}

- (void) directConfiguration : (INPopoverController*) internalPopover
{
	internalPopover.borderColor = [[self popoverBorderColor] colorWithAlphaComponent:0.8];
	internalPopover.color = [self popoverArrowColor];
	internalPopover.borderWidth = 4;
}

- (void) configurePopover : (INPopoverController*) internalPopover
{
	[self directConfiguration:internalPopover];
}

#pragma mark - Colors

- (RakColor *) popoverBorderColor
{
	return [Prefs getSystemColor:COLOR_INACTIVE];
}

- (RakColor *) popoverArrowColor
{
	return [Prefs getSystemColor:COLOR_TABS_BACKGROUND];
}

- (RakColor *) borderColor
{
	return [Prefs getSystemColor:COLOR_TABS_BORDER];
}

- (RakColor *) backgroundColor
{
	return 	[Prefs getSystemColor:COLOR_TABS_BACKGROUND];
}

#pragma mark - Toolbox

- (void) updateOrigin : (NSPoint) origin : (BOOL) animated
{
	[popover updatePosition:origin :animated];
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	
	[self.layer setBorderColor:[self popoverBorderColor].CGColor];
	[self.layer setBackgroundColor:[self popoverArrowColor].CGColor];
	
	[self additionalUpdateOnThemeChange];
	
	[self setNeedsDisplay:YES];
	
	if(additionalConfigRequired)
		[popover additionalConfiguration:self];
	else
		[popover justCallSuperConfigure : self];
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
