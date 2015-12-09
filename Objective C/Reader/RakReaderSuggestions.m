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
	BOOL answer = NO, haveValue = [RakPrefsRemindPopover getValueReminded : PREFS_REMIND_SUGGESTION : &answer];
	if(!haveValue || !answer || [(RakAppDelegate*) [NSApp delegate] window].shiftPressed)
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
			[workingThumbs addObject:view];
	}
	
	thumbs = [NSArray arrayWithArray:workingThumbs];
}

- (void) setupView
{
	NSSize firstItemSize = [thumbs firstObject].bounds.size, newSize = _bounds.size;
	CGFloat baseX = 8;
	
	//Update our width
	[self setFrameSize:newSize = NSMakeSize(baseX + [thumbs count] * (firstItemSize.width + baseX), MAX(newSize.height, firstItemSize.height + 28))];
	
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
	
	for(RakThumbProjectView * view in thumbs)
	{
		[view setFrameOrigin:NSMakePoint(baseX, 5)];
		[self addSubview:view];
		
		baseX += 8 + firstItemSize.width;
	}
}

- (void) configurePopover:(INPopoverController *)internalPopover
{
	[super configurePopover:internalPopover];
	
	internalPopover.closesWhenApplicationBecomesInactive = YES;
	internalPopover.closesWhenPopoverResignsKey = YES;
}

@end
