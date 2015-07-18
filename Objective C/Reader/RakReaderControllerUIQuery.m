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

@implementation RakReaderControllerUIQuery

- (instancetype) initWithData : (MDL*) tabMDL : (PROJECT_DATA) project : (BOOL) isTome : (int*) arraySelection : (uint) sizeArray
{
	if(tabMDL == nil || arraySelection == NULL || sizeArray == 0)
		return nil;
	
	_anchor = tabMDL;	_project = project;		_isTome = isTome;	_arraySelection = arraySelection;	_sizeArray = sizeArray;
	_tabReader = [(RakAppDelegate*) [NSApp delegate] reader];
	
	//We check if the user asked not to be annoyed again
	BOOL data = NO;
	
	_remind = [RakPrefsRemindPopover getValueReminded : PREFS_REMIND_AUTODL : &data];
	if(_remind && ![(RakAppDelegate*) [NSApp delegate] window].shiftPressed)
	{
		if(data)
			[self confirmed];
		
		return nil;
	}
	
	self = [super initWithFrame : NSMakeRect([((RakAppDelegate *) [NSApp delegate]) window].isFullscreen ? 85 : 0, 0, 170, 170)];
	
	if(self != nil)
	{
		NSRect frame = NSZeroRect;
		
		if(_tabReader != nil)
			frame.size.width = _tabReader.frame.origin.x;
		
		[self internalInit : _anchor : frame : YES];
		[(MDL*) _anchor registerPopoverExistance:self];
	}
	
	return self;
}

- (void) setupView
{
	NSRect frame = _frame;
	
	NSString * string = NSLocalizedString(_isTome ? (_sizeArray == 1 ? @"AUTO-DL-ONE-VOL" : @"AUTO-DL-SEVERAL-VOL") : (_sizeArray == 1 ? @"AUTO-DL-ONE-CHAP" : @"AUTO-DL-SEVERAL-CHAP"), nil);
	
	RakText * contentText = [[RakText alloc] initWithText:frame :string :[Prefs getSystemColor : COLOR_HIGHLIGHT:nil]];
	if(contentText != nil)
	{
		[contentText setFont:[NSFont fontWithName:[Prefs getFontName:GET_FONT_RD_BUTTONS] size:13]];
		[contentText setAlignment:NSCenterTextAlignment];
		[contentText sizeToFit];
		
		[contentText setFrameOrigin:NSMakePoint(frame.size.width / 2 - contentText.bounds.size.width / 2, frame.size.height - 10 - contentText.frame.size.height)];
		
		[self addSubview : contentText];
	}
	
	RakQuerySegmentedControl * button = [[RakQuerySegmentedControl alloc] initWithData : NSMakeRect(0, 0, self.frame.size.width, contentText.frame.origin.y - 10) : NSLocalizedString(@"YES", nil) : NSLocalizedString(@"NO", nil)];
	if(button != nil)
	{
		[button setTarget:self];
		[button setAction:@selector(buttonClicked:)];
		[self addSubview:button];
	}
	
	RakButton * buttonRemind = [RakButton allocWithText:NSLocalizedString(@"REMIND", nil) :NSMakeRect(button.frame.origin.x, button.frame.origin.y - 5 - button.frame.size.height, button.frame.size.width, button.frame.size.height)];
	if(buttonRemind != nil)
	{
		[buttonRemind setTarget:self];
		[buttonRemind setAction:@selector(remindSwitched:)];
		
		((RakButtonCell*)buttonRemind.cell).forceHighlight = _remind;
		[((RakButtonCell*)buttonRemind.cell) reloadFontColor];
		
		[self addSubview:buttonRemind];
	}
}

- (void) configurePopover:(INPopoverController *)internalPopover
{
	internalPopover.closeWhenAnchorMoveOffScreen = NO;
	internalPopover.overrideAnchorFrameUpdate = YES;
	[super configurePopover:internalPopover];
}

//Toolbox

- (void) locationUpdated : (NSRect) MDLFrame : (BOOL) animated
{
	NSPoint origin = [_anchor convertPoint:NSMakePoint([((RakAppDelegate *) [NSApp delegate]) window].isFullscreen ? 85 : 0, NSMaxY(MDLFrame)) toView:nil];
	origin = [_anchor.window convertRectToScreen: (NSRect) {origin, NSZeroSize}].origin;

	if(_tabReader != nil)
		origin.x += [_tabReader createFrame].origin.x / 2;
	else
		origin.x += MDLFrame.size.width / 2;
	
	dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.01 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
		[self updateOrigin : origin : animated];
	});
}

- (void) additionalUpdateOnThemeChange
{
	for(RakText * view in self.subviews)
	{
		if([view class] == [RakText class])
		{
			[view setTextColor:[Prefs getSystemColor : COLOR_HIGHLIGHT:nil]];
			break;
		}
	}
}

- (void)popoverDidClose:(INPopoverController *)discarded;
{
	[(MDL*) _anchor registerPopoverExistance:nil];
	[super popoverDidClose:discarded];
	[popover setDelegate:nil];
}

#pragma mark - Payload

- (void) buttonClicked : (RakQuerySegmentedControl*) sender
{
	BOOL output = NO;
	
	if([sender selectedSegment] == 0)
	{
		output = YES;
		[self confirmed];
	}
	
	if(_remind)
		[RakPrefsRemindPopover setValueReminded : PREFS_REMIND_AUTODL : output];
	else
		[RakPrefsRemindPopover removeValueReminded : PREFS_REMIND_AUTODL];
	
	
	[popover closePopover];
}

- (void) remindSwitched : (RakButton*) sender
{
	_remind = !_remind;
	
	if(sender != nil && [sender class] == [RakButton class] && sender.cell != nil && [sender.cell class] == [RakButtonCell class])
		((RakButtonCell*)sender.cell).forceHighlight = _remind;
}

- (void) confirmed
{
	for (uint pos = 0, handledArray = _sizeArray - 1; pos < handledArray; pos++)
	{
		[(MDL*) _anchor proxyAddElement:_project isTome:_isTome element:_arraySelection[pos] partOfBatch:YES];
	}
	
	[(MDL*) _anchor proxyAddElement:_project isTome:_isTome element:_arraySelection[_sizeArray - 1] partOfBatch:NO];
}

@end

@implementation RakQuerySegmentedControl

- (instancetype) initWithData : (NSRect) frame : (NSString *) agree : (NSString *) disagree
{
	self = [super initWithFrame:[self getButtonFrame:frame]];
	
	if(self != nil)
	{
		uint widthButton1 = self.frame.size.width / 2, widthButton2 = self.frame.size.width - widthButton1 - 1;
		
		[self setSegmentCount:2];
		
		NSDictionary * attribute = @{NSFontAttributeName:[NSFont fontWithName:[Prefs getFontName:GET_FONT_RD_BUTTONS] size:13]};
		NSSize sizeFirst = [agree sizeWithAttributes:attribute], sizeSecond = [disagree sizeWithAttributes:attribute];
		
		if(sizeFirst.width + 1 > widthButton1 || sizeSecond.width + 1 > widthButton2)	//Fuck, not enough space
		{
			if(sizeFirst.width + sizeSecond.width + 2 < self.frame.size.width)	//We can recover
			{
				CGFloat delta = self.frame.size.width - (sizeFirst.width + sizeSecond.width);
				widthButton1 = sizeFirst.width + delta / 2;
				widthButton2 = self.frame.size.width - widthButton1;
			}
		}
		
		[self setLabel:agree forSegment:0];
		[self setWidth:widthButton1 forSegment:0];
		
		[self setLabel:disagree forSegment:1];
		[self setWidth:widthButton2 forSegment:1];
		
		[self setFrameOrigin:[self getButtonFrame:frame].origin];
	}
	
	return self;
}

- (NSRect) getButtonFrame : (NSRect) superviewAvailableSpace
{
	NSRect frame = self.frame;
	
	frame.size.height = CT_READERMODE_HEIGHT_CT_BUTTON;
	if(frame.size.width > superviewAvailableSpace.size.width || !frame.size.width)
		frame.size.width = superviewAvailableSpace.size.width * 3 / 4;
	
	frame.origin.y = superviewAvailableSpace.size.height - frame.size.height;
	frame.origin.x = superviewAvailableSpace.size.width / 2 - frame.size.width / 2;
	
	return frame;
}

- (void) setFrame:(NSRect)frameRect
{
	NSRect newFrame = [self getButtonFrame:frameRect];
	[super setFrame: newFrame];
	
	if(newFrame.size.width == frameRect.size.width)
	{
		[self sizeToFit];
		newFrame = [self getButtonFrame:frameRect];
		[super setFrame: newFrame];
	}
}

- (void) resizeAnimation : (NSRect) frameRect
{
	[self.animator setFrame : [self getButtonFrame:frameRect]];
}

+ (Class)cellClass
{
	return [RakSegmentedButtonCell class];
}

@end

