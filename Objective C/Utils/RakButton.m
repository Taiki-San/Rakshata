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
		output.layer.backgroundColor = [Prefs getSystemColor:GET_COLOR_BACKGROUD_BACK_BUTTONS].CGColor;
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
	}
	
	return self;
}

- (id) copyWithZone:(NSZone *)zone
{
	return textCell ? nil : [[RakButtonCell allocWithZone:zone] initWithRawData:clicked :nonClicked :unAvailable];
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

//Image only code

- (id) initWithPage : (NSString*) imageName : (short) state
{
	self = [self init];
	
	if(self != nil)
	{
		clicked		= [[RakResPath craftResNameFromContext:imageName : YES : YES : 1] retain];
		nonClicked	= [[RakResPath craftResNameFromContext:imageName : NO : YES : 1] retain];
		unAvailable = [[RakResPath craftResNameFromContext:imageName : NO : NO : 1] retain];
		notAvailable = false;
		canHighlight = true;
		
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
		{
			NSLog(@"Failed at create button for icon: %@", imageName);
			[self release];
			return nil;
		}
	}
	
	return self;
}

- (id) initWithRawData : (NSImage*) _clicked : (NSImage*) _nonClicked : (NSImage*) _unAvailable
{
	self = [self init];
	
	if(self != nil)
	{
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
		[self.image release];
		[self setImage:clicked];
	}
	else if(value == RB_STATE_STANDARD)
	{
		[self.image release];
		[self setImage:nonClicked];
	}
	else if(value == RB_STATE_UNAVAILABLE)
	{
		[self.image release];
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
		}
	}
	
	return self;
}

- (NSColor*) getBorderColor
{
	return [NSColor colorWithDeviceWhite:32.0f/255.0f alpha:1.0f];
}

- (NSColor*) getBackgroundColor
{
	return [NSColor colorWithDeviceWhite:39.0f/255.0f alpha:1.0];
}

- (NSColor *) getFontColor
{
	if([self isHighlighted] || self.forceHighlight)
		return [Prefs getSystemColor:GET_COLOR_FONT_BUTTON_CLICKED];
	else
		return [Prefs getSystemColor:GET_COLOR_FONT_BUTTON_NONCLICKED];
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