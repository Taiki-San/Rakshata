/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at https://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 ********************************************************************************************/

@implementation RakPageCounter

- (instancetype) init: (RakView*) superview : (CGFloat) posX : (uint) currentPageArg : (uint) pageMaxArg : (Reader *) target
{
	if(target.initWithNoContent)
		return nil;
	
	self = [super initWithText:[superview bounds] :[NSString stringWithFormat:@"%d/%d", currentPageArg+1, pageMaxArg] :[self getFontColor]];
	if(self != nil)
	{
		[self setFont:[NSFont boldSystemFontOfSize:13]];
		[self updateSize:[superview bounds].size.height : posX];
		[Prefs registerForChange:self forType:KVO_THEME];
		
		currentPage = currentPageArg+1;
		pageMax = pageMaxArg;
		_target = target;
		
		self.drawsBackground = YES;
		[self setBackgroundColor:[self getColorBackground]];
	}
	return self;
}

- (void) dealloc
{
	[Prefs deRegisterForChange:self forType:KVO_THEME];
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

- (RakColor *) getColorBackground
{
	return [Prefs getSystemColor:COLOR_READER_BAR_PAGE_COUNTER];
}

- (RakColor *) getBorderColor
{
	return [RakColor blackColor];
}

- (RakColor *) getFontColor
{
	return [Prefs getSystemColor:COLOR_INACTIVE];
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	
	[self setTextColor:[self getFontColor]];
	[self setBackgroundColor:[self getColorBackground]];
	[self setNeedsDisplay:YES];
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
	pageMax = newPageMax;
	
	[self updateContext];
}

#pragma mark - Events

- (void) mouseDown:(NSEvent *)theEvent
{
	if(![self openPopover])
		[super mouseDown:theEvent];
}

- (BOOL) openPopover
{
	if(popover == nil && !popoverStillAround)
	{
		[[NSBundle mainBundle] loadNibNamed:@"jumpPage" owner:self topLevelObjects:nil];
		[popover launchPopover : self : currentPage : pageMax];
		return YES;
	}
	return NO;
}

- (void) closePopover
{
	[popover closePopover];
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
	if([partialString length] == 0)
		return YES;
	
	if([partialString rangeOfCharacterFromSet:[[NSCharacterSet decimalDigitCharacterSet] invertedSet]].location != NSNotFound)
		return NO;

	NSNumber * content = getNumberForString(partialString);
	
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

- (void) launchPopover : (RakView *) anchor : (uint) curPage : (uint) maxPage
{
	_anchor = anchor;
	_maxPage = maxPage;
	
	[self internalInitWithAnchor:anchor
						 atFrame:NSMakeRect(0, 0, _anchor.frame.size.width, _anchor.frame.size.height)
					wantCallback:YES
			closeOnContextChange:YES];
	
	[self.window makeFirstResponder : textField];
}

- (void) setupView
{
	if(mainLabel != nil)
	{
		[mainLabel setStringValue:NSLocalizedString(@"READER-JUMP-TO", nil)];
		[mainLabel sizeToFit];
		[mainLabel setTextColor:[Prefs getSystemColor:COLOR_ACTIVE]];
	}
	
	if(gotoButtonContainer != nil)
	{
		RakButton * gotoButton = [RakButton allocWithText : NSLocalizedString(@"GO", nil) : gotoButtonContainer.bounds];
		if(gotoButton != nil)
		{
			[gotoButton setTarget:self];
			[gotoButton setAction:@selector(jumpTrigered)];
			[gotoButtonContainer addSubview:gotoButton];
		}
	}
	
	if(textField != nil)
	{
		((RakTextCell*)textField.cell).customizedInjectionPoint = YES;
		[textField setBackgroundColor:[Prefs getSystemColor:COLOR_TEXTFIELD_BACKGROUND]];
		[textField setTextColor:[Prefs getSystemColor:COLOR_CLICKABLE_TEXT]];
		[textField setBezeled:NO];
		
		RakFormatterNumbersOnly * formater = [[RakFormatterNumbersOnly alloc] init];
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
	
	if(newSize.width < self.frame.size.height)
		[super setFrameSize:newSize];
}

- (void) additionalUpdateOnThemeChange
{
	if(mainLabel != nil)
	{
		[mainLabel setTextColor:[Prefs getSystemColor:COLOR_ACTIVE]];
		[mainLabel setNeedsDisplay:YES];
	}
	
	if(textField != nil)
	{
		[textField setBackgroundColor:[Prefs getSystemColor:COLOR_TEXTFIELD_BACKGROUND]];
		[textField setTextColor:[Prefs getSystemColor:COLOR_CLICKABLE_TEXT]];
		[textField setNeedsDisplay:YES];
	}
}

#pragma mark - Payload

- (void) jumpTrigered
{
	NSInteger value = [[textField stringValue] integerValue];
	
	if(value > 0)
	{
		if(value > _maxPage)
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
