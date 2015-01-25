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

@implementation Reader (PageManagement)

- (BOOL) initPage : (PROJECT_DATA) dataRequest : (int) elemRequest : (BOOL) isTomeRequest : (int) startPage
{
	_alreadyRefreshed = false;
	_dontGiveACrapAboutCTPosUpdate = false;
	
	if(![self initialLoading:dataRequest :elemRequest :isTomeRequest : startPage])
		return NO;
	
	mainScroller = [[NSPageController alloc] init];
	mainScroller.view = container;
	mainScroller.transitionStyle = dataRequest.japaneseOrder ? NSPageControllerTransitionStyleStackHistory : NSPageControllerTransitionStyleStackBook;
	mainScroller.delegate = self;

	[self updateEvnt];
	
	return YES;
}

- (NSString *) getContextToGTFO
{
	NSPoint sliders = NSZeroPoint;

	if(_scrollView != nil)
		sliders = [[_scrollView contentView] bounds].origin;
	
	return [NSString stringWithFormat:@"%s\n%d\n%d\n%d\n%d\n%.0f\n%.0f", _project.repo->URL, _project.projectID, _currentElem, self.isTome ? 1 : 0, _data.pageCourante, sliders.x, sliders.y];
}

/*Handle the position of the whole thing when anything change*/

#pragma mark    -   Position manipulation

- (void) initialPositionning : (RakPageScrollView *) scrollView
{
	NSRect tabFrame = [self lastFrame], scrollViewFrame = scrollView.scrollViewFrame;
	
	scrollView.scrollViewFrame = NSZeroRect;

	//Hauteur
	if (tabFrame.size.height < scrollView.contentFrame.size.height)
	{
		scrollView.pageTooHigh = YES;
		scrollViewFrame.size.height = tabFrame.size.height;
	}
	else
	{
		scrollView.pageTooHigh = NO;
		scrollViewFrame.origin.y = tabFrame.size.height / 2 - scrollView.contentFrame.size.height / 2;
		scrollViewFrame.size.height = scrollView.contentFrame.size.height;
	}
	
	if(self.mainThread != TAB_READER)	//Dans ce contexte, les calculs de largeur n'ont aucune importance
	{
		scrollView.scrollViewFrame = scrollViewFrame;
		return;
	}
	
	//Largeur
	if(tabFrame.size.width < scrollView.contentFrame.size.width + 2 * READER_BORDURE_VERT_PAGE)	//	Page trop large
	{
		scrollView.pageTooLarge = YES;
		scrollViewFrame.size.width = tabFrame.size.width - 2 * READER_BORDURE_VERT_PAGE;
		scrollViewFrame.origin.x = READER_BORDURE_VERT_PAGE;
	}
	else
	{
		scrollView.pageTooLarge = NO;
		scrollViewFrame.origin.x = tabFrame.size.width / 2 - scrollView.contentFrame.size.width / 2;
		scrollViewFrame.size.width = scrollView.contentFrame.size.width;
	}
	
	scrollView.scrollViewFrame = scrollViewFrame;
}

- (void) setFrameInternal : (NSRect) frameRect : (BOOL) isAnimated
{
	if(self.mainThread != TAB_READER)
		frameRect.size.width = container.frame.size.width;
	
	[container setFrame:NSMakeRect(0, 0, frameRect.size.width, frameRect.size.height)];
	
	if(_scrollView != nil)
	{
		if(self.mainThread != TAB_READER)
			frameRect.origin = _scrollView.frame.origin;
		
		[_scrollView.superview setFrame:container.frame];
		
		NSSize oldSize = _scrollView.frame.size;
		
		[self initialPositionning : _scrollView];
		[self updateScrollerAfterResize : _scrollView : oldSize];
		
		if(isAnimated)
			[_scrollView.animator setFrame:_scrollView.scrollViewFrame];
		else
			[_scrollView setFrame:_scrollView.scrollViewFrame];
	}
	else
	{
		RakImageView * view = [mainScroller.selectedViewController.view.subviews objectAtIndex:0];
		if([view class] == [RakImageView class])
		{
			NSRect frame = view.frame;		//view is smaller than the smallest possible reader, so its h/w won't change
			
			frame.origin.y = frameRect.size.height / 2 - frame.size.height / 2;

			if(self.mainThread == TAB_READER)
				frame.origin.x = frameRect.size.width / 2 - frame.size.width / 2;
			
			[view.superview setFrame:frame];
		}
	}
}

/*Event handling*/

#pragma mark    -   Events

- (void)mouseUp:(NSEvent *)theEvent
{
	bool fail = false;

	if(self.mainThread != TAB_READER || !noDrag || _scrollView == nil)
		fail = true;
	else
	{
		NSPoint mouseLoc = [self convertPoint:[theEvent locationInWindow] fromView:nil];
		
		if(_scrollView.pageTooHigh)
		{
			mouseLoc.y += [_scrollView.contentView documentRect].size.height - [_scrollView frame].size.height - [_scrollView.contentView documentVisibleRect].origin.y;
			if(mouseLoc.y < READER_PAGE_TOP_BORDER || mouseLoc.y > [(NSView*) _scrollView.documentView frame].size.height - READER_PAGE_BOTTOM_BORDER)
				fail = true;
		}
		
		if(_scrollView.pageTooLarge)
		{
			mouseLoc.x += [_scrollView.contentView documentRect].size.width - [_scrollView frame].size.width - [_scrollView.contentView documentVisibleRect].origin.x;
			if(mouseLoc.x < READER_BORDURE_VERT_PAGE || mouseLoc.x > [(NSView*) _scrollView.documentView frame].size.width - READER_BORDURE_VERT_PAGE)
				fail = true;
		}
	}
	
	if(fail)
		[super mouseUp:theEvent];
	else
		[self nextPage];
}

- (void) keyDown:(NSEvent *)theEvent
{
	NSString*   const   character   =   [theEvent charactersIgnoringModifiers];
    unichar     const   code        =   [character characterAtIndex:0];
	bool isModPressed = ((RakAppDelegate*)[NSApp delegate]).window.shiftPressed;
	
    switch (code)
    {
        case NSUpArrowFunctionKey:
        {
			if(isModPressed)
				[self moveSliderX:-PAGE_MOVE];
            else
				[self moveSliderY:PAGE_MOVE];
            break;
        }
        case NSDownArrowFunctionKey:
        {
			if(isModPressed)
				[self moveSliderX:PAGE_MOVE];
            else
				[self moveSliderY:-PAGE_MOVE];
            break;
        }
        case NSLeftArrowFunctionKey:
        {
            [self prevPage];
            break;
        }
        case NSRightArrowFunctionKey:
        {
            [self nextPage];
            break;
		}
			
		case NSPageUpFunctionKey:
		{
			[self scrollToExtreme : _scrollView : YES];
			break;
		}
			
		case NSPageDownFunctionKey:
		{
			[self scrollToExtreme : _scrollView : NO];
			break;
		}
			
		default:
		{
			const char * string = [character cStringUsingEncoding:NSASCIIStringEncoding];
			char c;
			
			if(character == nil || string == nil)
				break;
			
			if(string[0] >= 'A' && string[0] <= 'Z')
				c = string[0] + 'a' - 'A';
			else
				c = string[0];

			switch (c)
			{
				case 'a':
				{
					[self prevPage];
					break;
				}
					
				case 'd':
				{
					if(isModPressed && ((RakAppDelegate*)[NSApp delegate]).window.commandPressed)
					{
						[self switchDistractionFree];
					}
					else
					{
						[self nextPage];
					}
					break;
				}
					
				case 'q':
				{
					[self prevChapter];
					break;
				}
					
				case 'e':
				{
					[self nextChapter];
					break;
				}
					
				case 'w':
				{
					[self moveSliderY:PAGE_MOVE];
					break;
				}
					
				case 's':
				{
					[self moveSliderY:-PAGE_MOVE];
					break;
				}
					
				case ' ':
				{
					[self jumpPressed : isModPressed];
					break;
				}
					
				default:
					break;
			}
		}
    }
}

/*Error management*/

#pragma mark    -   Errors

- (void) failure
{
	[self failure : UINT32_MAX : nil];
}

- (void) failure : (uint) page : (NSMutableArray**) data
{
	NSLog(@"Something went wrong delete?");
	
	if(page != UINT32_MAX)
	{
		NSRect frame = NSZeroRect;
		frame.size = loadingFailedPlaceholder.size;
		
		RakImageView * image = [[RakImageView alloc] initWithFrame : frame];
		[image setImage : loadingFailedPlaceholder];
		
		if(image != nil)
		{
			__autoreleasing NSMutableArray* bak = nil;
			if(data == nil)
				data = &bak;
			
			image.page = page;
			
			[self updatePCState : data : page : image];
		}
	}
	
	cacheSession++;
}

#pragma mark - DB update

- (void) DBUpdated : (NSNotification*) notification
{
	if([RakDBUpdate analyseNeedUpdateProject:notification.userInfo :_project])
	{
		PROJECT_DATA project = getElementByID(_project.cacheDBID);
		if(project.isInitialized)
		{
			releaseCTData(_project);
			_project = project;
			_posElemInStructure = reader_getPosIntoContentIndex(_project, _currentElem, self.isTome);
		}
	}
}

#pragma mark - High level API

- (void) nextPage
{
	[self nextPage:NO];
}

- (void) nextPage : (BOOL) animated
{
	[self changePage:READER_ETAT_NEXTPAGE:animated];
}

- (void) prevPage
{
	[self prevPage:NO];
}

- (void) prevPage : (BOOL) animated
{
	[self changePage:READER_ETAT_PREVPAGE : animated];
}

- (void) nextChapter
{
	[self changeChapter : YES : NO];
}

- (void) prevChapter
{
	[self changeChapter : NO : NO];
}

//Did the scroll succeed, or were we alredy at the bottom
- (BOOL) moveSliderX : (int) move
{
	return [self _moveSliderX:move : NO : NO];
}

- (BOOL) _moveSliderX : (int) move : (BOOL) animated : (BOOL) contextExist
{
	if(_scrollView == nil || !_scrollView.pageTooLarge)
		return NO;
	
	NSPoint point = [[_scrollView contentView] bounds].origin;
	
	if(move < 0 && point.x == 0)
		return NO;
	else if(move < 0 && point.x < -move)
		point.x = 0;
	
	else if(move > 0)
	{
		CGFloat basePos = [_scrollView.documentView frame].size.width - _scrollView.frame.size.width;
		if(point.x == basePos)
			return NO;
		else if(point.x > basePos - move)
			point.x = basePos;
		else
			point.x += move;
	}
	else
		point.x += move;
	
	if(animated)
	{
		if(!contextExist)
		{
			[NSAnimationContext beginGrouping];
			[[NSAnimationContext currentContext] setDuration:0.3];
		}
		
		[_scrollView.contentView.animator setBoundsOrigin:point];
		
		if(!contextExist)
			[NSAnimationContext endGrouping];
	}
	else
		[_scrollView scrollToPoint:point];
	return YES;
}

- (BOOL) moveSliderY : (int) move
{
	return [self _moveSliderY:move :NO :NO];
}

- (BOOL) _moveSliderY : (int) move : (BOOL) animated : (BOOL) contextExist
{
	if(_scrollView == nil || !_scrollView.pageTooHigh)
		return NO;
	
	NSPoint point = _scrollView.contentView.bounds.origin;
	
	if(move < 0 && point.y == 0)
		return NO;
	else if(move < 0 && point.y < -move)
		point.y = 0;
	
	else if(move > 0)
	{
		CGFloat basePos = round([_scrollView.documentView frame].size.height - _scrollView.bounds.size.height);
		if(point.y == basePos)
			return NO;
		else if(point.y > basePos - move)
			point.y = basePos;
		else
			point.y += move;
	}
	else
		point.y += move;
	
	if(animated)
	{
		if(!contextExist)
		{
			[NSAnimationContext beginGrouping];
			[[NSAnimationContext currentContext] setDuration:0.2];
		}
		
		[_scrollView.contentView.animator setBoundsOrigin:point];
		
		if(!contextExist)
			[NSAnimationContext endGrouping];
	}
	else
		[_scrollView scrollToPoint:point];
	return YES;
}

- (void) scrollToExtreme : (RakPageScrollView *) scrollview : (BOOL) toTheTop
{
	if(scrollview != nil && [scrollview class] == [RakPageScrollView class])
	{
		if(toTheTop)
			[scrollview scrollToBeginningOfDocument];
		else
			[scrollview scrollToEndOfDocument];
	}
}

- (void) setSliderPos : (NSPoint) newPos
{
	if (_scrollView != nil)
	{
		NSPoint point = _scrollView.contentView.bounds.origin;
		
		[self moveSliderX : newPos.x - point.x];
		[self moveSliderY : newPos.y - point.y];
	}
}

/*Active routines*/

#pragma mark    -   Active routines

- (BOOL) initialLoading : (PROJECT_DATA) dataRequest : (int) elemRequest : (BOOL) isTomeRequest : (int) startPage
{
	_project = getCopyOfProjectData(dataRequest);
	_currentElem = elemRequest;
	self.isTome = isTomeRequest;
	
	_cacheBeingBuilt = false;
	
	_posElemInStructure = reader_getPosIntoContentIndex(_project, _currentElem, self.isTome);
	if(_posElemInStructure == -1)
	{
		[self failure];
		return NO;
	}
	
	if(_project.isPaid && !preventWindowCaptureForWindow(self.window))
	{
		[self failure];
		return NO;
	}
	
	[self updateTitleBar:_project :isTomeRequest :_posElemInStructure];

	setLastChapitreLu(_project, self.isTome, _currentElem);
	if(reader_isLastElem(_project, self.isTome, _currentElem))
	{
		[self performSelectorInBackground:@selector(checkIfNewElements) withObject:nil];
	}
	
	if(configFileLoader(_project, self.isTome, _currentElem, &_data))
	{
		[self failure];
		return NO;
	}
	
	if(startPage < 0)
		_data.pageCourante = 0;
	else if(startPage < _data.nombrePageTotale)
		_data.pageCourante = startPage;
	else
		_data.pageCourante = _data.nombrePageTotale;
	
	return YES;
}

- (NSData *) getPage : (uint) posData : (DATA_LECTURE*) data
{
	if(_data.path == NULL)
		return nil;
	
	IMG_DATA * dataPage = loadSecurePage(data->path[data->pathNumber[posData]], data->nomPages[posData], data->chapitreTomeCPT[data->pathNumber[posData]], data->pageCouranteDuChapitre[posData]);
	
	if(dataPage == IMGLOAD_INCORRECT_DECRYPTION)
		return nil;

	else if(dataPage == IMGLOAD_NEED_CREDENTIALS_MAIL || dataPage == IMGLOAD_NEED_CREDENTIALS_PASS)
	{
		if(dataPage == IMGLOAD_NEED_CREDENTIALS_PASS)
			_needPassword = true;
		
		MUTEX_VAR * lock = [(RakAppDelegate*) [NSApp delegate]sharedLoginMutex : YES];
		
		[self performSelectorOnMainThread:@selector(setWaitingLoginWrapper:) withObject:@(true) waitUntilDone:NO];
		
		while(COMPTE_PRINCIPAL_MAIL == NULL || (_needPassword && !getPassFromCache(NULL)))
		{
			pthread_cond_wait([(RakAppDelegate*) [NSApp delegate]sharedLoginLock], lock);
		}
		
		pthread_mutex_unlock(lock);

		[self performSelectorOnMainThread:@selector(setWaitingLoginWrapper:) withObject:@(false) waitUntilDone:NO];
		
		return [self getPage : posData : data];
	}
	else if(dataPage == IMGLOAD_NODATA)
		return nil;
	
	NSData *output = [NSData dataWithBytes:dataPage->data length:dataPage->length];
		
	free(dataPage->data);
	free(dataPage);
	
	return output;
}

- (void) changePage : (byte) switchType
{
	[self changePage:switchType :NO];
}

- (void) changePage : (byte) switchType : (BOOL) animated
{
	if(switchType == READER_ETAT_NEXTPAGE)
	{
		if(_data.pageCourante + 1 > _data.nombrePageTotale)
		{
			[self changeChapter : YES : YES];
			return;
		}
		_data.pageCourante++;
	}
	else if(switchType == READER_ETAT_PREVPAGE)
	{
		if(_data.pageCourante < 1)
		{
			[self changeChapter : NO : YES];
			return;
		}
		_data.pageCourante--;
	}
	
	if(switchType != READER_ETAT_DEFAULT && mainScroller.selectedIndex != _data.pageCourante + 1)
	{
		MUTEX_LOCK(cacheMutex);
		
		self.preventRecursion = YES;
		
		if(animated)
		{
			if(switchType == READER_ETAT_NEXTPAGE)
				[mainScroller navigateForward:self];
			else
				[mainScroller navigateBack:self];
		}
		else
		{
			mainScroller.selectedIndex = _data.pageCourante + 1;
		}
		self.preventRecursion = NO;
		
		MUTEX_UNLOCK(cacheMutex);
	}
	
	previousMove = switchType;
	
	[self updatePage:_data.pageCourante : _data.nombrePageTotale];	//And we update the bar
	
	if(switchType == READER_ETAT_DEFAULT)
		[self updateEvnt];
	else
	{
		if([mainScroller.arrangedObjects[_data.pageCourante + 1] class] == [RakPageScrollView class])
			_scrollView = mainScroller.arrangedObjects[_data.pageCourante + 1];
		else
			_scrollView = nil;
		
		[self optimizeCache : nil];
	}
}

- (void) jumpToPage : (uint) newPage
{
	if (newPage == _data.pageCourante || newPage > _data.nombrePageTotale)
		return;
	
	int pageCourante = _data.pageCourante;
	
	if(newPage == pageCourante - 1)
		[self changePage:READER_ETAT_PREVPAGE];
	else if(newPage == pageCourante + 1)
		[self changePage:READER_ETAT_NEXTPAGE];
	else
	{
		_data.pageCourante = newPage;
		[self changePage:READER_ETAT_JUMP];
	}
}

- (void) changeChapter : (BOOL) goToNext : (BOOL) byChangingPage
{
	uint newPosIntoStruct = _posElemInStructure;
	
	if(changeChapter(&_project, self.isTome, &_currentElem, &newPosIntoStruct, goToNext))
	{
		cacheSession++;
		_posElemInStructure = newPosIntoStruct;
		
		[self updateTitleBar:_project :self.isTome :_posElemInStructure];
		
		[self updateCTTab];
		
		if((goToNext && nextDataLoaded) || (!goToNext && previousDataLoaded))
		{
			uint currentPage;
			
			if(goToNext)
			{
				releaseDataReader(&_previousData);
				currentPage = _data.nombrePageTotale + 2;

				memcpy(&_previousData, &_data, sizeof(DATA_LECTURE));
				previousDataLoaded = dataLoaded;
				
				memcpy(&_data, &_nextData, sizeof(DATA_LECTURE));
				_data.pageCourante = 0;
				dataLoaded = nextDataLoaded;

				nextDataLoaded = NO;
				
			}
			else
			{
				releaseDataReader(&_nextData);
				currentPage = 0;

				memcpy(&_nextData, &_data, sizeof(DATA_LECTURE));
				nextDataLoaded = dataLoaded;
				
				memcpy(&_data, &_previousData, sizeof(DATA_LECTURE));
				_data.pageCourante = _data.nombrePageTotale;
				dataLoaded = previousDataLoaded;
				
				previousDataLoaded = NO;
			}
			
			if(!byChangingPage)
				_data.pageCourante = 0;
			
			id currentPageView = mainScroller.arrangedObjects[currentPage];
			
			[self updateContext : YES];
			
			//We inject the page we already loaded inside mainScroller
			NSMutableArray * array = [mainScroller.arrangedObjects mutableCopy];
			
			[array replaceObjectAtIndex:1 withObject:currentPageView];
			
			MUTEX_LOCK(cacheMutex);
			mainScroller.arrangedObjects = array;
			MUTEX_UNLOCK(cacheMutex);
		}
		else
		{
			if(!byChangingPage)
				_data.pageCourante = 0;

			[self updateContext : NO];
		}
	}
}

- (void) changeProject : (PROJECT_DATA) projectRequest : (int) elemRequest : (bool) isTomeRequest : (int) startPage
{
	if(_dontGiveACrapAboutCTPosUpdate)
		return;
	
	if(projectRequest.cacheDBID != _project.cacheDBID)
		_alreadyRefreshed = false;
	else if(elemRequest == _currentElem && isTomeRequest == self.isTome)
	{
		[self jumpToPage:startPage];
		return;
	}
	
	[self flushCache];
	releaseDataReader(&_data);
	
	if([self initialLoading:projectRequest :elemRequest :isTomeRequest : startPage])
	{
		[self updateCTTab];
		[self changePage:READER_ETAT_DEFAULT];
	}
	
	addRecentEntry(_project, false);
}

- (void) updateCTTab
{
	CTSelec * tabCT = [(RakAppDelegate*) [NSApp delegate]CT];
	
	_dontGiveACrapAboutCTPosUpdate = true;
	[tabCT selectElem: _project.cacheDBID :self.isTome :_currentElem];
	_dontGiveACrapAboutCTPosUpdate = false;
}

- (void) updateContext : (BOOL) dataAlreadyLoaded
{
	[self flushCache];
	
	setLastChapitreLu(_project, self.isTome, _currentElem);
	if(reader_isLastElem(_project, self.isTome, _currentElem))
        [self performSelectorInBackground:@selector(checkIfNewElements) withObject:nil];
	
	if(dataLoaded)
	{
		if(configFileLoader(_project, self.isTome, _currentElem, &_data))
		{
			_data.nombrePageTotale = 1;
			[self failure : 0 : nil];
		}
	}
	else
		_data.pageCourante = 0;
	
	[self changePage:READER_ETAT_DEFAULT];
}

- (void) updateEvnt
{
	//We rebuild the cache from scratch
	NSMutableArray * array = [NSMutableArray arrayWithArray:mainScroller.arrangedObjects];
	
	[array removeAllObjects];
	
	[array addObject:@(-1)];	//Placeholder for last page of previous chapter
	
	for(uint i = 0; i <= _data.nombrePageTotale; i++)
		[array addObject:@(i)];
	
	[array addObject:@(-2)];	//Placeholder for first page of next chapter
	
	_scrollView = nil;
	
	if(mainScroller != nil)
	{
		MUTEX_LOCK(cacheMutex);
	
		[array replaceObjectAtIndex:_data.pageCourante withObject:@(_data.pageCourante)];

		mainScroller.arrangedObjects = array;
		mainScroller.selectedIndex = _data.pageCourante + 1;
		
		MUTEX_UNLOCK(cacheMutex);
	}

	[self performSelectorInBackground:@selector(buildCache:) withObject:@(++cacheSession)];
}

- (RakPageScrollView *) getScrollView : (uint) page : (DATA_LECTURE*) data
{
	NSData * imageData = [self getPage : page : data];
	
	if(imageData == nil)
		return nil;
	
	NSImage * image = [[NSImage alloc] initWithData : imageData];
	
#ifdef DEV_VERSION
	if(image == nil)
		[imageData writeToFile:@"lol.png" atomically:NO];
#endif

	if(image == nil)
		return nil;
	
	[image setCacheMode:NSImageCacheNever];
	
	RakPageScrollView * output = [[RakPageScrollView alloc] init];
	
	[self addPageToView:image :output];
	output.page = page;
	
	return output;
}

- (void) deleteElement
{
	cacheSession++;	//Tell the cache system to stop
	while (_cacheBeingBuilt);
	
	deleteProject(_project, _currentElem, self.isTome);
	[RakDBUpdate postNotificationProjectUpdate:_project];
		
	if(_posElemInStructure != (self.isTome ? _project.nombreTomesInstalled : _project.nombreChapitreInstalled))
		[self nextChapter];
	else if(_posElemInStructure > 0)
		[self prevChapter];
	else
	{
		_data.pageCourante = 0;
		_data.nombrePageTotale = 1;
		[self failure : 0 : nil];
		mainScroller.selectedIndex = 1;
	}
}

- (void) addPageToView : (NSImage *) page : (RakPageScrollView *) scrollView
{
	if(page == nil || scrollView == nil)
		return;
	
	scrollView.contentFrame = NSMakeRect(0, 0, page.size.width, page.size.height + READER_PAGE_BORDERS_HIGH);

	//We create the view that si going to be displayed
	NSImageView * pageView = [[NSImageView alloc] initWithFrame: scrollView.contentFrame];
	[pageView setImageAlignment:NSImageAlignCenter];
	[pageView setImageFrameStyle:NSImageFrameNone];
	[pageView setImage:page];
	
	scrollView.documentView = pageView;
	
	[CATransaction begin];

	[self initialPositionning : scrollView];
	
	[scrollView setFrame : scrollView.scrollViewFrame];
	[scrollView scrollToBeginningOfDocument];

	[CATransaction commit];
}

- (void) updateScrollerAfterResize : (RakPageScrollView *) scrollView : (NSSize) previousSize
{
	NSPoint sliderStart = [[_scrollView contentView] bounds].origin;
		
	if(scrollView.pageTooHigh)
		sliderStart.y += (previousSize.height - scrollView.scrollViewFrame.size.height) / 2;
	
	if(scrollView.pageTooLarge)
		sliderStart.x += (previousSize.width - scrollView.scrollViewFrame.size.width) / 2;
	
	[scrollView scrollToPoint:sliderStart];
}

- (void) jumpPressed : (BOOL) withShift
{
	CGFloat height = self.bounds.size.height, delta = height - READER_PAGE_BOTTOM_BORDER;
	
	if(!withShift)
		delta *= -1;
	
	if(![self _moveSliderY : delta : YES : NO])
	{
		CGFloat width = self.bounds.size.width;
		delta = width - 2 * READER_BORDURE_VERT_PAGE;
		
		if(withShift ^ !_project.japaneseOrder)
			delta *= -1;
		
		
		if(![self _moveSliderX : delta : YES : NO])
		{
			if(withShift)
			{
				//arrangedObjects est offset d'un rang vers la droite (+1), du coup, _data.pageCourante correspond à l'index de la page précédante
				[CATransaction begin];
				[self scrollToExtreme : mainScroller.arrangedObjects[_data.pageCourante] : NO];
				[CATransaction commit];
				
				[self prevPage : YES];
			}
			else
			{
				[self nextPage : YES];
				[self scrollToExtreme : _scrollView : YES];
			}
		}
		else
		{
			height = [_scrollView.documentView frame].size.height;
			if(withShift)	height *= -1;

			[self moveSliderY : height];
		}
	}
}

#pragma mark - Cache generation

- (void) buildCache : (NSNumber *) session
{
	_cacheBeingBuilt = true;
	
	if(_data.pageCourante > _data.nombrePageTotale)	//Données hors de nos bornes
	{
		_cacheBeingBuilt = false;
		return;
	}
	
	MUTEX_LOCK(cacheMutex);
	
	NSMutableArray * data = mainScroller.arrangedObjects.mutableCopy;
	
	MUTEX_UNLOCK(cacheMutex);
	
	@autoreleasepool
	{
		[self _buildCache : [session unsignedIntValue] : data];
		
		[CATransaction begin];
		[CATransaction setDisableActions:YES];
	}
	[CATransaction commit];
	
	_cacheBeingBuilt = false;
}

- (void) _buildCache : (uint) session : (NSMutableArray *) data
{
	while(session == cacheSession)	//While the active chapter is still the same
	{
		//Page courante
		if(![self entryValid : data : _data.pageCourante + 1])
		{
			[self loadPageCache: _data.pageCourante : session : &data];
		}
		
		//Encore de la place dans le cache
		else if([self nbEntryRemaining : data])
		{
			char move = previousMove == READER_ETAT_PREVPAGE ? -1 : 1;	//Next page by default
			uint i, max = _data.nombrePageTotale;
			
			//_data.pageCourante + i * move is unsigned, so it should work just fine
			for(i = 0; i < 5 && _data.pageCourante + i * move <= max; i++)
			{
				if(![self entryValid : data : _data.pageCourante + 1 + i * move])
				{
					[self loadPageCache:_data.pageCourante + i * move :session :&data];
					move = 0;
					break;
				}
			}
			
			if(!move)		//If we found something, we go back to the begining of the loop
				continue;
			
			else if(i != 5)	//We hit the max
			{
				if([self loadAdjacentChapter : move == 1 : &data : session])
					continue;
			}
			
			//We cache the previous page, in the case the user want to go back
			//First, we check if we are in the general case
			if(_data.pageCourante - move <= max)
			{
				if(![self entryValid : data :_data.pageCourante + 1 - move])
				{
					[self loadPageCache:_data.pageCourante - move : session : &data];
					continue;
				}
			}
			else	//We are at the begining/end of the chapter
			{
				if([self loadAdjacentChapter : move == -1 : &data : session])
					continue;
			}
			
			//Ok then, we cache everythin after
			for (i = _data.pageCourante + 1; i <= max; i++)
			{
				if(![self entryValid : data : i + 1])
				{
					[self loadPageCache : i :session :&data];
					break;
				}
			}
			
			if(i == max + 1)	//Nothing else to load
				break;
		}
		else
			break;
	}
}

- (BOOL) loadAdjacentChapter : (BOOL) loadNext : (NSMutableArray **) data : (uint) currentSession
{
	int nextElement;
	uint nextElementPos = _posElemInStructure;
	
	//Check if next CT is readable
	if(changeChapter(&_project, self.isTome, &nextElement, &nextElementPos, loadNext))
	{
		//Load next CT data
		if((loadNext && nextDataLoaded) || (!loadNext && previousDataLoaded) || !configFileLoader(_project, self.isTome, nextElement, (loadNext ? &_nextData : &_previousData)))
		{
			if(loadNext && ![self entryValid : *data : _data.nombrePageTotale + 2])
			{
				nextDataLoaded = YES;
				[self loadPageCache : 0 : &_nextData : currentSession : _data.nombrePageTotale + 2 : data];
				
				return YES;
			}
			else if(!loadNext && ![self entryValid : *data : 0])
			{
				previousDataLoaded = YES;
				[self loadPageCache : _previousData.nombrePageTotale : &_previousData : currentSession : 0 : data];
				
				return YES;
			}
		}
	}

	return NO;
}

#define NB_ELEM_MAX_IN_CACHE 30			//5 behind, current, 24 ahead

- (uint) nbEntryRemaining : (NSArray *) data
{
	uint nbElemCounted = 0, count = MIN([data count], NB_ELEM_MAX_IN_CACHE);
	
	for(id object in data)
	{
		if ([object class] == [RakPageScrollView class])
		{
			nbElemCounted++;

			if(nbElemCounted > NB_ELEM_MAX_IN_CACHE)
				break;
		}
	}
	
	return count - nbElemCounted;
}

- (BOOL) entryValid : (NSArray*) data : (uint) index
{
	Class class = [data[index] class];

	return class == [RakPageScrollView class] || class == [RakImageView class];
}

- (void) optimizeCache : (NSMutableArray *) data
{
	uint curPage = _data.pageCourante + 1, objectPage, validFound = 0, invalidFound = 0;
	
	NSMutableArray * internalData, *freeList = [NSMutableArray array];
	RakPageScrollView* object;
	
	if(data == nil)
		internalData = [NSMutableArray arrayWithArray:mainScroller.arrangedObjects];
	else
		internalData = data;
	
	MUTEX_LOCK(cacheMutex);

	for(uint pos = 0, max = [internalData count]; pos < max; pos++)
	{
		object = [internalData objectAtIndex:pos];
		
		if ([object class] == [RakPageScrollView class])
		{
			objectPage = object.page;
			
			if(objectPage < MAX(curPage, 5) - 5 ||	//Too far behind
			   objectPage > curPage + 24)			//Too far ahead
			{
				[freeList addObject:object];
				[internalData replaceObjectAtIndex:pos withObject:@(pos)];
				invalidFound++;
			}
			else
				validFound++;
		}
	}
	
	if(invalidFound)
	{
		mainScroller.arrangedObjects = internalData;
		
		MUTEX_UNLOCK(cacheMutex);
	}
	else
	{
		MUTEX_UNLOCK(cacheMutex);
	}
	
	if(validFound != NB_ELEM_MAX_IN_CACHE && !_cacheBeingBuilt)
	{
		[self performSelectorInBackground:@selector(buildCache:) withObject:@(++cacheSession)];
	}
}

- (BOOL) loadPageCache : (uint) page : (uint) currentSession : (NSMutableArray **) data
{
	return [self loadPageCache: page : &_data : currentSession : page + 1 : data];
}

- (BOOL) loadPageCache : (uint) page : (DATA_LECTURE*) dataLecture : (uint) currentSession : (uint) position : (NSMutableArray **) data
{
	RakPageScrollView *view = [self getScrollView : page : dataLecture];
	
	if(view == nil)			//Loading failure
	{
		[self failure : position : data];
		return NO;
	}
	
	if(currentSession != cacheSession)	//Didn't changed of chapter since the begining of the loading
		return NO;
	
	[self updatePCState : data : position : view];
	
	if(&_data == dataLecture && page == dataLecture->pageCourante)		//If current page, we update the main scrollview pointer (click management)
		_scrollView = view;
	
	return YES;
}

#pragma mark - NSPageController interface

- (void) updatePCState : (NSMutableArray **) data : (uint) page : (NSView *) view
{
	[CATransaction begin];
	
	MUTEX_LOCK(cacheMutex);

	*data = [NSMutableArray arrayWithArray:mainScroller.arrangedObjects];
	[*data replaceObjectAtIndex:page withObject:view];
	mainScroller.arrangedObjects = *data;
	
	MUTEX_UNLOCK(cacheMutex);
	
	[CATransaction commit];
}

- (NSString *)pageController:(NSPageController *)pageController identifierForObject : (RakPageScrollView*) object
{
	return @"dashie is best pony";
}

- (NSViewController *)pageController:(NSPageController *)pageController viewControllerForIdentifier:(NSString *)identifier
{
	NSViewController * controller = [NSViewController new];
	
	controller.view = [[NSView alloc] initWithFrame : container.frame];
	
	return controller;
}

- (void) pageController : (NSPageController *) pageController prepareViewController : (NSViewController *) viewController withObject : (RakPageScrollView*) object
{
	NSView * view = viewController.view;
	NSArray * subviews = [NSArray arrayWithArray:view.subviews];
	
	for(NSView * sub in subviews)
	{
		[sub removeFromSuperview];
		
		if([sub class] == [RakImageView class])
			[(RakImageView*) sub stopAnimation];
	}
	
	[view setFrame : container.frame];
	
	if(object == nil || ([object class] != [RakPageScrollView class] && [object class] != [RakImageView class]))
	{
		RakImageView * placeholder = [[RakImageView alloc] initWithFrame:NSMakeRect(0, 0, loadingPlaceholder.size.width, loadingPlaceholder.size.height)];
		[placeholder setImage:loadingPlaceholder];

		if([object isKindOfClass:[NSNumber class]])
			placeholder.page = [(NSNumber *) object unsignedIntValue];
		else
			placeholder.page = UINT_MAX;
		
		[viewController.view addSubview : placeholder];
		
		if(object != nil)
			[placeholder startAnimation];
	}
	else if([object class] == [RakPageScrollView class])
	{
		[self initialPositionning : object];
		
		if(object.page != _data.pageCourante)
			[object scrollToBeginningOfDocument];
		
		[object setFrame:object.scrollViewFrame];
		
		[viewController.view addSubview: object];
		viewController.representedObject = object;
	}
	else
	{
		[viewController.view addSubview : object];
	}
}

- (NSRect) pageController : (NSPageController *) pageController frameForObject : (RakPageScrollView*) object
{
	if(object == nil || [object class] != [RakPageScrollView class])
	{
		NSRect frame;
		
		if([object class] == [RakImageView class])
			frame.size = object.frame.size;
		else
			frame.size = loadingPlaceholder.size;
		
		frame.origin.x = container.frame.size.width / 2 - frame.size.width / 2;
		frame.origin.y = container.frame.size.height / 2 - frame.size.height / 2;
		
		return frame;
	}
	
	return container.frame;
}

- (void) pageController : (NSPageController *) pageController didTransitionToObject : (RakPageScrollView *) object
{
	if(object == nil || _flushingCache)
		return;
	
	//We are to an adjacent chapter page
	uint index = pageController.selectedIndex;
	
	if(index == 0)
	{
		[self changeChapter : NO : YES];
	}
	else if(index == _data.nombrePageTotale + 2)
	{
		[self changeChapter : YES : YES];
	}
	else
	{
		uint requestedPage;
		
		if([object superclass] == [NSNumber class])
			requestedPage = [(NSNumber*) object intValue];
		else
			requestedPage = object.page;
		
		if(requestedPage != _data.pageCourante && !self.preventRecursion)
		{
			[self changePage : requestedPage > _data.pageCourante ? READER_ETAT_NEXTPAGE : READER_ETAT_PREVPAGE];
		}
	}
}

- (void)pageControllerDidEndLiveTransition : (NSPageController *) pageController
{
	[pageController completeTransition];
}

#pragma mark - Checks if new elements to download

- (void) checkIfNewElements
{
	if(_alreadyRefreshed)
		return;
	else
		_alreadyRefreshed = true;
	
	uint nbElemToGrab = checkNewElementInRepo(&_project, self.isTome, _currentElem);
	
	if(!nbElemToGrab)
		return;
	
	PROJECT_DATA localProject = getCopyOfProjectData(_project);
	RakArgumentToRefreshAlert * argument = [RakArgumentToRefreshAlert alloc];
	argument.data = &localProject;
	argument.nbElem = nbElemToGrab;
	
	[self performSelectorOnMainThread:@selector(promptToGetNewElems:) withObject:argument waitUntilDone:YES];
	
	releaseCTData(localProject);
}

- (void) promptToGetNewElems : (RakArgumentToRefreshAlert *) arguments
{
	PROJECT_DATA localProject = *arguments.data;
	uint nbElemToGrab = arguments.nbElem, nbElemValidated = 0;
	
	if(_project.cacheDBID != localProject.cacheDBID)	//The active project changed meanwhile
		return;
	
	//We're going to evaluate in which case we are (>= 2 elements, 1, none)
	int * selection = calloc(nbElemToGrab, sizeof(int));
	MDL * tabMDL = [(RakAppDelegate*) [NSApp delegate]MDL];
	
	if(selection == NULL || tabMDL == nil)
	{
		free(selection);
		return;
	}
	
	if(!self.isTome)
	{
		for(nbElemToGrab = localProject.nombreChapitre - nbElemToGrab; nbElemToGrab < localProject.nombreChapitre; nbElemToGrab++)
		{
			if(![tabMDL proxyCheckForCollision :localProject : self.isTome :localProject.chapitresFull[nbElemToGrab]])
				selection[nbElemValidated++] = localProject.chapitresFull[nbElemToGrab];
		}
	}
	else
	{
		for(nbElemToGrab = localProject.nombreTomes - nbElemToGrab; nbElemToGrab < localProject.nombreTomes; nbElemToGrab++)
		{
			if(![tabMDL proxyCheckForCollision :localProject : self.isTome :localProject.tomesFull[nbElemToGrab].ID])
				selection[nbElemValidated++] = localProject.tomesFull[nbElemToGrab].ID;
		}
	}
	
	//We got the data, now, craft the alert. ARC will complain if we don't put it in a var...
	RakReaderControllerUIQuery *letItRun = [RakReaderControllerUIQuery alloc];
	letItRun = [letItRun initWithData : tabMDL : _project :self.isTome :selection :nbElemValidated];
}

#pragma mark - Quit

- (void) flushCache
{
	cacheSession++;		//tell the cache to stop
	_flushingCache = true;
	
	if(mainScroller != nil)
	{
		MUTEX_LOCK(cacheMutex);
		
		NSMutableArray * array = [NSMutableArray arrayWithArray:mainScroller.arrangedObjects];
		
		[array removeAllObjects];
		[array insertObject:@(0) atIndex:0];
		
		_scrollView = nil;
		
		mainScroller.selectedIndex = 0;
		mainScroller.arrangedObjects = array;
		
		MUTEX_UNLOCK(cacheMutex);
	}
	
	_flushingCache = false;
}

- (void) deallocInternal
{
	[self flushCache];
	releaseDataReader(&_data);
	
	NSArray * array = [NSArray arrayWithArray:container.subviews], *subArray;

	for(NSView * view in array)	//In theory, it's NSPageView background, so RakGifImageView, inside a superview
	{
		subArray = [NSArray arrayWithArray:view.subviews];
		
		for(NSView * subview in subArray)
		{
			[subview removeFromSuperview];
		}
		
		[view removeFromSuperview];
	}
}

@end

@implementation RakArgumentToRefreshAlert
@end

