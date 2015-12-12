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

@interface RakReaderSuggestions()
{
	RakText * header;
	NSArray <RakThumbProjectView *> * thumbs;
	RakButton * cancel, * discardForEvar;
	BOOL defaultValueDiscard;
}

@end

@implementation RakReaderSuggestions

- (instancetype) autoInit
{
	return [self initWithFrame: NSMakeRect(0, 0, 42, 155)];
}

- (void) launchPopover : (NSView *) anchor withProjectID : (uint) cacheDBID
{
	//We check if the user asked not to be annoyed again
	BOOL haveValue = [RakPrefsRemindPopover getValueReminded : PREFS_REMIND_SUGGESTION : &defaultValueDiscard];
	if(!haveValue || !defaultValueDiscard || [(RakAppDelegate*) [NSApp delegate] window].shiftPressed)
	{
		_anchor = anchor;
		_cacheDBID = cacheDBID;
		
		NSArray <NSDictionary *> * data = [[RakSuggestionEngine getShared] getSuggestionForProject:cacheDBID withNumber:2];
		
		if([data count] != 0)
		{
			[self createUIItems:data];
			
			if(thumbs != nil && [thumbs count] > 0)
				[self internalInit: anchor : NSMakeRect(0, 0, _anchor.frame.size.width, 0) : YES];
		}
	}
}

#pragma mark - View configuration

- (void) createUIItems : (NSArray < NSDictionary * > *) data
{
	NSMutableArray <RakThumbProjectView *> * workingThumbs = [[NSMutableArray alloc] initWithCapacity:[data count]];
	if(workingThumbs == nil)
		return;
	
	for(NSDictionary * dict in data)
	{
		RakThumbProjectView * view = [[RakThumbProjectView alloc] initWithProject:[[RakSuggestionEngine getShared] dataForIndex:[[dict objectForKey:@"ID"] unsignedIntValue]] reason:[(NSNumber *) [dict objectForKey:@"reason"] unsignedCharValue]];
		
		if(view != nil)
		{
			view.controller = self;
			view.clickValidation = @selector(receiveClick:forClick:);
			view.mustHoldTheWidth = YES;
			[workingThumbs addObject:view];
		}
	}
	
	thumbs = [NSArray arrayWithArray:workingThumbs];
}

- (void) setupView
{
	NSSize firstItemSize = [thumbs firstObject].bounds.size, newSize = _bounds.size;
	CGFloat baseX = 8;
	
	//Create the buttons
	cancel = [RakButton allocWithText:NSLocalizedString(@"CANCEL", nil)];
	if(cancel != nil)
	{
		cancel.target = self;
		cancel.action = @selector(close);
		[self addSubview:cancel];
	}
	
	discardForEvar = [RakButton allocWithText:NSLocalizedString(@"READER-DISCARD-FOREVER", nil)];
	if(discardForEvar != nil)
	{
		((RakButtonCell *) discardForEvar.cell).activeAllowed = YES;
		discardForEvar.state = defaultValueDiscard ? NSOnState : NSOffState;
		discardForEvar.target = self;
		discardForEvar.action = @selector(dontShowAgain);
		[self addSubview:discardForEvar];
	}
	
	//Update our width
	[self setFrameSize:newSize = NSMakeSize(baseX + [thumbs count] * (firstItemSize.width + baseX), firstItemSize.height + 28 + discardForEvar.bounds.size.height + 5)];
	
	header = [[RakText alloc] initWithText:NSLocalizedString(@"READER-SUGG-TITLE", nil) :[Prefs getSystemColor:COLOR_SURVOL]];
	if(header != nil)
	{
		if(newSize.width < header.bounds.size.width + 10)
		{
			baseX += header.bounds.size.width / 2 - newSize.width / 2;
			[self setFrameSize:newSize = NSMakeSize(header.bounds.size.width + 10, newSize.height)];
		}
		
		[header setFrameOrigin:NSMakePoint(newSize.width / 2 - header.bounds.size.width / 2, newSize.height - header.bounds.size.height - 3)];
		[self addSubview:header];
	}
	
	CGFloat baseHeaderY = header.frame.origin.y - 8, baseY = baseHeaderY;
	
	for(RakThumbProjectView * view in thumbs)
	{
		[view setFrameOrigin:NSMakePoint(baseX, baseHeaderY - view.bounds.size.height)];
		[self addSubview:view];
		
		baseX += 8 + firstItemSize.width;
		baseY = MIN(baseY, view.frame.origin.y);
	}
	
	[cancel setFrameOrigin:NSMakePoint(newSize.width / 4 - cancel.bounds.size.width / 2, 8)];
	[discardForEvar setFrameOrigin:NSMakePoint(newSize.width * 3 / 4 - discardForEvar.bounds.size.width / 2, 8)];
}

- (void) configurePopover:(INPopoverController *)internalPopover
{
	[super configurePopover:internalPopover];
	
	internalPopover.closesWhenApplicationBecomesInactive = YES;
	internalPopover.closesWhenPopoverResignsKey = YES;
}

#pragma mark - Button target

- (BOOL) receiveClick : (RakThumbProjectView *) project forClick : (byte) selection
{
	if(selection != THUMBVIEW_CLICK_NONE)
		[self close];

	if(selection != THUMBVIEW_CLICK_PROJECT)
		return YES;
	
	//Return YES will open the CT tab
	//Return NO will tell the routine we did the work, and to simply return
	BOOL retvalue = ![RakSuggestionEngine suggestionWasClicked:project.elementID];
	
	if(!retvalue && [[NSApp delegate] reader].distractionFree != _openedLeavingDFMode)
	{
		[[[NSApp delegate] reader] switchDistractionFree];
	}
	
	return retvalue;
}

- (void) close
{
	[self closePopover];
}

- (void) dontShowAgain
{
	[RakPrefsRemindPopover setValueReminded:PREFS_REMIND_SUGGESTION :discardForEvar.state == NSOnState];
}

@end
