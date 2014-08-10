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

@implementation RakPageCounter

- (id)init: (NSView*) superView : (CGFloat) posX : (uint) currentPageArg : (uint) pageMaxArg : (Reader *) target
{
    self = [super initWithText:[superView bounds] :[NSString stringWithFormat:@"%d/%d", currentPageArg+1, pageMaxArg+1] :[self getFontColor]];
    if (self)
	{
		[self setFont:[NSFont boldSystemFontOfSize:13]];
		[self updateSize:[superView bounds].size.height : posX];
		[Prefs getCurrentTheme:self];	//register
		
		currentPage = currentPageArg+1;
		pageMax = pageMaxArg+1;
		_target = target;
		
		[self setBackgroundColor:[self getColorBackground]];
    }
    return self;
}

- (void) updateContext
{
	[self setStringValue:[NSString stringWithFormat:@"%d/%d", currentPage, pageMax]];
	
	[self updateSize: self.superview.bounds.size.height : -1];

	[self setNeedsDisplay:YES];
}

- (void) updatePopoverFrame : (NSRect) newFrame : (BOOL) animated
{
	if(popover != nil)
		[popover locationUpdated: newFrame : animated];
}

- (void) stopUsePopover
{
	popover = nil;
	popoverStillAround = YES;
}

- (void) removePopover
{
	popoverStillAround = NO;
}

#pragma mark - Size related

- (void) updateSize : (CGFloat) heightSuperView : (CGFloat) posX
{
	NSPoint origin;
	
	if(posX == -1)
	{
		CGFloat previousWidth = self.bounds.size.width, newWidth;
		
		[self sizeToFit];
		
		newWidth = self.bounds.size.width;
		origin.x = self.frame.origin.x - (newWidth - previousWidth) / 2;
	}
	else
	{
		[self sizeToFit];
		origin.x = posX - self.bounds.size.width / 2;
	}
	
	origin.y = heightSuperView / 2 - self.bounds.size.height / 2;
	
	[self setFrameOrigin:origin];
}

+ (Class)cellClass
{
	return [RakCenteredTextFieldCell class];
}

#pragma mark - Color stuffs

- (NSColor *) getColorBackground
{
	return [Prefs getSystemColor:GET_COLOR_READER_BAR_PAGE_COUNTER:nil];
}

- (NSColor *) getBorderColor
{
	return [NSColor blackColor];
}

- (NSColor *) getFontColor
{
	return [Prefs getSystemColor:GET_COLOR_INACTIVE:nil];
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return;
	
	[self setTextColor:[self getFontColor]];
	[self setBackgroundColor:[self getColorBackground]];
}

- (void) drawBorder
{
	CGContextRef contextBorder = [[NSGraphicsContext currentContext] graphicsPort];
	CGContextBeginPath(contextBorder);

	CGContextSetLineWidth(contextBorder, 2);
	[[self getBorderColor] setStroke];

	CGContextMoveToPoint(contextBorder, 0, self.bounds.size.height);
	CGContextAddLineToPoint(contextBorder, 0, 0);
	CGContextAddLineToPoint(contextBorder, self.bounds.size.width, 0);
	
	CGContextStrokePath(contextBorder);
}

- (void)drawRect:(NSRect)dirtyRect
{
    [super drawRect:dirtyRect];

	[self drawBorder];
}

#pragma mark - Setters/Getters

- (void) updatePage : (uint) newCurrentPage : (uint) newPageMax
{
	currentPage = newCurrentPage + 1;
	pageMax = newPageMax + 1;
	
	[self updateContext];
}

#pragma mark - Events

- (void) mouseDown:(NSEvent *)theEvent
{
	if(popover == nil && !popoverStillAround)
	{
		[[NSBundle mainBundle] loadNibNamed:@"jumpPage" owner:self topLevelObjects:nil];
		[popover launchPopover : self : currentPage : pageMax];
	}
	else
		[super mouseDown:theEvent];
}

- (void) transmitPageJump : (uint) newPage
{
	[_target jumpToPage:newPage];
}

@end

@interface RakFormatterNumbersOnly : NSNumberFormatter

@end

@implementation RakFormatterNumbersOnly

- (BOOL)isPartialStringValid:(NSString*)partialString newEditingString:(NSString**)newString errorDescription:(NSString**)error
{
	if ([partialString length] == 0)
		return YES;
	
	if([partialString rangeOfCharacterFromSet:[[NSCharacterSet decimalDigitCharacterSet] invertedSet]].location != NSNotFound)
		return NO;
	
	NSNumberFormatter * f = [[[NSNumberFormatter alloc] init] autorelease];
	[f setNumberStyle:NSNumberFormatterDecimalStyle];
	NSNumber * content = [f numberFromString:partialString];
	
	if(content == nil)
		return NO;

	if(content > [self maximum])
	{
		*newString = [NSString stringWithFormat:@"%d", [[self maximum] unsignedIntValue]];
		return NO;
	}
	
	return YES;
}

@end

@implementation RakPageCounterPopover

- (void) launchPopover : (NSView *) anchor : (uint) curPage : (uint) maxPage
{
	_anchor = anchor;
	_maxPage = maxPage;
	[Prefs getCurrentTheme:self];
	
	[self internalInit: anchor : NSMakeRect(0, 0, _anchor.frame.size.width, _anchor.frame.size.height) : YES];
	
	[self.window makeFirstResponder : textField];
}

- (void) setupView
{
	if(mainLabel != nil)
	{
		[mainLabel setStringValue:@"Aller à:"];
		[mainLabel sizeToFit];
		[mainLabel setTextColor:[Prefs getSystemColor:GET_COLOR_ACTIVE :nil]];
	}
	
	if(gotoButtonContainer != nil)
	{
		RakButton * gotoButton = [[RakButton allocWithText:@"Go":gotoButtonContainer.bounds] autorelease];
		[gotoButton setTarget:self];
		[gotoButton setAction:@selector(jumpTrigered)];
		[gotoButtonContainer addSubview:gotoButton];
	}
	
	if(textField != nil)
	{
		((RakTextCell*)textField.cell).customizedInjectionPoint = YES;
		[textField setBackgroundColor:[Prefs getSystemColor:GET_COLOR_BACKGROUND_TEXTFIELD :nil]];
		[textField setTextColor:[Prefs getSystemColor:GET_COLOR_CLICKABLE_TEXT :nil]];
		[textField setBezeled:NO];
		
		RakFormatterNumbersOnly * formater = [[[RakFormatterNumbersOnly alloc] init] autorelease];
		[formater setMinimum:@(1)];
		[formater setMaximum:@(_maxPage)];
		
		[textField setFormatter:formater];
		
		//Enter key is pressed
		[textField setTarget:self];
		[textField setAction:@selector(jumpTrigered)];
		
	}
}

- (void) configurePopover:(INPopoverController *)internalPopover
{
	[super configurePopover:internalPopover];
	
	internalPopover.closesWhenApplicationBecomesInactive = YES;
	internalPopover.closesWhenPopoverResignsKey = YES;
}

//Toolbox

- (void) locationUpdated : (NSRect) frame : (BOOL) animated
{
	[self updateOrigin : frame.origin : animated];
}

- (void) setFrameSize:(NSSize)newSize
{
	//TODO: investigate this bug...
	
	if (newSize.width < self.frame.size.height)
		[super setFrameSize:newSize];
}

- (void) additionalUpdateOnThemeChange
{
	if(mainLabel != nil)
	{
		[mainLabel setTextColor:[Prefs getSystemColor:GET_COLOR_ACTIVE :nil]];
		[mainLabel setNeedsDisplay:YES];
	}
	
	if(textField != nil)
	{
		[textField setBackgroundColor:[Prefs getSystemColor:GET_COLOR_BACKGROUND_TEXTFIELD :nil]];
		[textField setTextColor:[Prefs getSystemColor:GET_COLOR_CLICKABLE_TEXT :nil]];
		[textField setNeedsDisplay:YES];
	}
}

#pragma mark - Payload

- (void) jumpTrigered
{
	NSInteger value = [[textField stringValue] integerValue];
	
	if(value > 0)
	{
		if (value > _maxPage)
		{
			[textField setStringValue:[NSString stringWithFormat:@"%d", _maxPage]];
		}
		else
		{
			if([_anchor class] == [RakPageCounter class])
				[(RakPageCounter*) _anchor transmitPageJump : value - 1];
			
			[popover closePopover];
		}
	}
}

@end
