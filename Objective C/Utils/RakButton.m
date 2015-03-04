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

@implementation RakButton

+ (instancetype) allocImageWithBackground : (NSString*) imageName : (short) stateAtStartup : (id) target : (SEL) selectorToCall
{
	RakButton *output = [self allocImageWithoutBackground:imageName :stateAtStartup: target :selectorToCall];
	
	if(output != nil)
	{
		[output triggerBackground];
	}
	
	return output;
}

+ (instancetype) allocImageWithoutBackground : (NSString*) imageName : (short) stateAtStartup : (id) target : (SEL) selectorToCall
{
	RakButton* output = [self new];
	
	if(output != nil)
	{
		output.textButton = NO;
		
		output.cell = [output.cell initWithPage: imageName : stateAtStartup];
		
		//Update a couple of prefs
		[output sizeToFit];
		[output setBordered:NO];
		
		//Set action
		if(target != nil)
		{
			[output setTarget:target];
			[output setAction:selectorToCall];
		}
	}
	
	return output;
}

+ (instancetype) allocWithText : (NSString*) string : (NSRect) frame
{
	RakButton* output = [self new];
	
	if(output != nil)
	{
		output.textButton = YES;
		output.wantsLayer = YES;

		output.cell = [output.cell initWithText:string];
		
		if(!NSEqualRects(frame, NSZeroRect))
			[output setFrame:frame];
	}
	
	return output;
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return;
	
	self.layer.backgroundColor = [Prefs getSystemColor:GET_COLOR_BACKGROUD_BACK_BUTTONS:self].CGColor;
	[self setNeedsDisplay];
}

- (void) sizeToFit
{
	if(self.textButton)
	{
		NSRect frame = self.frame;
		
		frame.size = [self.cell sizeOfTextCell];
		frame.size.height += 4;
		frame.size.width += 10;
		
		self.frame = frame;
	}
	else
		[super sizeToFit];
}

+ (Class) cellClass
{
	return [RakButtonCell class];
}

#pragma mark - Helper

+ (instancetype) allocForReader : (NSView*) superview : (NSString*) imageName : (short) stateAtStartup : (CGFloat) posX : (BOOL) posXFromLeftSide : (id) target : (SEL) selectorToCall
{
	RakButton * button = [self allocImageWithoutBackground:imageName :stateAtStartup :target :selectorToCall];
	
	if(button != nil && superview != nil)
	{
		//Set origin
		
		NSPoint point;
		
		if(posXFromLeftSide)
			point = NSMakePoint(posX, superview.frame.size.height / 2 - button.frame.size.height / 2);
		else
			point = NSMakePoint(posX - button.frame.size.width, superview.frame.size.height / 2 - button.frame.size.height / 2);
		
		[button setFrameOrigin: point];
		
		//Add to the superview
		[superview addSubview:button];
	}
	
	return button;
}

#pragma mark - Interface

- (BOOL) hasBorder
{
	return ((RakButtonCell *) self.cell).hasBorder;
}

- (void) setHasBorder : (BOOL) hasBorder
{
	((RakButtonCell *) self.cell).hasBorder = hasBorder;
}

- (void) triggerBackground
{
	if(haveBackground)
	{
		self.wantsLayer = NO;
	}
	else
	{
		self.wantsLayer = YES;
		self.layer.backgroundColor = [Prefs getSystemColor:GET_COLOR_BACKGROUD_BACK_BUTTONS : self].CGColor;
		self.layer.cornerRadius = 4;
	}
	haveBackground = !haveBackground;
}

@end

@implementation RakButtonCell

- (id) init
{
	self = [super init];
	
	if(self != nil)
	{
		self.focusRingType = NSFocusRingTypeNone;
		textCell = nil;
		_imageName = nil;
	}
	
	return self;
}

- (void) dealloc
{
	if(!textCell)
		[self setImage:nil];
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return;
	
	if(_imageName == nil)	//text cell
	{
		[textCell setTextColor:[self getFontColor]];
	}
	else					//img cell
	{
		uint state = 0;
		
		//We get the previous state to restore it
		if(self.image == clicked)
			state = RB_STATE_HIGHLIGHTED;
		else if(self.image == nonClicked)
			state = RB_STATE_STANDARD;
		
		//Free the previous images
		[self loadIcon:state :[Prefs getCurrentTheme:nil]];
	}
}

//Image only code

- (id) initWithPage : (NSString*) imageName : (short) state
{
	self = [self init];
	
	if(self != nil)
	{
		_imageName = [NSString stringWithString:imageName];
		
		notAvailable = false;
		canHighlight = true;
		
		if(![self loadIcon:state :[Prefs getCurrentTheme:self]])
		{
			NSLog(@"Failed at create button for icon: %@", imageName);
			return nil;
		}
	}
	
	return self;
}

- (BOOL) loadIcon : (short) state : (uint) currentTheme
{
	clicked		= [RakResPath craftResNameFromContext:_imageName : YES : YES : currentTheme];
	nonClicked	= [RakResPath craftResNameFromContext:_imageName : NO : YES : currentTheme];
	unAvailable = [RakResPath craftResNameFromContext:_imageName : NO : NO : currentTheme];
	
	if(state == RB_STATE_STANDARD && nonClicked != nil)
		[self setImage:nonClicked];
	else if(state == RB_STATE_HIGHLIGHTED && clicked != nil)
		[self setImage:clicked];
	else if(unAvailable != nil)
	{
		[self setImage:unAvailable];
		notAvailable = true;
	}
	else
		return NO;
	return YES;
}

- (id) initWithRawData : (NSString *) imageName : (NSImage*) _clicked : (NSImage*) _nonClicked : (NSImage*) _unAvailable
{
	self = [self init];
	
	if(self != nil)
	{
		_imageName = [imageName copy];
		clicked = _clicked;
		nonClicked = _nonClicked;
		unAvailable = _unAvailable;
		
		[self setImage:nonClicked];
	}
	
	return self;
}

- (void) setState : (NSInteger)value
{
	notAvailable = false;
	
	if(value == RB_STATE_HIGHLIGHTED && canHighlight)
	{
		[self setImage:clicked];
	}
	else if(value == RB_STATE_STANDARD)
	{
		[self setImage:nonClicked];
	}
	else if(value == RB_STATE_UNAVAILABLE)
	{
		[self setImage:unAvailable];
		notAvailable = true;
	}
}

- (void) setHighlightAllowed : (BOOL) allowed
{
	canHighlight = allowed;
}

- (bool) isHighlightAllowed
{
	return canHighlight;
}

- (void) highlight:(BOOL)flag withFrame:(NSRect)cellFrame inView:(NSView*)controlView
{
	if(textCell)
	{
		[super highlight:flag withFrame:cellFrame inView:controlView];
		[textCell setTextColor:[self getFontColor]];
	}
	else if(!notAvailable)
	{
		if (flag)
			self.backgroundColor = [NSColor clearColor];
		[super highlight:flag withFrame:cellFrame inView:controlView];
	}
}

//Text only code

- (id) initWithText : (NSString *) text
{
	self = [self init];
	
	if(self != nil)
	{
		_hasBorder = YES;
		textCell = [[RakCenteredTextFieldCell alloc] initTextCell:text];
		if(textCell != nil)
		{
			textCell.centered = YES;
			[textCell setFont:[NSFont fontWithName:[Prefs getFontName:GET_FONT_RD_BUTTONS] size:13]];
			[textCell setAlignment:NSCenterTextAlignment];
			[textCell setTextColor:[self getFontColor]];
			[Prefs getCurrentTheme:self];	//Register to changes
		}
	}
	return self;
}

- (void) reloadFontColor
{
	if(textCell != nil)
		[textCell setTextColor:[self getFontColor]];
}

- (NSSize) sizeOfTextCell
{
	if(textCell != nil)
		return textCell.attributedStringValue.size;
	
	return NSZeroSize;
}

- (NSColor*) getBorderColor
{
	return [Prefs getSystemColor:GET_COLOR_BORDER_BUTTONS :nil];
}

- (NSColor*) getBackgroundColor
{
	return [Prefs getSystemColor:GET_COLOR_BACKGROUND_BUTTON_UNSELECTED :nil];
}

- (NSColor *) getFontColor
{
	if([self isHighlighted] || self.forceHighlight)
		return [Prefs getSystemColor:GET_COLOR_FONT_BUTTON_CLICKED : nil];
	else if([self isEnabled])
		return [Prefs getSystemColor:GET_COLOR_FONT_BUTTON_NONCLICKED : nil];
	else
		return [Prefs getSystemColor:GET_COLOR_FONT_BUTTON_UNAVAILABLE :nil];
}

- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
	if(textCell != nil)
	{
		if(_hasBorder)
		{
			[[self getBorderColor] setFill];
			NSRectFill(cellFrame);
			
			cellFrame.size.width -= 2;	//border
			cellFrame.size.height -= 2;
			cellFrame.origin.x++;
			cellFrame.origin.y++;
		}
		
		[[self getBackgroundColor] setFill];
		NSRectFill(cellFrame);
		
		[textCell drawInteriorWithFrame:cellFrame inView:controlView];
	}
	else
		[super drawWithFrame:cellFrame inView:controlView];
}

@end