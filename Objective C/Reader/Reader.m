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

@implementation Reader

#pragma mark - Main view management

- (instancetype) init : (NSView*)contentView : (NSString *) state
{
	self = [super init];
	if (self != nil)
	{
		flag = TAB_READER;
		gonnaReduceTabs = 0;
		
		[Prefs getCurrentTheme:self];		//register for changes
		[RakDBUpdate registerForUpdate:self :@selector(DBUpdated:)];
		
		[self initView : contentView : state];
		self.layer.cornerRadius = 0;
		
		pthread_mutex_init(&cacheMutex, NULL);
		
		container = [[NSView alloc] initWithFrame:self.bounds];
		[self addSubview:container];
		
		loadingPlaceholder = [NSImage imageNamed:@"loading.gif"];
		if(loadingPlaceholder != nil)
		{
			NSBitmapImageRep *gifRep = [[loadingPlaceholder representations] objectAtIndex:0];
			[gifRep setProperty:NSImageLoopCount withValue:@(0)];
			[gifRep setProperty:NSImageCurrentFrameDuration withValue:@(0.1f)];
		}
		loadingFailedPlaceholder = [NSImage imageNamed:@"failed_loading"];
		
		[self initReaderMainView : state];
	}
	return self;
}

- (void) initReaderMainView : (NSString *) state
{
	initialized = NO;
	self.initWithNoContent = YES;
	
	if(state != nil && [state caseInsensitiveCompare:STATE_EMPTY] != NSOrderedSame)
	{
		NSArray *componentsWithSpaces = [state componentsSeparatedByCharactersInSet:[NSCharacterSet newlineCharacterSet]];
		NSArray *dataState = [componentsWithSpaces filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"length > 0"]];
		
		if([dataState count] == 7)
		{
			do
			{
				uint64_t repoID = getRepoIndexFromURL((char*)[[dataState objectAtIndex:0] cStringUsingEncoding:NSASCIIStringEncoding]);
				if(repoID == UINT64_MAX)
				{
					NSLog(@"Couldn't find the repo to restore, abort :/");
					break;
				}
				
				//We have a valid index, now, let's query the database to get the project
				
				const uint projectID = [[dataState objectAtIndex:1] longLongValue];
				
				PROJECT_DATA * project = getDataFromSearch(repoID, projectID, true);
				
				if(project == NULL)
				{
					NSLog(@"Couldn't find the project to restore, abort :/");
					break;
				}
				
				//Perfect! now, all we have to do is to sanitize last few data :D
				
				int elemToRead;
				BOOL isTome;
				uint page;
				
				elemToRead = [[dataState objectAtIndex:2] intValue];
				isTome = [[dataState objectAtIndex:3] boolValue];
				page = [[dataState objectAtIndex:4] intValue];
				
				if(((RakAppDelegate*)[NSApp delegate]).CT.initWithNoContent)
				{
					[((RakAppDelegate*)[NSApp delegate]).CT updateProject :*project :isTome :elemToRead];
				}
				
				[self startReading: *project: elemToRead: isTome : page];
				
				free(project);
				
				[self setSliderPos:NSMakePoint([[dataState objectAtIndex:5] intValue], [[dataState objectAtIndex:6] intValue])];
				
			}while (0);
		}
	}
	
	[self readerIsOpening : REFRESHVIEWS_CHANGE_MT];
}

- (void) startReading : (PROJECT_DATA) project : (int) elemToRead : (BOOL) isTome : (uint) startPage
{
	BOOL shouldNotifyBottomBarInitialized = NO;
	
	initialized = YES;
	
	if(self.initWithNoContent)
	{
		self.initWithNoContent = NO;
		
		if(![self initPage: project: elemToRead: isTome : startPage])	//Failed at initializing, most probably because of unreadable data
		{
			self.initWithNoContent = YES;
			return;
		}
		
		shouldNotifyBottomBarInitialized = YES;
	}
	else
		[self changeProject : project : elemToRead : isTome : startPage];
	
	if(bottomBar == nil)
	{
		bottomBar = [[RakReaderBottomBar alloc] init: self.mainThread == TAB_READER: self];
		
		if(foregroundView.superview == self)
			[self addSubview:bottomBar positioned:NSWindowBelow relativeTo:foregroundView];
		else
			[self addSubview:bottomBar];
	}
	
	[bottomBar favsUpdated:project.favoris];
	
	if(shouldNotifyBottomBarInitialized)
		[self updatePage:_data.pageCourante : _data.nombrePage];
}

- (void) resetReader
{
	if(!self.initWithNoContent)
	{
		self.initWithNoContent = YES;
		
		[self flushCache];
		releaseDataReader(&_data);
		releaseCTData(_project);
		_project.isInitialized = NO;
		
		[self updatePage:0 :0];
	}
}

- (NSString *) byebye
{
	NSString * output;
	
	if (initialized)
		output = [self getContextToGTFO];
	else
		output = [super byebye];
	
	return output;
}

- (void) dealloc
{
	[RakDBUpdate unRegister : self];
	
	[bottomBar removeFromSuperview];
	
	[self deallocInternal];
	
	[container removeFromSuperview];
}

- (uint) getFrameCode
{
	return PREFS_GET_TAB_READER_FRAME;
}

- (void) refreshViewSize
{
	if(gonnaReduceTabs)
	{
		uint mainTab;
		BOOL isReaderMode;
		[Prefs getPref:PREFS_GET_IS_READER_MT :&isReaderMode];
		[Prefs getPref:PREFS_GET_READER_TABS_STATE :&mainTab];
		if(!isReaderMode || (mainTab & STATE_READER_TAB_DEFAULT) == 0)
			gonnaReduceTabs = 0;
	}
	
	[super refreshViewSize];
}

- (void) resize : (NSRect) frame : (BOOL) animated
{
	[self setFrameInternal: frame : animated];
	
	if(animated)
		[bottomBar resizeAnimation:frame];
	else
		[bottomBar setFrame:frame];
}

- (void) readerIsOpening : (byte) context
{
	if(context == REFRESHVIEWS_CHANGE_MT && self.mainThread == TAB_READER)
	{
		uint copy;
		do
		{
			gonnaReduceTabs = copy = getRandom();
		} while (!copy);
		
#ifdef NO_RETRACTION_WIP
		copy--;
#endif
		
		dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 2 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
			if(gonnaReduceTabs == copy && self.mainThread == TAB_READER)
			{
				[self collapseAllTabs : NO];
			}
		});
	}
}

- (void) willLeaveReader
{
	if(bottomBar != nil)
		bottomBar.readerMode = NO;
}

- (void) willOpenReader
{
	if(bottomBar != nil)
		bottomBar.readerMode = YES;
	
	if(_posElemInStructure != -1)
		[self updateTitleBar :_project :_isTome :_posElemInStructure];
}

- (void) setUpViewForAnimation : (uint) mainThread
{
	uint prevMainThread = self.mainThread;
	self.mainThread = mainThread;
	
	if(mainThread == TAB_READER && prevMainThread != TAB_READER)
		[self willOpenReader];
	else if(mainThread != TAB_READER && prevMainThread == TAB_READER)
		[self willLeaveReader];
	
	[super setUpViewForAnimation:mainThread];
}

- (NSColor*) getMainColor
{
	return [Prefs getSystemColor:GET_COLOR_BACKGROUND_READER_INTAB : nil];
}

- (BOOL) isStillCollapsedReaderTab
{
	uint state;
	[Prefs getPref:PREFS_GET_READER_TABS_STATE :&state];
	return ((state == STATE_READER_TAB_ALL_COLLAPSED) || (state == STATE_READER_TAB_DISTRACTION_FREE)) == 0;
}

- (void)mouseExited:(NSEvent *)theEvent
{
	[self abortFadeTimer];
}

- (void) collapseAllTabs : (BOOL) forced
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

#pragma mark - Distraction Free mode

- (void) switchDistractionFree
{
	bottomBarHidden = NO;	//We reset it
	
	//We have to leave distraction-free mode
	if(self.distractionFree && ![Prefs setPref : PREFS_SET_READER_DISTRACTION_FREE : 1])
	{
		self.distractionFree = NO;
		if([Prefs setPref : PREFS_SET_READER_DISTRACTION_FREE : 0])
			[self fadeBottomBar : 1];
		
		else
			return;
	}
	else if(self.distractionFree)	//We were out of sync, but now, we're in DF mode
	{
		[self refreshLevelViews : [self superview] : REFRESHVIEWS_CHANGE_READER_TAB];
		[self startFadeTimer:[NSEvent mouseLocation]];
		[bottomBar setAlphaValue:1];
	}
	
	//We have to get into the DF mode
	else
	{
		self.distractionFree = YES;
		if([Prefs setPref : PREFS_SET_READER_DISTRACTION_FREE : 1])
		{
			[self fadeBottomBar : READER_BB_ALPHA_DF];
			[self startFadeTimer:[NSEvent mouseLocation]];
		}
		
		else
			return;
	}
	
	//Do we have to switch to fullscreen, or can we animate
	if(self.distractionFree && !((RakWindow*) self.window).fullscreen)
		[self.window toggleFullScreen:self];
	
	else
		[self refreshLevelViews : [self superview] : REFRESHVIEWS_CHANGE_READER_TAB];
}

- (void) shouldLeaveDistractionFreeMode
{
	if(self.distractionFree)
	{
		self.distractionFree = NO;
		[Prefs setPref : PREFS_SET_READER_DISTRACTION_FREE : 0];
		[self fadeBottomBar:1];
	}
}

//The following code fade out the bottom bar if the cursor stay static for more than two seconds
//We fire a timer each time the cursor move (and abort the previous one) for two seconds
//If the timer end, we fade the bottom bar to 0.1, and hide the cursor

- (void) mouseMoved:(NSEvent *)theEvent
{
	if(self.distractionFree && !bottomBar.highjackedMouseEvents)
		[self startFadeTimer:[theEvent locationInWindow]];
	
	[super mouseMoved:theEvent];
}

- (void) startFadeTimer : (NSPoint) cursorPosition
{
	[self abortFadeTimer];
	
	cursorPosBeforeLastMove = cursorPosition;
	delaySinceLastMove = [NSTimer scheduledTimerWithTimeInterval:READER_DELAY_CURSOR_FADE target:self selector:@selector(cursorShouldFadeAway) userInfo:nil repeats:NO];
	
	if(bottomBarHidden)
	{
		bottomBarHidden = NO;
		[self fadeBottomBar : READER_BB_ALPHA_DF];
	}
}

- (void) abortFadeTimer
{
	if(delaySinceLastMove != nil)
	{
		[delaySinceLastMove invalidate];
		delaySinceLastMove = nil;
	}
}

- (void) cursorShouldFadeAway
{
	delaySinceLastMove = nil;
	
	NSPoint point = [NSEvent mouseLocation];
	
	if(!bottomBarHidden)
	{
		bottomBarHidden = YES;
		[self fadeBottomBar: READER_BB_ALPHA_DF_STATIC];
	}
	
	if(cursorPosBeforeLastMove.x == point.x && cursorPosBeforeLastMove.y == point.y)
	{
		[NSCursor setHiddenUntilMouseMoves:YES];
	}
}

- (void) fadeBottomBar : (CGFloat) alpha
{
	if(bottomBar == nil)
		return;
	
	[NSAnimationContext beginGrouping];
	[[NSAnimationContext currentContext] setDuration:0.1f];
	
	if(alpha == 0)
	{
		[[NSAnimationContext currentContext] setCompletionHandler : ^{
			[bottomBar setHidden:YES];
		}];
	}
	
	else
	{
		[bottomBar setAlphaValue:0];
		[bottomBar setHidden:NO];
	}
	
	[bottomBar.animator setAlphaValue:alpha];
	
	[NSAnimationContext endGrouping];
}

#pragma mark - Proxy work

- (PROJECT_DATA) activeProject
{
	return _project;
}

- (int) currentElem
{
	return _currentElem;
}

- (void) updateContextNotification:(PROJECT_DATA)project :(BOOL)isTome :(int)element
{
	if(element != VALEUR_FIN_STRUCT)
	{
		[self startReading : project : element : isTome : -1];
		[self ownFocus];
	}
}

- (void) switchFavs
{
	setFavorite(&_project);
	[bottomBar favsUpdated:_project.favoris];
}

- (void) triggerFullscreen
{
	[self.window toggleFullScreen:self];
}

- (void) updatePage : (uint) newCurrentPage : (uint) newPageMax
{
	[bottomBar updatePage:newCurrentPage :newPageMax];
}

- (void) updateTitleBar : (PROJECT_DATA) project : (BOOL) isTome : (uint) position
{
	if(self.mainThread == TAB_READER && project.isInitialized)
	{
		NSString * string;
		
		if(isTome)
		{
			if(project.tomesInstalled == NULL)
				return;
			
			META_TOME tome = project.tomesInstalled[position];
			
			if(tome.readingName[0])
				string = getStringForWchar(tome.readingName);
			else
				string = [NSString stringWithFormat:NSLocalizedString(@"VOLUME-%d", nil), tome.readingID];
		}
		else
		{
			if(project.chapitresInstalled == NULL)
				return;
			
			int element = project.chapitresInstalled[position];
			
			if(element % 10)
				string = [NSString stringWithFormat:NSLocalizedString(@"CHAPTER-%d.%d", nil), element / 10, element % 10];
			else
				string = [NSString stringWithFormat:NSLocalizedString(@"CHAPTER-%d", nil), element / 10];
		}
		
		[((RakAppDelegate *)[NSApp delegate]).window setCTTitle:project :string];
	}
}

#pragma mark - Waiting login

- (NSString*) waitingLoginMessage
{
	return [NSString stringWithFormat:NSLocalizedString(_isTome ? @"AUTH-REQUIRED-READER-VOL-OF-%@" : @"AUTH-REQUIRED-READER-CHAP-OF-%@", nil), getStringForWchar(_project.projectName)];
}

#pragma mark - Drop support

- (BOOL) receiveDrop : (PROJECT_DATA) data : (BOOL) isTome : (int) element : (uint) sender
{
	if(element != VALEUR_FIN_STRUCT && (sender != TAB_MDL || (isTome ? checkTomeReadable(data, element) : checkChapterReadable(data, element))))
	{
		[self updateContextNotification:data :isTome :element];
		return YES;
	}
	return NO;
}

- (NSDragOperation) dropOperationForSender : (uint) sender : (BOOL) canDL
{
	if (sender == TAB_CT || sender == TAB_MDL)
		return canDL ? NSDragOperationNone : NSDragOperationCopy;
	
	return [super dropOperationForSender:sender:canDL];
}

@end
