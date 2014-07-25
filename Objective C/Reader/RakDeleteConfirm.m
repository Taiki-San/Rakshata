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

@implementation RakDeleteConfirm

- (instancetype) autoInit
{
	return [self initWithFrame: NSMakeRect(0, 0, 170, 210)];
}

- (void) launchPopover : (NSView *) anchor : (Reader*) receiver : (BOOL) isTome
{
	_isTome = isTome;
	_receiver = receiver;
	_anchor = anchor;

	[Prefs getCurrentTheme:self];
	
	[self internalInit: anchor : NSMakeRect(0, 0, _anchor.frame.size.width, 0) : YES];
}

- (void) setupView
{
	NSString * string = nil, *complement = _isTome ? @" tome,\n" : @"\nchapitre,";
	
	string = [NSString stringWithFormat:@"Attention: vous vous\napprêtez à supprimer\ndéfinitivement un%@pour le relire, vous\naurez à le télécharger\nde nouveau, en\nêtes vous sûr?", complement];
	
	RakText * contentText = [[[RakText alloc] initWithText:self.frame :string :[Prefs getSystemColor : GET_COLOR_ACTIVE:nil]] autorelease];
	[contentText setFont:[NSFont fontWithName:[Prefs getFontName:GET_FONT_RD_BUTTONS] size:13]];
	[contentText sizeToFit];
	[self addSubview : contentText];
	[contentText setFrameOrigin:NSMakePoint(10 , self.frame.size.height - 10 - contentText.frame.size.height)];
	
	RakQuerySegmentedControl * button = [[[RakQuerySegmentedControl alloc] initWithFrame:NSMakeRect(0, 0, self.frame.size.width, contentText.frame.origin.y - 15)] autorelease];
	[button setTarget:self];
	[button setAction:@selector(buttonClicked:)];
	[self addSubview:button];
	
	RakButton * buttonRemind = [[RakButton allocWithText:@"Se souvenir" :NSMakeRect(button.frame.origin.x, button.frame.origin.y - 5 - button.frame.size.height, button.frame.size.width, button.frame.size.height)] autorelease];
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

//Colors

- (NSColor *) popoverBorderColor
{
	return [NSColor colorWithSRGBRed:255/255.0f green:38/255.0f  blue:0 alpha:1];
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

}

@end
