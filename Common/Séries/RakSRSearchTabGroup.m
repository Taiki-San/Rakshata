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

enum
{
	BORDER_X_FREE_BUTTON = 10,
	BORDER_FREE_BUTTON_TEXT = 5,
	BORDER_Y_FREE = 8,
	
	BORDER_LIST = 3,
	BORDER_SEARCH_LIST = 4
};

@implementation RakSRSearchTabGroup

- (instancetype) initWithFrame:(NSRect)frameRect : (byte) ID
{
	self = [super initWithFrame:frameRect];
	
	if(self != nil)
	{
		_ID = ID;
		
		if(_ID != SEARCH_BAR_ID_EXTRA)
		{
			[self loadDataForListAndSearch];
			
			list = [[RakSRSearchList alloc] init:[self getTableFrame : _bounds] ofType:_ID withData:listData ofSize:nbDataList andIndexes:indexesData];
			if(list != nil)
			{
				[[NSNotificationCenter defaultCenter] addObserver:list selector:@selector(fullCleanup) name:SR_NOTIFICATION_FULL_UNSELECTION_TRIGGERED object:nil];
				[self addSubview:[list getContent]];
			}
			
			searchBar = [[RakSRSearchBar alloc] initWithFrame:[self getSearchFrame:_bounds] ID:_ID andData:listData ofSize:nbDataList andIndexes:indexesData];
			if(searchBar != nil)
				[self addSubview:searchBar];
			
			[RakDBUpdate registerForUpdate:self :@selector(DBUpdated:)];
			self.backgroundColor = [Prefs getSystemColor:COLOR_SEARCHBAR_BACKGROUND];
		}
		else
		{
			close = [RakButton allocWithText:NSLocalizedString(@"PROJ-CLOSE-FILTER-ARRAY", nil)];
			if(close != nil)
			{
				[close setTarget:self];
				[close setAction:@selector(close)];
				
				[close.cell setActiveAllowed:NO];
				[close.cell setFont:[NSFont fontWithName:[Prefs getFontName:GET_FONT_RD_BUTTONS] size:11]];
				
				[self addSubview:close];
			}
			
			buttonContainer = [[RakView alloc] initWithFrame:NSMakeRect(0, 0, frameRect.size.width, 55)];
			if(buttonContainer != nil)
			{
				buttonContainer.backgroundColor = [Prefs getSystemColor:COLOR_SEARCHBAR_BACKGROUND];
				buttonContainer.layer.cornerRadius = 4;
				
				freeSwitch = [[RakSwitchButton alloc] init];
				if(freeSwitch != nil)
				{
					freeText = [[RakText alloc] initWithText:NSLocalizedString(@"PROJ-FILTER-FREE-ONLY", nil) :[self textColor]];
					if(freeText != nil)
					{
						freeSwitch.state = NSOffState;
						freeSwitch.target = self;
						freeSwitch.action = @selector(triggerFree);
						
						[buttonContainer addSubview:freeSwitch];
						
						freeText.clicTarget = self;
						freeText.clicAction = @selector(switchTriggerFree);
						
						[buttonContainer addSubview:freeText];
					}
					else
						freeSwitch = nil;
				}
				
				favsSwitch = [[RakSwitchButton alloc] init];
				if(freeSwitch != nil)
				{
					favsText = [[RakText alloc] initWithText:NSLocalizedString(@"PROJ-FILTER-FAVS-ONLY", nil) :[self textColor]];
					if(favsText != nil)
					{
						favsSwitch.state = NSOffState;
						favsSwitch.target = self;
						favsSwitch.action = @selector(triggerFavs);
						
						[buttonContainer addSubview:favsSwitch];
						
						favsText.clicTarget = self;
						favsText.clicAction = @selector(switchTriggerFavs);
						
						[buttonContainer addSubview:favsText];
					}
					else
						favsSwitch = nil;
				}
				
				[self setSizeContainer];
				[self addSubview:buttonContainer];
			}
			
			flush = [RakButton allocWithText:NSLocalizedString(@"PROJ-FILTER-FLUSH", nil)];
			if(flush != nil)
			{
				flush.target = self;
				flush.action = @selector(flushFilters);

				[flush.cell setActiveAllowed:NO];
				[flush.cell setFont:[NSFont fontWithName:[Prefs getFontName:GET_FONT_RD_BUTTONS] size:11]];
				
				[self addSubview:flush];
			}
			
			if(close != nil && flush != nil)
			{
				NSSize closeSize = close.frame.size, flushSize = flush.frame.size;
				const CGFloat maxWidth = MAX(frameRect.size.width * 9 / 10.f, MAX(closeSize.width, flushSize.width));
				
				if(closeSize.width != maxWidth)
					[close setFrameSize:NSMakeSize(maxWidth, closeSize.height)];
				
				if(flushSize.width != maxWidth)
					[flush setFrameSize:NSMakeSize(maxWidth, flushSize.height)];
			}
			
			[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(cleanupCustomTab) name:SR_NOTIFICATION_FULL_UNSELECTION_TRIGGERED object:nil];
			
			self.backgroundColor = [Prefs getSystemColor:COLOR_SEARCHBAR_BACKGROUND_EXTRA];
		}
		
		self.layer.cornerRadius = 3;
		[Prefs registerForChange:self forType:KVO_THEME];
	}
	
	return self;
}

- (void) dealloc
{
	if(_ID == SEARCH_BAR_ID_EXTRA)
		[[NSNotificationCenter defaultCenter] removeObserver:self];
	else
	{
		[RakDBUpdate unRegister:self];
	
		_cachedListData = listData;
		_cachedIndexesData = indexesData;
		_cachedNbDataList = nbDataList;
		
		[self releaseData];
	}
	
	[Prefs deRegisterForChange:self forType:KVO_THEME];
}

- (void) releaseData
{
	while(_cachedNbDataList--)
		free(_cachedListData[_cachedNbDataList]);
	
	_cachedNbDataList = 0;
	free(_cachedListData);		_cachedListData = NULL;
	free(_cachedIndexesData);	_cachedIndexesData = NULL;
}

- (void) setFrame:(NSRect)frameRect
{
	[super setFrame:frameRect];
	
	if(_ID == SEARCH_BAR_ID_EXTRA)
	{
		NSPoint newOrigin = NSMakePoint(frameRect.size.width / 2 - close.bounds.size.width / 2, frameRect.size.height - 5 - close.bounds.size.height);
		[close setFrameOrigin:newOrigin];
		
		NSRect switchFrame = flush.bounds;
		newOrigin = NSMakePoint(frameRect.size.width / 2 - switchFrame.size.width / 2, newOrigin.y - BORDER_Y_FREE - switchFrame.size.height);
		[flush setFrameOrigin:newOrigin];
		
		//Free button
		newOrigin = NSMakePoint(0, newOrigin.y - BORDER_Y_FREE - buttonContainer.bounds.size.height);
		[buttonContainer setFrameOrigin:newOrigin];
	}
	else
	{
		[list setFrame:[self getTableFrame:frameRect]];
		[searchBar setFrame:[self getSearchFrame:frameRect]];
	}
}

- (void) resizeAnimation:(NSRect)frameRect
{
	[self setFrameAnimated:frameRect];
	
	if(_ID == SEARCH_BAR_ID_EXTRA)
	{
		NSPoint newOrigin = NSMakePoint(frameRect.size.width / 2 - close.bounds.size.width / 2, frameRect.size.height - 2 - close.bounds.size.height);
		[close setFrameOriginAnimated:newOrigin];
		
		NSRect switchFrame = flush.bounds;
		newOrigin = NSMakePoint(frameRect.size.width / 2 - switchFrame.size.width / 2, newOrigin.y - BORDER_Y_FREE - switchFrame.size.height);
		[flush setFrameOriginAnimated:newOrigin];

		//Free button
		newOrigin = NSMakePoint(0, newOrigin.y - BORDER_Y_FREE - buttonContainer.bounds.size.height);
		[buttonContainer setFrameOriginAnimated:newOrigin];
	}
	else
	{
		[list resizeAnimation:[self getTableFrame:frameRect]];
		[searchBar resizeAnimation:[self getSearchFrame:frameRect]];
	}
}

- (void) setSizeContainer
{
	//Free button
	NSRect bounds = buttonContainer.bounds, switchFrame = freeSwitch.frame, textFrame = freeText.frame;
	CGFloat cumulatedWidth = switchFrame.size.width + BORDER_FREE_BUTTON_TEXT + textFrame.size.width;
	NSPoint newOrigin = NSMakePoint(bounds.size.width / 2 - cumulatedWidth / 2, bounds.size.height - BORDER_Y_FREE - textFrame.size.height), oldOrigin = newOrigin;
	[freeSwitch setFrameOrigin:newOrigin];
	
	newOrigin = NSMakePoint(newOrigin.x + switchFrame.size.width + BORDER_FREE_BUTTON_TEXT, newOrigin.y + (switchFrame.size.height / 2 - textFrame.size.height / 2));
	[freeText setFrameOrigin:newOrigin];
	
	//Favs button
	switchFrame = favsSwitch.frame;
	textFrame = favsText.frame;
	
	newOrigin.x = oldOrigin.x;
	newOrigin.y -= BORDER_Y_FREE + textFrame.size.height;
	[favsSwitch setFrameOrigin:newOrigin];
	
	newOrigin = NSMakePoint(newOrigin.x + switchFrame.size.width + BORDER_FREE_BUTTON_TEXT, newOrigin.y + (switchFrame.size.height / 2 - textFrame.size.height / 2));
	[favsText setFrameOrigin:newOrigin];
}

- (NSRect) getSearchFrame : (NSRect) frame
{
	frame.origin.y = frame.size.height - SR_SEARCH_FIELD_HEIGHT;
	frame.size.height = SR_SEARCH_FIELD_HEIGHT;
	
	frame.origin.x = 0;
	
	return frame;
}

- (NSRect) getTableFrame : (NSRect) frame
{
	frame.size.height -= SR_SEARCH_FIELD_HEIGHT + BORDER_SEARCH_LIST;
	
	frame.size.width -= 2 * BORDER_LIST;
	
	frame.origin.x = BORDER_LIST;
	frame.origin.y = 0;
	
	return frame;
}

#pragma mark - Controller

- (void) loadDataForListAndSearch
{
	sessionID = getSessionForType(getRestrictionTypeForSBID(_ID));

	_cachedListData = listData;			listData = NULL;
	_cachedIndexesData = indexesData;	indexesData = NULL;
	_cachedNbDataList = nbDataList;		nbDataList = 0;
	
	byte type = getRestrictionTypeForSBID(_ID);
	if(type != RDBS_TYPE_SOURCE)
	{
		indexesData = getSearchData(type, &listData, &nbDataList);
		return;
	}

	BOOL fail = NO;
	uint nbElem;
	REPO_DATA ** repo = (REPO_DATA **) getCopyKnownRepo(&nbElem, NO);
	
	if(repo != NULL)
	{
		uint64_t * _indexes = malloc(nbElem * sizeof(uint64_t));
		charType ** output = malloc(nbElem * sizeof(charType *));
		if(output != NULL && _indexes != NULL)
		{
			qsort(repo, nbElem, sizeof(REPO_DATA *), sortRepo);
			
			for(uint i = 0; i < nbElem; i++)
			{
				output[i] = malloc(REPO_NAME_LENGTH * sizeof(charType));
				if(output[i] == NULL)
				{
					while (i-- > 0)
						free(output[i]);
					
					fail = YES;
					break;
				}
				wstrncpy(output[i], REPO_NAME_LENGTH, repo[i]->name);
				_indexes[i] = getRepoID(repo[i]);
			}
			
			if(fail)
			{
				free(_indexes);
				free(output);
			}
			else
			{
				nbDataList = nbElem;
				listData = output;
				indexesData = _indexes;
			}
		}
		else
		{
			free(_indexes);
			free(output);
		}
		
		freeRepo(repo);
	}
}

- (void) DBUpdated : (NSNotification*) notification
{
	if( _ID == SEARCH_BAR_ID_EXTRA ||
	   (_ID == SEARCH_BAR_ID_SOURCE && ![RakDBUpdate getUpdatedRepo: notification.userInfo : NULL]) ||
	   (_ID != SEARCH_BAR_ID_SOURCE && (getSessionForType(getRestrictionTypeForSBID(_ID)) == sessionID || ![RakDBUpdate isProjectUpdate:notification.userInfo])))
		return;
	
	[self loadDataForListAndSearch];
	
	[list updateData:listData ofSize:nbDataList andIndexes:indexesData];
	[searchBar updateData:listData ofSize:nbDataList andIndexes:indexesData];
	
	[self releaseData];
}

- (RakSRSearchBar *) searchBar	{	return searchBar;	}

#pragma mark - Color

- (RakColor *) textColor
{
	return [Prefs getSystemColor:COLOR_SURVOL];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	
	if(_ID == SEARCH_BAR_ID_EXTRA)
	{
		buttonContainer.backgroundColor = [Prefs getSystemColor:COLOR_SEARCHBAR_BACKGROUND];

		if(freeText != nil)
			freeText.textColor = [self textColor];
		
		if(favsText != nil)
			favsText.textColor = [self textColor];

		self.backgroundColor = [Prefs getSystemColor:COLOR_SEARCHBAR_BACKGROUND_EXTRA];
	}
	else
		self.backgroundColor = [Prefs getSystemColor:COLOR_SEARCHBAR_BACKGROUND];
	
	[self setNeedsDisplay:YES];
}

#pragma mark - Responder

- (void) close
{
	[[NSNotificationCenter defaultCenter] postNotificationName: SR_NOTIF_NAME_SEARCH_TRIGGERED
														object:@(SEARCH_BAR_ID_FORCE_CLOSE) userInfo: @{SR_NOTIF_NEW_STATE:@(NO)}];
	[self.window makeFirstResponder:nil];
	close.state = NSOffState;
}

- (void) triggerFree
{
	if(!manualUpdateNoNotif)
		[[NSNotificationCenter defaultCenter] postNotificationName:NOTIFICATION_FREE_ONLY object:@(freeSwitch.state == NSOnState)];
}

- (void) triggerFavs
{
	if(!manualUpdateNoNotif)
		[[NSNotificationCenter defaultCenter] postNotificationName:NOTIFICATION_FAVS_ONLY object:@(favsSwitch.state == NSOnState)];
}

- (void) switchTriggerFree
{
	[freeSwitch performClick:self];
}

- (void) switchTriggerFavs
{
	[favsSwitch performClick:self];
}

- (void) cleanupCustomTab
{
	manualUpdateNoNotif = YES;

	if(freeSwitch.state == NSOnState)
		[self switchTriggerFree];
	
	if(favsSwitch.state == NSOnState)
		[self switchTriggerFavs];
	
	manualUpdateNoNotif = NO;
}

- (void) flushFilters
{
	setLockStatusNotifyRestrictionChanged(true);

	[[NSNotificationCenter defaultCenter] postNotificationName:SR_NOTIFICATION_FULL_UNSELECTION_TRIGGERED object:nil];
	
	setLockStatusNotifyRestrictionChanged(false);
	
	[self close];
	
	notifyRestrictionChanged();
}

@end
