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
 *********************************************************************************************/

#define OFFSET 22

@interface RakSRSearchBar ()
{
	APPLSuggestionsWindowController * _suggestionsController;
}

@end

@implementation RakSRSearchBarCell

- (NSText*) setUpFieldEditorAttributes : (NSText*) textObj
{
	NSTextView * output = (NSTextView*) [super setUpFieldEditorAttributes:textObj];
	
	[output setInsertionPointColor:[Prefs getSystemColor:COLOR_INSERTION_POINT]];
	[output setSelectedTextAttributes: @{NSBackgroundColorAttributeName : [Prefs getSystemColor:COLOR_SEARCHBAR_SELECTION_BACKGROUND],
										 NSForegroundColorAttributeName : [Prefs getSystemColor:COLOR_SURVOL]}];
	
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

- (void) selectWithFrame:(NSRect)aRect inView:(RakView *)controlView editor:(NSText *)textObj delegate:(id)anObject start:(NSInteger)selStart length:(NSInteger)selLength
{
	if([controlView isKindOfClass:[RakSRSearchBar class]] && [[(RakSRSearchBar *) controlView stringValue] isEqualToString:@""])
		[(RakSRSearchBar *) controlView updatePlaceholder:NO];
	
	[super selectWithFrame:[self centerCell:aRect] inView:controlView editor:textObj delegate:anObject start:selStart length:selLength];
}

- (void) editWithFrame:(NSRect)aRect inView:(RakView *)controlView editor:(NSText *)textObj delegate:(id)anObject event:(NSEvent *)theEvent
{
	if([controlView isKindOfClass:[RakSRSearchBar class]] && [[(RakSRSearchBar *) controlView stringValue] isEqualToString:@""])
		[(RakSRSearchBar *) controlView updatePlaceholder:NO];
	
	[super editWithFrame:[self centerCell:aRect] inView:controlView editor:textObj delegate:anObject event:theEvent];
}

- (NSRect) centerCell : (NSRect) originalRect
{
	NSSize textSize = [self cellSizeForBounds:originalRect];
	double heightDelta = originalRect.size.height - textSize.height;
	
	if(heightDelta > 0)
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
		
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(fullFlush) name:SR_NOTIFICATION_FULL_UNSELECTION_TRIGGERED object:nil];

		CONFIGURE_APPEARANCE_DARK(self);
		self.bezeled = NO;
		self.drawsBackground = NO;
		self.focusRingType = NSFocusRingTypeNone;
		self.textColor = [self getTextColor];
		
		self.wantsLayer = YES;
		self.layer.backgroundColor = [RakSRSearchBar getBackgroundColor].CGColor;
		self.layer.cornerRadius = 4;
		self.layer.borderWidth = 1;
		self.layer.borderColor = [self getBorderColor].CGColor;
		
		[self updatePlaceholder:YES];
		[Prefs registerForChange:self forType:KVO_THEME];
		
		self.delegate = self;
		[self.cell setSendsWholeSearchString:NO];
		[self.cell setSendsSearchStringImmediately:YES];
		
		self.target = self;
		self.action = @selector(performSearch);
		self.recentsAutosaveName = [NSString stringWithFormat:@"RakRecentSearch#%d", _ID];
	}
	
	return self;
}

- (instancetype) initWithFrame : (NSRect) frameRect ID : (byte) ID andData : (charType **) names ofSize : (uint) nbData andIndexes : (uint64_t *) listIndexes
{
	self = [self initWithFrame:frameRect :ID];

	if(self != nil)
		[self updateData:names ofSize:nbData andIndexes:listIndexes];
	
	return self;
}

- (void) updateData : (charType **) names ofSize : (uint) nbData andIndexes : (uint64_t *) listIndexes
{
	NSMutableArray * array = [NSMutableArray new];
	
	if(names != nil)
	{
		for(uint i = 0; i < nbData; ++i)
		{
			[array addObject:getStringForWchar(names[i])];
		}
	}
	
	indexes = listIndexes;
	data = [NSArray arrayWithArray:array];
}

- (void) initCell
{
	NSSearchFieldCell * cell = self.cell;
	RakButtonCell * button = [[RakButtonCell alloc] initWithPage : @"search"];
	if(button != nil)
	{
		[button setBordered:NO];
		[button setActiveAllowed:NO];
		
		NSButtonCell * _oldButton = cell.searchButtonCell;
		button.target = _oldButton.target;
		button.action = _oldButton.action;
		
		[cell setSearchButtonCell:button];
	}
	
	button = [[RakButtonCell alloc] initWithPage:@"discard"];
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

- (void) dealloc
{
	[Prefs deRegisterForChange:self forType:KVO_THEME];
	[[NSNotificationCenter defaultCenter] removeObserver:self];
}

#pragma mark - Interface

+ (RakColor *) getBackgroundColor
{
	return [Prefs getSystemColor:COLOR_SEARCHBAR_BACKGROUND];
}

- (RakColor *) getBorderColor
{
	return [Prefs getSystemColor:COLOR_SEARCHBAR_BORDER];
}

- (RakColor *) getPlaceholderTextColor
{
	return [Prefs getSystemColor:COLOR_SEARCHBAR_PLACEHOLDER_TEXT];
}

- (RakColor *) getTextColor
{
	return [Prefs getSystemColor:COLOR_HIGHLIGHT];
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
	
	else if(_ID == SEARCH_BAR_ID_CAT)
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
	if(self.cell.stringValue != nil && ![self.cell.stringValue isEqualToString:@""])
		self.cell.stringValue = @"";

	dispatch_async(dispatch_get_main_queue(), ^{
		[self.window makeFirstResponder:RakApp.serie];
		didAuthorizeSearch = YES;
		[self performSearch];
	});
}

- (void) fullFlush
{
	[self cancelProxy];
}

#pragma mark - View stuffs

- (void) resizeAnimation : (NSRect) frame
{
	[self setFrameAnimated:frame];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return;
	
	[self initCell];
	
	self.layer.backgroundColor = [RakSRSearchBar getBackgroundColor].CGColor;
	self.layer.borderColor = [self getBorderColor].CGColor;
	
	[self.cell setTextColor:[self getTextColor]];
	[self updatePlaceholder : _currentPlaceholderState];
	
	[self setNeedsDisplay:YES];
}

#pragma mark - Logic behind auto-completion

- (void) performSearch
{
	if(noRecursive || !didAuthorizeSearch)
		return;
	
	didAuthorizeSearch = NO;
	
	if(normalKeyPressed)
	{
		normalKeyPressed = NO;
		
		if(_ID == SEARCH_BAR_ID_AUTHOR || _ID == SEARCH_BAR_ID_SOURCE || _ID == SEARCH_BAR_ID_TAG || _ID == SEARCH_BAR_ID_CAT)
		{
			NSString * string = self.cell.stringValue;
			
			NSUInteger index = [data indexOfObject:string];
			if(index != NSNotFound)
			{
				[[NSNotificationCenter defaultCenter] postNotificationName:getNotificationNameForSBID(_ID) object:string userInfo:@{SR_NOTIF_CACHEID : @(indexes[index]), SR_NOTIF_OPTYPE : @(YES)}];
			}
		}
	}
	else
	{
		[self willLooseFocus];
		[[NSNotificationCenter defaultCenter] postNotificationName:NOTIFICATION_SEARCH_UPDATED object:self.stringValue];
	}
}

- (NSArray *)control:(NSControl *)control textView:(NSTextView *)textView completions:(NSArray *)words forPartialWordRange:(NSRange)charRange indexOfSelectedItem:(NSInteger *)index
{
	return [self suggestionsForText:textView.string];
}

#pragma mark - Delegate code

- (void)controlTextDidBeginEditing:(NSNotification *)notification
{
	// We keep the suggestionsController around, but lazely allocate it the first time it is needed.
	if (!_suggestionsController)
	{
		_suggestionsController = [[APPLSuggestionsWindowController alloc] init];
		_suggestionsController.target = self;
		_suggestionsController.action = @selector(updateWithSelectedSuggestion:);
	}
	
	[self shouldSuggest : notification.object];
}

- (void)controlTextDidChange:(NSNotification *)obj
{
	NSTextView * view = [obj.userInfo objectForKey:@"NSFieldEditor"];
	
	if(!noRecursive && view != nil && ![view.string isEqualToString:@""])
	{
		if(deletinChar)
		{
			[_suggestionsController cancelSuggestions];
			deletinChar = NO;
		}
		else
		{
			normalKeyPressed = YES;
			noRecursive = YES;
			[self shouldSuggest : view];
//			[view complete:nil];
			noRecursive = NO;
		}
	}
}

- (void)controlTextDidEndEditing:(NSNotification *)obj
{
	//If the suggestionController is already in a cancelled state, this call does nothing and is therefore always safe to call.
	[_suggestionsController cancelSuggestions];
}

- (BOOL)control:(NSControl *)control textView:(NSTextView *)textView doCommandBySelector:(SEL)commandSelector
{
	BOOL result = NO;
	
	if(commandSelector == @selector(insertTab:))
	{
		if(textView.nextKeyView != self.nextKeyView)
			textView.nextKeyView = self.nextKeyView;
	}
	
	
	else if (commandSelector == @selector(moveUp:))
	{
		// Move up in the suggested selections list
		[_suggestionsController moveUp:textView];
		return YES;
	}
	
	else if (commandSelector == @selector(moveDown:))
	{
		// Move down in the suggested selections list
		[_suggestionsController moveDown:textView];
		return YES;
	}
	
	else if (commandSelector == @selector(complete:))
	{
		// The user has pressed the key combination for auto completion. AppKit has a built in auto completion. By overriding this command we prevent AppKit's auto completion and can respond to the user's intention by showing or cancelling our custom suggestions window.
		if ([_suggestionsController.window isVisible])
			[_suggestionsController cancelSuggestions];
		else
			[self shouldSuggest : textView];
		
		return YES;
	}
	
	else if([textView respondsToSelector:commandSelector])
	{
		if(commandSelector == @selector(insertNewline:))
		{
			didAuthorizeSearch = YES;
			normalKeyPressed = NO;
		}
		else
		{
			if(commandSelector == @selector(deleteBackward:))
				deletinChar = YES;

			normalKeyPressed = YES;
		}
		
		IMP imp = [textView methodForSelector:commandSelector];
		void (*func)(id, SEL, id) = (void *)imp;
		func(textView, commandSelector, nil);
		
		result = YES;
	}
	
	return result;
}

#pragma mark - UI Completion hooks

/* Update the field editor with a suggested string. The additional suggested characters are auto selected.
 */
- (void)updateFieldEditor:(NSText *)fieldEditor withSuggestion:(NSString *)suggestion
{
	NSRange selection = NSMakeRange([fieldEditor selectedRange].location, [suggestion length]);
	[fieldEditor setString:suggestion];
	[fieldEditor setSelectedRange:selection];
}

- (void) updateWithSelectedSuggestion : (id)sender
{
	NSDictionary * entry = [sender selectedSuggestion];
	if (entry != nil)
	{
		NSText *fieldEditor = [self.window fieldEditor:NO forObject:self];
		if (fieldEditor)
		{
			[self updateFieldEditor:fieldEditor withSuggestion:[entry objectForKey:kSuggestionString]];
		}
	}
}

- (void) shouldSuggest : (NSView *) view
{
	if(_ID != SEARCH_BAR_ID_MAIN)
	{
		[view complete:nil];
	}
	
	NSText *fieldEditor = [self.window fieldEditor:NO forObject:self];
	if (fieldEditor != nil)
	{
		// Only use the text up to the caret position
		NSRange selection = [fieldEditor selectedRange];
		NSString *text = [[fieldEditor string] substringToIndex:selection.location];
		
		NSArray *suggestions = [self suggestionsForText:text];
		if ([suggestions count] > 0)
		{
			// We have at least 1 suggestion. Update the field editor to the first suggestion and show the suggestions window.
			NSDictionary *suggestion = [suggestions objectAtIndex:0];
			[self updateFieldEditor:fieldEditor withSuggestion:[suggestion objectForKey:kSuggestionString]];
			
			[_suggestionsController setSuggestions:suggestions];
			if (![_suggestionsController.window isVisible])
			{
				[_suggestionsController beginForTextField:self];
			}
		}
		else
		{
			// No suggestions. Cancel the suggestion window
			[_suggestionsController cancelSuggestions];
		}
	}

}

- (NSArray *) suggestionsForText : (NSString *) prefix
{
	NSMutableArray * array = @[].mutableCopy;
	uint length = [prefix length];
	const char * prefixChar = [prefix UTF8String];
	
	while(length > 0 && prefixChar[length - 1] != ' ')
		length -= 1;
	
	if(_ID == SEARCH_BAR_ID_MAIN)
	{
		uint nbElem;
		SEARCH_SUGGESTION * output = getProjectNameWith([prefix UTF8String], &nbElem, false);
		
		if(output == NULL || nbElem == 0)
		{
			free(output);
			return @[];
		}
		
		for(uint i = 0; i < nbElem; i++)
		{
			[array addObject:@{kSuggestionString:[NSString stringWithUTF8String:&output[i].string[length]],
							   kSuggestionType	:@(output[i].type),
							   kSuggestionID	:@(output[i].cacheDBID)}];
			free(output[i].string);
		}

		free(output);
	}
	
	else if(_ID == SEARCH_BAR_ID_AUTHOR || _ID == SEARCH_BAR_ID_SOURCE || _ID == SEARCH_BAR_ID_TAG || _ID == SEARCH_BAR_ID_CAT)
	{
		[data enumerateObjectsUsingBlock:^(NSString * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop)
		 {
			 if([obj hasPrefix:prefix caseInsensitive:YES])
			 {
				 [array addObject:[obj substringFromIndex:length]];
			 }
		 }];
	}
	
	return array;
}

@end
