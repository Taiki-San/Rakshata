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

- (id) initWithData : (MDL*) tabMDL : (PROJECT_DATA) project : (BOOL) isTome : (int*) arraySelection : (uint) sizeArray
{
	if(tabMDL == nil || arraySelection == NULL || sizeArray == 0)
		return nil;
	
	self = [super initWithFrame : NSMakeRect(0, 0, 170, 170)];
	
	if(self != nil)
	{
		NSRect frame = NSZeroRect;
		_anchor = tabMDL;	_project = project;		_isTome = isTome;	_arraySelection = arraySelection;	_sizeArray = sizeArray;
		_remind = false;	_tabReader = nil;
		
		NSArray *subviews;
		if(_anchor.superview != nil && (subviews = _anchor.superview.subviews) != nil)
		{
			for(NSView * view in subviews)
			{
				if([view class] == [Reader class])
				{
					_tabReader = (Reader*) view;
					frame.size.width = view.frame.origin.x;
					break;
				}
			}
		}
	
		[self internalInit : _anchor : frame : YES];
		[(MDL*) _anchor registerPopoverExistance:self];
	}
	
	return self;
}

- (void) setupView
{
	NSString * string = nil, *complement = _isTome ? @"tome" : @"chapitre";
	
	if (_sizeArray == 1)
		string = [NSString stringWithFormat:@" J'ai remarqué %s y a un %@\nnon-téléchargé après\ncelui-là. Voulez vous\nle télécharger?", _isTome ? "\nqu'il" : "qu'il\n", complement];
	else
		string = [NSString stringWithFormat:@" J'ai remarqué %s y a des %@s\nnon-téléchargés après\ncelui-là. Voulez vous\nles télécharger?", _isTome ? "\nqu'il" : "qu'il\n", complement];
	
	RakText * contentText = [[[RakText alloc] initWithText:self.frame :string :[Prefs getSystemColor : GET_COLOR_ACTIVE:nil]] autorelease];
	[contentText setFont:[NSFont fontWithName:[Prefs getFontName:GET_FONT_RD_BUTTONS] size:13]];
	[contentText sizeToFit];
	[self addSubview : contentText];
	[contentText setFrameOrigin:NSMakePoint(10 , self.frame.size.height - 10 - contentText.frame.size.height)];
	
	RakQuerySegmentedControl * button = [[[RakQuerySegmentedControl alloc] initWithData : NSMakeRect(0, 0, self.frame.size.width, contentText.frame.origin.y - 15) : @"Oui" : @"Non"] autorelease];
	[button setTarget:self];
	[button setAction:@selector(buttonClicked:)];
	[self addSubview:button];
	
	RakButton * buttonRemind = [[RakButton allocWithText:@"Se souvenir" :NSMakeRect(button.frame.origin.x, button.frame.origin.y - 5 - button.frame.size.height, button.frame.size.width, button.frame.size.height)] autorelease];
	[buttonRemind setTarget:self];
	[buttonRemind setAction:@selector(remindSwitched:)];
	[self addSubview:buttonRemind];
}

//Colors

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

//Toolbox

- (void) locationUpdated : (NSRect) MDLFrame : (BOOL) animated
{
	NSPoint origin = NSMakePoint(0, MDLFrame.origin.y + MDLFrame.size.height);
	origin = [_anchor.window convertBaseToScreen:[_anchor convertPoint:origin toView:nil]];
	
	if(_tabReader != nil)
		origin.x += [_tabReader createFrame].origin.x / 2;
	else
		origin.x += MDLFrame.size.width / 2;
	
	[self updateOrigin : origin : animated];
}

- (void) additionalUpdateOnThemeChange
{
	for(RakText * view in self.subviews)
	{
		if([view class] == [RakText class])
		{
			[view setTextColor:[Prefs getSystemColor : GET_COLOR_ACTIVE:nil]];
			break;
		}
	}
}

- (void)popoverDidClose:(INPopoverController *)discarded;
{
	[(MDL*) _anchor registerPopoverExistance:nil];
	[super popoverDidClose:discarded];
}

#pragma mark - Payload

- (void) buttonClicked : (RakQuerySegmentedControl*) sender
{
	if([sender selectedSegment] == 0)
		[self confirmed];
	[popover closePopover];
}

- (void) remindSwitched : (RakButton*) sender
{
	_remind = !_remind;
	
	if (sender != nil && [sender class] == [RakButton class] && sender.cell != nil && [sender.cell class] == [RakButtonCell class])
		((RakButtonCell*)sender.cell).forceHighlight = _remind;
}

- (void) confirmed
{
	for (uint pos = 0, handledArray = _sizeArray - 1; pos < handledArray; pos++)
	{
		[(MDL*) _anchor proxyAddElement: _project: _isTome : _arraySelection[pos] : YES];
	}

	[(MDL*) _anchor proxyAddElement: _project: _isTome: _arraySelection[_sizeArray - 1] : NO];
}

@end

@implementation RakQuerySegmentedControl

- (id) initWithData : (NSRect) frame : (NSString *) agree : (NSString *) disagree
{
	self = [super initWithFrame:[self getButtonFrame:frame]];
	
	if (self != nil)
	{
		uint widthButton1 = self.frame.size.width / 2, widthButton2 = self.frame.size.width - widthButton1 - 1;
		
		[self setSegmentCount:2];
		
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

- (void)resizeSubviewsWithOldSize:(NSSize)oldBoundsSize
{
	NSLog(@"[%@] - Weird constraints detected!", self);
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

