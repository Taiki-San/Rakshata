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

+ (id) allocForSeries : (NSView*) superView : (NSString*) imageName : (NSPoint) origin : (id) target : (SEL) selectorToCall
{
	RakButton *output = [RakButton new];
	
	if(output != nil)
	{
		[output.cell initWithPage: imageName : RB_STATE_STANDARD];
		
		//Update a couple of prefs
		[output sizeToFit];
		output.wantsLayer = YES;
		output.layer.backgroundColor = [Prefs getSystemColor:GET_COLOR_BACKGROUD_BACK_BUTTONS:self].CGColor;
		output.layer.cornerRadius = 4;
		[output setBordered:NO];
		
		//Set action
		if(target != nil)
		{
			[output setTarget:target];
			[output setAction:selectorToCall];
		}
		
		if(superView != nil)
		{
			//Set origin
			origin.x -= ((RakButtonCell*)output.cell).cellSize.width / 2;
			[output setFrameOrigin: origin];
			
			//Add to the superview
			[superView addSubview:output];
		}
	}
	
	return output;
}

+ (id) allocForReader : (NSView*) superView : (NSString*) imageName : (short) stateAtStartup : (CGFloat) posX : (BOOL) posXFromLeftSide : (id) target : (SEL) selectorToCall
{
	RakButton *output = [RakButton new];
	
	if(output != nil)
	{
		[output.cell initWithPage: imageName : stateAtStartup];
		
		//Update a couple of prefs
		[output sizeToFit];
		[output setBordered:NO];
		
		//Set action
		[output setTarget:target];
		[output setAction:selectorToCall];
		
		//Set origin
		NSPoint point;
		
		if(posXFromLeftSide)
			point = NSMakePoint(posX, superView.frame.size.height / 2 - output.frame.size.height / 2);
		else
			point = NSMakePoint(posX - output.frame.size.width, superView.frame.size.height / 2 - output.frame.size.height / 2);
		
		[output setFrameOrigin: point];
		
		//Add to the superview
		[superView addSubview:output];
	}
	
	return output;
}

+ (id) allocWithText : (NSString*) string : (NSRect) frame
{
	RakButton * output = [RakButton new];
	
	if(output != nil)
	{
		[output.cell initWithText:string];
		[output setFrame:frame];
	}
	
	return output;
}

- (id) copyWithZone:(NSZone *)zone
{
	RakButton * output = [[RakButton allocWithZone:zone] init];
	
	if(output != nil)
	{
		NSCell * cell = [self.cell copy];
		[output setCell:cell];
		[cell release];
		
		[output sizeToFit];
		[output setBordered:NO];
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

+ (Class) cellClass
{
	return [RakButtonCell class];
}

@end

@implementation RakButtonCell

- (id) init
{
	self = [super init];
	
	if(self != nil)
	{
		textCell = nil;
		_imageName = nil;
	}
	
	return self;
}

- (id) copyWithZone:(NSZone *)zone
{
	return textCell ? nil : [[RakButtonCell allocWithZone:zone] initWithRawData: _imageName  :clicked :nonClicked :unAvailable];
}

- (void) dealloc
{
	if(!textCell)
	{
		[clicked release];
		[nonClicked release];
		[unAvailable release];
		NSImage * bak = self.image;
		[self setImage:nil];
		[bak release];
	}
	[super dealloc];
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
		[clicked release];		[nonClicked release];	[unAvailable release];
		clicked = self.image;	[self setImage:nil];	[clicked release];
		
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
			[self release];
			return nil;
		}
	}
	
	return self;
}

- (BOOL) loadIcon : (short) state : (uint) currentTheme
{
	clicked		= [[RakResPath craftResNameFromContext:_imageName : YES : YES : currentTheme] retain];
	nonClicked	= [[RakResPath craftResNameFromContext:_imageName : NO : YES : currentTheme] retain];
	unAvailable = [[RakResPath craftResNameFromContext:_imageName : NO : NO : currentTheme] retain];
	
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
		clicked = [_clicked retain];
		nonClicked = [_nonClicked retain];
		unAvailable = [_unAvailable retain];
		
		[self setImage:nonClicked];
	}
	
	return self;
}

- (void) setState:(NSInteger)value
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
		textCell = [[RakCenteredTextFieldCell alloc] initTextCell:text];
		if(textCell != nil)
		{
			[textCell setFont:[NSFont fontWithName:[Prefs getFontName:GET_FONT_RD_BUTTONS] size:13]];
			[textCell setAlignment:NSCenterTextAlignment];
			[textCell setTextColor:[self getFontColor]];
			[Prefs getCurrentTheme:self];	//Register to changes
		}
	}
	
	return self;
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
	else
		return [Prefs getSystemColor:GET_COLOR_FONT_BUTTON_NONCLICKED : nil];
}

- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
	if(textCell != nil)
	{
		[[self getBorderColor] setFill];
		NSRectFill(cellFrame);
		
		NSRect frame = cellFrame;
		
		frame.size.width -= 2;	//border
		frame.size.height -= 2;
		frame.origin.x++;
		frame.origin.y++;
		
		[[self getBackgroundColor] setFill];
		NSRectFill(frame);
		
		[textCell drawInteriorWithFrame:frame inView:controlView];
	}
	else
		[super drawWithFrame:cellFrame inView:controlView];
}

@end