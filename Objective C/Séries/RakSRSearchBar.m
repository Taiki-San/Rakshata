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

#define OFFSET 22

@implementation RakSRSearchBarCell

- (NSText*) setUpFieldEditorAttributes : (NSText*) textObj
{
	NSTextView * output = (NSTextView*) [super setUpFieldEditorAttributes:textObj];
	
	[output setInsertionPointColor:[Prefs getSystemColor:GET_COLOR_INSERTION_POINT :nil]];
	[output setSelectedTextAttributes: @{NSBackgroundColorAttributeName : [Prefs getSystemColor:GET_COLOR_SEARCHBAR_SELECTION_BACKGROUND :nil],
										 NSForegroundColorAttributeName : [Prefs getSystemColor:GET_COLOR_SURVOL :nil]}];
	
	output.drawsBackground = YES;
	output.backgroundColor = [RakSRSearchBar getBackgroundColor];
	
	return output;
}

- (void) endEditing:(NSText *)textObj
{
	[textObj setSelectedRange:NSMakeRange(0, 0)];
	
	//We discovered on Yosemite that if bezel was disabled, things would go _very_ wrong
	//Basically, positionning of text would get completly nuts
	//We fixed most of the issues with the hooks below, but one remained, if text was inputed, then focus was lost,
	//we couldn't fix the y positionning. It seems that kill this function achieve that without significant issues
	
	if([self.stringValue isEqualToString:@""])
	{
		[super endEditing:textObj];
		[(RakSRSearchBar *) self.controlView updatePlaceholder:YES];
	}
}

- (void) selectWithFrame:(NSRect)aRect inView:(NSView *)controlView editor:(NSText *)textObj delegate:(id)anObject start:(NSInteger)selStart length:(NSInteger)selLength
{
	if([controlView isKindOfClass:[RakSRSearchBar class]] && [[(RakSRSearchBar *) controlView stringValue] isEqualToString:@""])
		[(RakSRSearchBar *) controlView updatePlaceholder:NO];
	
	[super selectWithFrame:[self centerCell:aRect] inView:controlView editor:textObj delegate:anObject start:selStart length:selLength];
}

- (void) editWithFrame:(NSRect)aRect inView:(NSView *)controlView editor:(NSText *)textObj delegate:(id)anObject event:(NSEvent *)theEvent
{
	if([controlView isKindOfClass:[RakSRSearchBar class]] && [[(RakSRSearchBar *) controlView stringValue] isEqualToString:@""])
		[(RakSRSearchBar *) controlView updatePlaceholder:NO];
	
	[super editWithFrame:[self centerCell:aRect] inView:controlView editor:textObj delegate:anObject event:theEvent];
}

- (NSRect) centerCell : (NSRect) originalRect
{
	NSSize textSize = [self cellSizeForBounds:originalRect];
	double heightDelta = originalRect.size.height - textSize.height;
	
	if (heightDelta > 0)
	{
		originalRect.size.height -= heightDelta;
		originalRect.origin.y += (heightDelta / 2);
	}
	
	originalRect.origin.x += OFFSET;
	originalRect.size.width -= 2 * OFFSET;
	
	return originalRect;
}

@end

@implementation RakSRSearchBar

- (instancetype) initWithFrame : (NSRect) frameRect : (byte) ID
{
	self = [super initWithFrame:frameRect];
	
	if(self != nil)
	{
		_ID = ID;
		_currentPlaceholderState = YES;
		
		[self initCell];
		
		self.bezeled = NO;
		self.drawsBackground = NO;
		self.focusRingType = NSFocusRingTypeNone;
		self.textColor = [self getTextColor];
		
		self.wantsLayer = YES;
		self.layer.cornerRadius = 4;
		self.layer.backgroundColor = [RakSRSearchBar getBackgroundColor].CGColor;
		self.layer.borderWidth = 1;
		self.layer.borderColor = [self getBorderColor].CGColor;
		
		[self updatePlaceholder:YES];
		[Prefs getCurrentTheme:self];
		
		self.delegate = self;
		[self.cell setSendsWholeSearchString:NO];
		[self.cell setSendsSearchStringImmediately:YES];
		
		self.target = self;
		self.action = @selector(performSearch);
		self.recentsAutosaveName = [NSString stringWithFormat:@"RakRecentSearch#%d", _ID];
	}
	
	return self;
}

- (void) initCell
{
	NSSearchFieldCell * cell = self.cell;
	RakButtonCell * button = [[RakButtonCell alloc] initWithPage : @"loupe" : RB_STATE_STANDARD];
	if(button != nil)
	{
		[button setBordered:NO];
		[button setActiveAllowed:NO];
		
		NSButtonCell * _oldButton = cell.searchButtonCell;
		button.target = _oldButton.target;
		button.action = _oldButton.action;
		
		[cell setSearchButtonCell:button];
	}
	
	button = [[RakButtonCell alloc] initWithPage:@"discard" :RB_STATE_STANDARD];
	if(button != nil)
	{
		[button setBordered:NO];
		[button setActiveAllowed:NO];
		
		button.target = self;
		button.action = @selector(cancelProxy);
		
		[cell setCancelButtonCell:button];
	}
}

+ (void) triggeringSearchBar : (BOOL) goingIn : (byte) ID
{
	if([NSThread isMainThread])
		[[NSNotificationCenter defaultCenter] postNotificationName: SR_NOTIF_NAME_SEARCH_TRIGGERED
															object:@(ID * SEARCH_BAR_ID_NBCASES + 1) userInfo: @{SR_NOTIF_NEW_STATE:@(goingIn)}];
	else
		dispatch_async(dispatch_get_main_queue(), ^{	[self triggeringSearchBar:goingIn:ID];	});
}

+ (Class)cellClass
{
	return [RakSRSearchBarCell class];
}

#pragma mark - Interface

+ (NSColor *) getBackgroundColor
{
	return [Prefs getSystemColor:GET_COLOR_SEARCHBAR_BACKGROUND :nil];
}

- (NSColor *) getBorderColor
{
	return [Prefs getSystemColor:GET_COLOR_SEARCHBAR_BORDER :nil];
}

- (NSColor *) getPlaceholderTextColor
{
	return [Prefs getSystemColor:GET_COLOR_SEARCHBAR_PLACEHOLDER_TEXT :nil];
}

- (NSColor *) getTextColor
{
	return [Prefs getSystemColor:GET_COLOR_ACTIVE :nil];
}

- (NSString *) placeholderMessage
{
	if(_ID == SEARCH_BAR_ID_MAIN)
		return NSLocalizedString(@"PROJ-SEARCH-MAIN", nil);
	
	else if(_ID == SEARCH_BAR_ID_AUTHOR)
		return NSLocalizedString(@"PROJ-SEARCH-AUTHOR", nil);
	
	else if(_ID == SEARCH_BAR_ID_SOURCE)
		return NSLocalizedString(@"PROJ-SEARCH-SOURCE", nil);
	
	else if(_ID == SEARCH_BAR_ID_TAG)
		return NSLocalizedString(@"PROJ-SEARCH-TAGS", nil);
	
	else if(_ID == SEARCH_BAR_ID_TYPE)
		return NSLocalizedString(@"PROJ-SEARCH-TYPE", nil);
	
	NSLog(@"Not supported yet");
	return @"Oops, erreur :/";
}

- (void) updatePlaceholder : (BOOL) inactive
{
	_currentPlaceholderState = inactive;
	
	[self.cell setPlaceholderAttributedString:[[NSAttributedString alloc] initWithString:[self placeholderMessage] attributes:
											   @{NSForegroundColorAttributeName : [self getPlaceholderTextColor],
												 NSBackgroundColorAttributeName : [RakSRSearchBar getBackgroundColor],
												 NSBaselineOffsetAttributeName : @(inactive ? -3 : 0)}]];
	
	[RakSRSearchBar triggeringSearchBar : !inactive : _ID];
}

- (void) willLooseFocus
{
	[self updatePlaceholder:YES];
}

- (void) cancelProxy
{
	dispatch_async(dispatch_get_main_queue(), ^{
		[self.window makeFirstResponder:[[NSApp delegate] serie]];
	});
}

#pragma mark - View stuffs

- (void) resizeAnimation : (NSRect) frame
{
	[self.animator setFrame:frame];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if ([object class] != [Prefs class])
		return;
	
	[self initCell];
	
	self.layer.backgroundColor = [RakSRSearchBar getBackgroundColor].CGColor;
	self.layer.borderColor = [self getBorderColor].CGColor;
	
	[self.cell setTextColor:[self getTextColor]];
	[self updatePlaceholder : _currentPlaceholderState];
}

#pragma mark - Logic behind auto-completion

- (void) performSearch
{
	if(noRecursive)
		return;
	
	if(normalKeyPressed)
	{
		normalKeyPressed = NO;
	}
	else
	{
		[self willLooseFocus];
		[[NSNotificationCenter defaultCenter] postNotificationName:NOTIFICATION_SEARCH_UPDATED object:self.stringValue];
	}
}

- (NSArray *)control:(NSControl *)control textView:(NSTextView *)textView completions:(NSArray *)words forPartialWordRange:(NSRange)charRange indexOfSelectedItem:(NSInteger *)index
{
	uint nbElem;
	char ** output = getProjectNameStartingWith([textView.string cStringUsingEncoding:NSUTF8StringEncoding], &nbElem);
	
	if(output == NULL || nbElem == 0)
	{
		free(output);
		return @[];
	}
	
	NSMutableArray * array = [NSMutableArray array];
	
	for(uint i = 0; i < nbElem; i++)
	{
		[array addObject:[NSString stringWithUTF8String:output[i]]];
		free(output[i]);
	}
	free(output);
	
	return array;
}

#pragma mark - Delegate code

- (void)controlTextDidChange:(NSNotification *)obj
{
	NSTextView * view = [obj.userInfo objectForKey:@"NSFieldEditor"];
	
	if(!noRecursive && view != nil && ![view.string isEqualToString:@""])
	{
		normalKeyPressed = YES;
		noRecursive = YES;
		[view complete:nil];
		noRecursive = NO;
	}
}

- (BOOL)control:(NSControl *)control textView:(NSTextView *)textView doCommandBySelector:(SEL)commandSelector
{
	BOOL result = NO;
	
	if ([textView respondsToSelector:commandSelector])
	{
		noRecursive = YES;
		
		normalKeyPressed = commandSelector != @selector(insertNewline:);
		
		IMP imp = [textView methodForSelector:commandSelector];
		void (*func)(id, SEL, id) = (void *)imp;
		func(textView, commandSelector, nil);
		
		noRecursive = NO;
		
		result = YES;
	}
	
	return result;
}

@end