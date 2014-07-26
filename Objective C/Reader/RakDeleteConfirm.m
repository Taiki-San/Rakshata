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

@interface RakDeleteButtonCell : RakButtonCell

@end

@implementation RakDeleteConfirm

- (instancetype) autoInit
{
	return [self initWithFrame: NSMakeRect(0, 0, 170, 173)];
}

- (void) launchPopover : (NSView *) anchor : (Reader*) receiver
{
	_remind = NO;
	_isTome = receiver.isTome;
	_receiver = receiver;
	_anchor = anchor;

	[Prefs getCurrentTheme:self];
	
	[self internalInit: anchor : NSMakeRect(0, 0, _anchor.frame.size.width, 0) : YES];
}

- (void) setupView
{
	NSString * string = nil;
	
	if(_isTome)
		string = @"Vous vous apprêtez\nà supprimer un tome.\nPour y accéder, il vous\nfaudra le télécharger\nà nouveau.";
	else
		string = @"Vous vous apprêtez à\nsupprimer un chapitre.\nPour y accéder, il vous\nfaudra le télécharger\nà nouveau.";
	
	RakText * contentText = [[[RakText alloc] initWithText:self.frame :string :[Prefs getSystemColor : GET_COLOR_DANGER_POPOVER_TEXT_COLOR:nil]] autorelease];
	[contentText setAlignment:NSCenterTextAlignment];
	[contentText setFont:[NSFont fontWithName:[Prefs getFontName:GET_FONT_RD_BUTTONS] size:13]];
	[contentText sizeToFit];
	[self addSubview : contentText];
	[contentText setFrameOrigin:NSMakePoint(self.frame.size.width / 2 - contentText.frame.size.width / 2 , self.frame.size.height - 10 - contentText.frame.size.height)];
	
	RakDeleteSegmentedControl * button = [[[RakDeleteSegmentedControl alloc] initWithData : NSMakeRect(0, 0, self.frame.size.width, contentText.frame.origin.y - 10) : @"Confirmer" : @"Non"] autorelease];
	[button setTarget:self];
	[button setAction:@selector(buttonClicked:)];
	[self addSubview:button];
	
	RakDeleteButton * buttonRemind = [[RakDeleteButton allocWithText:@"S'en souvenir" :NSMakeRect(button.frame.origin.x, button.frame.origin.y - 5 - button.frame.size.height, button.frame.size.width, button.frame.size.height)] autorelease];
	[buttonRemind setTarget:self];
	[buttonRemind setAction:@selector(remindSwitched:)];
	[self addSubview:buttonRemind];
}

- (void) configurePopover:(INPopoverController *)internalPopover
{
	[super configurePopover:internalPopover];
	
	internalPopover.closesWhenApplicationBecomesInactive = YES;
	internalPopover.closesWhenPopoverResignsKey = YES;
}

- (void) additionalUpdateOnThemeChange
{
	[super additionalUpdateOnThemeChange];
	
	for(RakText * view in self.subviews)
	{
		if([view class] == [RakText class])
		{
			[view setTextColor:[Prefs getSystemColor : GET_COLOR_DANGER_POPOVER_TEXT_COLOR:nil]];
			break;
		}
	}
}

//Colors

- (NSColor *) popoverBorderColor
{
	return [Prefs getSystemColor: GET_COLOR_DANGER_POPOVER_BORDER :nil];
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

//Toolbox

- (void) locationUpdated : (NSRect) frame : (BOOL) animated
{
	[self updateOrigin : frame.origin : animated];
}

//Payload

- (void) buttonClicked : (RakQuerySegmentedControl*) sender
{
	if([sender selectedSegment] == 0)
		[_receiver deleteElement];
	
	[popover closePopover];
}

- (void) remindSwitched : (RakButton*) sender
{
	_remind = !_remind;
	
	if (sender != nil && [sender class] == [RakDeleteButton class] && sender.cell != nil && [sender.cell class] == [RakDeleteButtonCell class])
		((RakButtonCell*)sender.cell).forceHighlight = _remind;
}

@end

@interface RakDeleteSegmentedControlCell : RakSegmentedButtonCell

@end

@implementation RakDeleteSegmentedControlCell

- (NSColor *) getFontColor : (uint) cellID
{
	if([self isSelectedForSegment:cellID])
		return [Prefs getSystemColor : GET_COLOR_DANGER_POPOVER_TEXT_COLOR_SELECTED:nil];
	else
		return [Prefs getSystemColor : GET_COLOR_DANGER_POPOVER_TEXT_COLOR:nil];
}

@end

@implementation RakDeleteSegmentedControl

+ (Class) cellClass
{
	return [RakDeleteSegmentedControlCell class];
}

@end

@implementation RakDeleteButtonCell

- (NSColor *) getFontColor
{
	if([self isHighlighted] || self.forceHighlight)
		return [Prefs getSystemColor : GET_COLOR_DANGER_POPOVER_TEXT_COLOR_SELECTED:nil];
	else
		return [Prefs getSystemColor : GET_COLOR_DANGER_POPOVER_TEXT_COLOR:nil];
}

@end

@implementation RakDeleteButton

+ (Class) cellClass
{
	return [RakDeleteButtonCell class];
}

@end
