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
	return [Prefs getSystemColor:GET_COLOR_READER_BAR_PAGE_COUNTER];
}

- (NSColor *) getBorderColor
{
	return [NSColor blackColor];
}

- (NSColor *) getFontColor
{
	return [Prefs getSystemColor:GET_COLOR_INACTIVE];
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
	[[NSBundle mainBundle] loadNibNamed:@"jumpPage" owner:self topLevelObjects:nil];
	[controller launchPopover : self : currentPage : pageMax];
}

- (void) transmitPageJump : (uint) newPage
{
	[_target jumpPage:newPage];
}

@end

@interface RakFormatterNumbersOnly : NSNumberFormatter

@end

@implementation RakFormatterNumbersOnly

- (BOOL)isPartialStringValid:(NSString*)partialString newEditingString:(NSString**)newString errorDescription:(NSString**)error
{
	if ([partialString length] == 0)
		return YES;
	
	return [partialString rangeOfCharacterFromSet:[[NSCharacterSet decimalDigitCharacterSet] invertedSet]].location == NSNotFound;
}

@end

@implementation RakPageCounterPopoverController

- (void) launchPopover : (NSView *) anchor : (uint) curPage : (uint) maxPage
{
	_anchor = anchor;
	_maxPage = maxPage;
	
	if(mainLabel != nil)
	{
		[mainLabel setStringValue:@"Aller à:"];
		[mainLabel sizeToFit];
		[mainLabel setTextColor:[Prefs getSystemColor:GET_COLOR_ACTIVE]];
	}
	
	if(gotoButton != nil)
	{
		[gotoButton setTitle:@"Go"];
		[gotoButton sizeToFit];
		[[gotoButton cell] setBackgroundColor:[Prefs getSystemColor:GET_COLOR_INACTIVE]];
		[gotoButton setTarget:self];
		[gotoButton setAction:@selector(jumpTrigered)];
	}
	
	if(textField != nil)
	{
		[textField setBackgroundColor:[NSColor blackColor]];
		[textField setTextColor:[Prefs getSystemColor:GET_COLOR_CLICKABLE_TEXT]];
		[textField setBezeled:NO];

		((NSTextView*) [textField.window fieldEditor:YES forObject:textField]).insertionPointColor = [Prefs getSystemColor:GET_COLOR_CLICKABLE_TEXT];
		[textField setFormatter:[[[RakFormatterNumbersOnly alloc] init] autorelease]];
		
		//Enter key is pressed
		[textField setTarget:self];
		[textField setAction:@selector(jumpTrigered)];

	}
	
	if(popover != nil)
	{
		[popover setAppearance:NSPopoverAppearanceHUD];
		[popover setBehavior:NSPopoverBehaviorTransient];
		[popover showRelativeToRect:[_anchor bounds] ofView:_anchor preferredEdge:NSMinYEdge];
	}
}

- (void) jumpTrigered
{
	NSInteger value = [[textField stringValue] integerValue];
	
	if(value > 0)
	{
		if (value >= _maxPage)
		{
			[textField setStringValue:[NSString stringWithFormat:@"%d", _maxPage]];
		}
		else
		{
			if([_anchor class] == [RakPageCounter class])
				[(RakPageCounter*) _anchor transmitPageJump : value - 1];
			
			[popover close];
		}
	}
}

@end
