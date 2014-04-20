/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite  **
 **                                                                                         **
 *********************************************************************************************/

@implementation Reader

#pragma mark - Main view management

- (id)init : (NSView*)contentView : (NSString *) state
{
    self = [super init];
    if (self)
	{
		flag = GUI_THREAD_READER;
		gonnaReduceTabs = 0;
		self = [self initView : contentView : state];
		
		[self initReaderMainView : state];
	}
    return self;
}

- (void) initReaderMainView : (NSString *) state
{
	initialized = false;
	bool failedInit = true;
	
	if(state != nil && [state caseInsensitiveCompare:STATE_EMPTY] != NSOrderedSame)
	{
		NSArray *componentsWithSpaces = [state componentsSeparatedByCharactersInSet:[NSCharacterSet newlineCharacterSet]];
		NSArray *dataState = [componentsWithSpaces filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"length > 0"]];
		
		if([dataState count] == 7)
		{
			do
			{
				uint nbElem, indexTeam;
				
				//We first get the index of the team, to perform a search in the DB
				const TEAMS_DATA **tmpData = getDirectAccessToKnownTeams(&nbElem);
				
				if(tmpData == NULL || nbElem == 0)
					break;
				
				const char * URLRepo = [[dataState objectAtIndex:0] cStringUsingEncoding:NSASCIIStringEncoding];
				
				for (indexTeam = 0; indexTeam < nbElem; indexTeam++)
				{
					if(tmpData[indexTeam] != NULL && !strcmp(tmpData[indexTeam]->URL_depot, URLRepo))
						break;
				}
				
				if(indexTeam == nbElem)
				{
					NSLog(@"Couldn't find the repo to restore, abort :/");
					break;
				}
				
				//We have a valid index, now, let's query the database to get the project
				
				const char * mangaNameCourt = [[dataState objectAtIndex:1] cStringUsingEncoding:NSASCIIStringEncoding];
	
				MANGAS_DATA * project = getDataFromSearch (indexTeam, mangaNameCourt, RDB_CTXLECTEUR);
				
				if(project == NULL)
				{
					NSLog(@"Couldn't find the project to restore, abort :/");
					break;
				}
				else
				{
					checkChapitreValable(project, NULL);
					checkTomeValable(project, NULL);
				}
				
				//Perfect! now, all we have to do is to sanitize last few data :D
				
				int elemToRead;
				bool isTome;
				uint page;
				
				elemToRead = [[dataState objectAtIndex:2] intValue];
				isTome = [[dataState objectAtIndex:3] boolValue];
				page = [[dataState objectAtIndex:4] intValue];
				
				[self startReading: *project: elemToRead: isTome : page];
				
				free(project);
				
				if(mainImage == nil)
					break;
				
				NSPoint sliderPos;
				sliderPos.x = [[dataState objectAtIndex:5] intValue];
				sliderPos.y = [[dataState objectAtIndex:6] intValue];
				[mainImage setSliderPos:sliderPos];
				
				failedInit = true;
			}while (0);
		}
	}
	
	if(!failedInit)
		[self noContent];
	else
	{
		resizeAnimationCount = -1;	//Disable animation
		[self readerIsOpening : REFRESHVIEWS_CHANGE_MT];
		resizeAnimationCount = 0;
	}
}

- (void) endOfInitialization
{
	
}

- (void) noContent
{
	MANGAS_DATA *mangaData = getCopyCache(RDB_LOADALL | SORT_NAME, NULL);
	[self startReading: mangaData[21] : 540: false : 0];
}

- (void) startReading : (MANGAS_DATA) project : (int) elemToRead : (bool) isTome : (uint) startPage
{
	bool shouldNotifyBottomBarInitialized = false;
	
	initialized = true;
	
	if(mainImage == nil)
	{
		mainImage = [[RakPage alloc] init: self: project: elemToRead: isTome : startPage];
		
		if(mainImage == nil)	//Failed at initializing, most probably because of unreadable data
			return;

		shouldNotifyBottomBarInitialized = true;
	}
	else
		[mainImage changeProject : project : elemToRead : isTome : startPage];
	
	if(bottomBar == nil)
		bottomBar = [[RakReaderBottomBar alloc] init: readerMode: self];
	
	if(shouldNotifyBottomBarInitialized)
		[mainImage bottomBarInitialized];
}

- (NSString *) byebye
{
	NSString * output;
	
	if (initialized)
	{
		output = [mainImage getContextToGTFO];
	}
	else
	{
		output = [super byebye];
	}
	
	return output;
}

- (void) dealloc
{
	[bottomBar removeFromSuperview];
	[bottomBar release];
	
	[mainImage removeFromSuperview];
	[mainImage getTheFuckOut];
	[mainImage release];
	
	[self removeFromSuperview];	
	[super dealloc];
}

- (int) getCodePref : (int) request
{
	int output;
	
	switch (request)
	{
		case CONVERT_CODE_POSX:
		{
			output = PREFS_GET_TAB_READER_POSX;
			break;
		}
			
		case CONVERT_CODE_POSY:
		{
			output = PREFS_GET_TAB_READER_POSY;
			break;
		}
			
		case CONVERT_CODE_HEIGHT:
		{
			output = PREFS_GET_TAB_READER_HEIGHT;
			break;
		}
			
		case CONVERT_CODE_WIDTH:
		{
			output = PREFS_GET_TAB_READER_WIDTH;
			break;
		}
			
		case CONVERT_CODE_FRAME:
		{
			output = PREFS_GET_TAB_READER_FRAME;
			break;
		}
			
		default:
			output = 0;
	}
	
	return output;
}

- (void) refreshViewSize
{
	if(gonnaReduceTabs)
	{
		int mainTab;
		bool isReaderMode;
		[Prefs getPref:PREFS_GET_IS_READER_MT :&isReaderMode];
		[Prefs getPref:PREFS_GET_READER_TABS_STATE :&mainTab];
		if(!isReaderMode || (mainTab & STATE_READER_TAB_DEFAULT) == 0)
			gonnaReduceTabs = 0;
	}
	
	[super refreshViewSize];
}

- (void) setFrame:(NSRect)frameRect
{
	[super setFrame:frameRect];
	[mainImage setFrame:frameRect];
	[bottomBar setFrame:frameRect];
}

- (void) readerIsOpening : (byte) context
{
	if(context == REFRESHVIEWS_CHANGE_MT)
	{
		uint copy;
		do
		{
			gonnaReduceTabs = copy = getRandom();
		}while (!copy);
		
#ifdef NO_RETRACTION_WIP
		copy--;
#endif
		
		dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
			if(gonnaReduceTabs == copy)
			{
				[self collapseAllTabs : false];
			}
		});
	}
}

- (void) willLeaveReader
{
	if(mainImage != nil)
		[mainImage leaveReaderMode];
	
	if(bottomBar != nil)
		[bottomBar leaveReaderMode];
	
	[self setAutoresizesSubviews:NO];
}

- (void) willOpenReader
{
	if(mainImage != nil)
		[mainImage startReaderMode];
	
	if(bottomBar != nil)
		[bottomBar startReaderMode];
	
	[self setAutoresizesSubviews:YES];
}

- (void) setUpViewForAnimation : (BOOL) newReaderMode
{
	if(newReaderMode && !readerMode)
		[self willOpenReader];
	else if(!newReaderMode && readerMode)
		[self willLeaveReader];
}

/**	Drawing	**/

- (NSColor*) getMainColor
{
	return [Prefs getSystemColor:GET_COLOR_BACKGROUND_READER_INTAB];
}

/**	Events **/
- (void) keyDown:(NSEvent *)theEvent
{
	[mainImage keyDown:theEvent];
}

/**	NSTrackingArea	**/

- (BOOL) isStillCollapsedReaderTab
{
	int state;
	[Prefs getPref:PREFS_GET_READER_TABS_STATE :&state];
	return ((state == STATE_READER_TAB_ALL_COLLAPSED) || (state == STATE_READER_TAB_DISTRACTION_FREE)) == 0;
}

- (void) resizeReaderCatchArea
{
	if([self isStillCollapsedReaderTab])
		[super resizeReaderCatchArea : readerMode];
}

- (void)mouseExited:(NSEvent *)theEvent
{
	
}

/**	Hide stuffs	**/

- (void) collapseAllTabs : (bool) forced
{
	if(forced || [self isCursorOnMe] || [self mouseOutOfWindow])
	{
		[Prefs setPref:PREFS_SET_READER_TABS_STATE: STATE_READER_TAB_ALL_COLLAPSED];
	}
	[super refreshLevelViews:[self superview] : REFRESHVIEWS_CHANGE_READER_TAB];	//Initialisera les surfaces de tracking
}

- (void) hideBothTab
{
	NSArray *subView = [[self superview] subviews];
	NSView *subViewView;
	NSUInteger i, count = [subView count];
	
	for(i = 0; i < count; i++)
	{
		subViewView = [subView objectAtIndex:i];
		if(subViewView != self)
			[subViewView setHidden:YES];
	}
	[Prefs setPref:PREFS_SET_READER_TABS_STATE :STATE_READER_TAB_DISTRACTION_FREE];
	[self refreshLevelViews:[self superview] : REFRESHVIEWS_CHANGE_READER_TAB];
}

- (void) unhideBothTab
{
	NSArray *subView = [[self superview] subviews];
	NSView *subViewView;
	NSUInteger i, count = [subView count];
	
	for(i = 0; i < count; i++)
	{
		subViewView = [subView objectAtIndex:i];
		if([subViewView isHidden])
			[subViewView setHidden:NO];
	}
}

- (void) hideCursor
{
	[NSCursor setHiddenUntilMouseMoves:YES];
}

#pragma mark - Proxy work

- (void) updateContextNotification:(MANGAS_DATA)project :(BOOL)isTome :(int)element
{
	if(element != VALEUR_FIN_STRUCTURE_CHAPITRE)
	{
		[self mouseDown:NULL];
		[self startReading : project : element : isTome : -1];
	}
}

- (void) prevPage
{
	[mainImage prevPage];
}

- (void) nextPage
{
	[mainImage nextPage];
}

- (void) prevChapter
{
	[mainImage prevChapter];
}

- (void) nextChapter
{
	[mainImage nextChapter];
}

- (void) triggerFullscreen
{
	[self.window toggleFullScreen:self];
}

- (void) deleteElement
{
	[mainImage deleteElement];
}

- (void) updatePage : (uint) newCurrentPage : (uint) newPageMax
{
	[bottomBar updatePage:newCurrentPage :newPageMax];
}

@end
