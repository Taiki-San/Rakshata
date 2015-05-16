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

+ (instancetype) allocWithText : (NSString*) string
{
	return [self allocWithText:string :NSZeroRect];
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
		else
			[output sizeToFit];
	}
	
	return output;
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return;
	
	self.layer.backgroundColor = [Prefs getSystemColor:GET_COLOR_BACKGROUND_BACK_BUTTONS:self].CGColor;
	[self setNeedsDisplay];
}

- (void) sizeToFit
{
	if(self.textButton)
	{
		NSRect frame = _frame;
		
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
		self.layer.backgroundColor = [Prefs getSystemColor:GET_COLOR_BACKGROUND_BACK_BUTTONS : self].CGColor;
		self.layer.cornerRadius = 4;
	}
	haveBackground = !haveBackground;
}

@end

@implementation RakButtonCell

#pragma mark - Initialization

- (instancetype) init
{
	self = [super init];
	
	if(self != nil)
	{
		self.focusRingType = NSFocusRingTypeNone;
		textCell = nil;
		_borderWidth = 1;
		_imageName = nil;
	}
	
	return self;
}

- (instancetype) initWithPage : (NSString*) imageName : (short) state
{
	self = [self init];
	
	if(self != nil)
	{
		_imageName = [imageName copy];
		
		notAvailable = NO;
		_highlightAllowed = YES;
		
		if(![self loadIcon:state :[Prefs getCurrentTheme:self]])
		{
			NSLog(@"Failed at create button for icon: %@", imageName);
			return nil;
		}
	}
	
	return self;
}

- (instancetype) initWithText : (NSString *) text
{
	self = [self init];
	if(self != nil)
	{
		_hasBorder = YES;

		textCell = [[RakCenteredTextFieldCell alloc] initTextCell:text];
		if(textCell != nil)
		{
			textCell.centered = YES;
			textCell.alignment = NSCenterTextAlignment;
			textCell.textColor = [self getFontColor];
			textCell.font = [NSFont fontWithName:[Prefs getFontName:GET_FONT_RD_BUTTONS] size:13];

			[Prefs getCurrentTheme:self];	//Register to changes
		}
	}
	return self;
}

- (void) dealloc
{
	if(!textCell)
		self.image = nil;
	
	[Prefs deRegisterForChanges:self];
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return;
	
	if(_imageName == nil)	//text cell
	{
		[self reloadFontColor];
		[self.controlView setNeedsDisplay:YES];
	}
	else					//img cell
	{
		uint state = RB_STATE_UNAVAILABLE;
		
		//We get the previous state to restore it
		if(self.image == clicked)
			state = RB_STATE_HIGHLIGHTED;
		else if(self.image == nonClicked)
			state = RB_STATE_STANDARD;
		
		[self loadIcon:state :[Prefs getCurrentTheme:nil]];
	}
}

#pragma mark - Utils

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
		notAvailable = YES;
	}
	else
		return NO;
	return YES;
}

- (void) setState : (NSInteger)value
{
	notAvailable = NO;
	
	if(value == RB_STATE_HIGHLIGHTED && _highlightAllowed)
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
		notAvailable = YES;
	}
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

#pragma mark - Sizing

- (void) setBorderWidth:(CGFloat)borderWidth
{
	_borderWidth = borderWidth;
	[(RakButton *) self.controlView sizeToFit];
}

- (NSSize) sizeOfTextCell
{
	if(textCell != nil)
	{
		NSSize size = textCell.attributedStringValue.size;
		
		size.width += 2 * _borderWidth;
		size.height += 2 * _borderWidth;
		
		return size;
	}
	
	return NSZeroSize;
}

#pragma mark - Drawing

- (NSColor*) getBorderColor
{
	return [Prefs getSystemColor:GET_COLOR_BORDER_BUTTONS :nil];
}

- (NSColor*) getBackgroundColor
{
	return _customBackgroundColor == nil ? [Prefs getSystemColor:GET_COLOR_BACKGROUND_BUTTON_UNSELECTED :nil] : _customBackgroundColor;
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

- (void) reloadFontColor
{
	if(textCell != nil)
		[textCell setTextColor:[self getFontColor]];
}

- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
	if(textCell != nil)
	{
		if(_hasBorder)
		{
			[[self getBorderColor] setFill];
			NSRectFill(cellFrame);
			
			cellFrame.size.width -= 2 * _borderWidth;	//border
			cellFrame.size.height -= 2 * _borderWidth;
			cellFrame.origin.x += _borderWidth;
			cellFrame.origin.y += _borderWidth;
		}
		
		[[self getBackgroundColor] setFill];
		NSRectFill(cellFrame);
		
		[textCell drawInteriorWithFrame:cellFrame inView:controlView];
	}
	else
		[super drawWithFrame:cellFrame inView:controlView];
}

@end