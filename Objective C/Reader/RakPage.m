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

#ifdef DEV_VERSION
//	#define PERF_ANALYSIS
#endif

enum
{
	COM_CT_SELEC,
	COM_CT_REFRESH
};

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
	
/*	if(mainScroller.arrangedObjects[_data.pageCourante])
	{
		[self failure];
		return NO;
	}*/
	
	return YES;
}

- (BOOL) isEditable
{
	return NO;
}

- (BOOL) allowsCutCopyPaste
{
	return NO;
}

- (NSString *) getContextToGTFO
{
	NSPoint sliders = NSZeroPoint;

	if(_scrollView != nil)
		sliders = [[_scrollView contentView] bounds].origin;
	
	return [NSString stringWithFormat:@"%s\n%d\n%d\n%d\n%d\n%.0f\n%.0f", _project.team->URLRepo, _project.projectID, _currentElem, self.isTome ? 1 : 0, _data.pageCourante, sliders.x, sliders.y];
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
	
	if(!readerMode)	//Dans ce contexte, les calculs de largeur n'ont aucune importance
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
	if(!readerMode)
		frameRect.size.width = container.frame.size.width;
	
	[container setFrame:NSMakeRect(0, 0, frameRect.size.width, frameRect.size.height)];
	
	if(_scrollView != nil)
	{
		if(!readerMode)
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
		RakGifImageView * view = [mainScroller.selectedViewController.view.subviews objectAtIndex:0];
		if([view class] == [RakGifImageView class])
		{
			NSRect frame = view.frame;		//view is smaller than the smallest possible reader, so its h/w won't change
			
			frame.origin.y = frameRect.size.height / 2 - frame.size.height / 2;

			if(readerMode)
				frame.origin.x = frameRect.size.width / 2 - frame.size.width / 2;
			
			[view.superview setFrame:frame];
		}
	}
}

- (void) leaveReaderMode
{
	readerMode = false;
}

- (void) startReaderMode
{
	readerMode = true;
}

/*Event handling*/

#pragma mark    -   Events

- (void)mouseUp:(NSEvent *)theEvent
{
	bool fail = false;

	if(!readerMode || !noDrag || _scrollView == nil)
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
	{
		[super mouseDown:NULL];
		[super mouseUp:theEvent];
	}
	else
		[self nextPage];
}

- (void) keyDown:(NSEvent *)theEvent
{
	NSString*   const   character   =   [theEvent charactersIgnoringModifiers];
    unichar     const   code        =   [character characterAtIndex:0];
	bool isModPressed = ([theEvent modifierFlags] & (NSAlternateKeyMask | NSShiftKeyMask)) != 0;
	
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
					[self nextPage];
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
	NSLog(@"Something went wrong delete?");
}

#pragma mark - High level API

- (void) nextPage
{
	[self changePage:READER_ETAT_NEXTPAGE];
	MUTEX_LOCK(cacheMutex);
	mainScroller.selectedIndex = _data.pageCourante;
	MUTEX_UNLOCK(cacheMutex);
}

- (void) prevPage
{
	[self changePage:READER_ETAT_PREVPAGE];
	MUTEX_LOCK(cacheMutex);
	mainScroller.selectedIndex = _data.pageCourante;
	MUTEX_UNLOCK(cacheMutex);
}

- (void) nextChapter
{
	[self changeChapter:true];
}

- (void) prevChapter
{
	[self changeChapter:false];
}

- (void) moveSliderX : (int) move
{
	if(_scrollView == nil || !_scrollView.pageTooLarge)
		return;
	
	NSPoint point = [[_scrollView contentView] bounds].origin;
	
	if(move < 0 && point.x < -move)
		point.x = 0;
	
	else if(move > 0)
	{
		CGFloat basePos = [_scrollView.documentView frame].size.width - _scrollView.frame.size.width;
		if(point.x > basePos - move)
			point.x = basePos;
		else
			point.x += move;
	}
	else
		point.x += move;
	
	[_scrollView.contentView scrollToPoint:point];
}

- (void) moveSliderY : (int) move
{
	if(_scrollView == nil || !_scrollView.pageTooHigh)
		return;
	
	NSPoint point = [[_scrollView contentView] bounds].origin;
	
	if(move < 0 && point.y < -move)
		point.y = 0;
	
	else if(move > 0)
	{
		CGFloat basePos = [_scrollView.documentView frame].size.height - _scrollView.frame.size.height;
		if(point.y > basePos - move)
			point.y = basePos;
		else
			point.y += move;
	}
	else
		point.y += move;
	
	[_scrollView.contentView scrollToPoint:point];
}

- (void) setSliderPos : (NSPoint) newPos
{
	if (_scrollView != nil)
	{
		NSPoint point = [[_scrollView contentView] bounds].origin;
		
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
	
	updateIfRequired(&_project, RDB_CTXLECTEUR);
	
	getUpdatedCTList(&_project, true);
	getUpdatedCTList(&_project, false);
	
	_posElemInStructure = reader_getPosIntoContentIndex(_project, _currentElem, self.isTome);
	if(_posElemInStructure == -1)
	{
		[self failure];
		return NO;
	}
	
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

#ifdef DEV_VERSION
	void updateChapter(DATA_LECTURE * dataLecteur, int numeroChapitre);
#endif

- (NSData *) getPage : (uint) posData
{
	if(_data.path == NULL)
	{
		[self failure];
		return nil;
	}
	
#ifdef PERF_ANALYSIS
	struct timeval t1, t2;
    double elapsedTime;
	gettimeofday(&t1, NULL);
#endif
	
	IMG_DATA * dataPage = loadSecurePage(_data.path[_data.pathNumber[posData]], _data.nomPages[posData], _data.chapitreTomeCPT[_data.pathNumber[posData]], _data.pageCouranteDuChapitre[posData]);
	
#ifdef PERF_ANALYSIS
	gettimeofday(&t2, NULL);
	
    // compute and print the elapsed time in millisec
    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000 + (t2.tv_usec - t1.tv_usec) / 1000.0;
	NSLog(@"Loading time: %f", elapsedTime);
#endif
	
	if(dataPage == IMGLOAD_INCORRECT_DECRYPTION)
	{
#ifdef DEV_VERSION
		updateChapter(&_data, _currentElem);
		dataPage = loadSecurePage(_data.path[_data.pathNumber[posData]], _data.nomPages[posData], _data.chapitreTomeCPT[_data.pathNumber[posData]], _data.pageCouranteDuChapitre[posData]);
		
		if(dataPage == (void*) 0x1)
			dataPage = NULL;
#endif
	}
	else if(dataPage == IMGLOAD_NEED_CREDENTIALS_MAIL || dataPage == IMGLOAD_NEED_CREDENTIALS_PASS)
	{
		if(dataPage == IMGLOAD_NEED_CREDENTIALS_PASS)
			_needPassword = true;
		
		MUTEX_VAR * lock = [[NSApp delegate] sharedLoginMutex : YES];
		
		[self performSelectorOnMainThread:@selector(setWaitingLoginWrapper:) withObject:@(true) waitUntilDone:NO];
		
		while(COMPTE_PRINCIPAL_MAIL == NULL || (_needPassword && !getPassFromCache(NULL)))
		{
			pthread_cond_wait([[NSApp delegate] sharedLoginLock], lock);
		}
		
		pthread_mutex_unlock(lock);

		[self performSelectorOnMainThread:@selector(setWaitingLoginWrapper:) withObject:@(false) waitUntilDone:NO];
		
		return [self getPage : posData];
	}
	else if(dataPage == IMGLOAD_NODATA)
	{
		[self failure];
		return NULL;
	}

	NSData *output = [NSData dataWithBytes:dataPage->data length:dataPage->length];
	
	free(dataPage->data);
	free(dataPage);

	return output;
}

- (void) changePage : (byte) switchType
{
	if(switchType == READER_ETAT_NEXTPAGE)
	{
		if(_data.pageCourante + 1 > _data.nombrePageTotale)
		{
			[self changeChapter:true];
			return;
		}
		_data.pageCourante++;
	}
	else if(switchType == READER_ETAT_PREVPAGE)
	{
		if(_data.pageCourante < 1)
		{
			[self changeChapter:false];
			return;
		}
		_data.pageCourante--;
	}
	
	if(switchType != READER_ETAT_DEFAULT)
	{
		mainScroller.selectedIndex = _data.pageCourante;
	}
	
	previousMove = switchType;
	
	[self updatePage:_data.pageCourante : _data.nombrePageTotale];	//And we update the bar
	
	if(switchType == READER_ETAT_DEFAULT)
		[self updateEvnt];
	else
	{
		if([mainScroller.arrangedObjects[_data.pageCourante] class] == [RakPageScrollView class])
			_scrollView = mainScroller.arrangedObjects[_data.pageCourante];
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

- (void) changeChapter : (bool) goToNext
{
	uint newPosIntoStruct = _posElemInStructure;
	
	if(changeChapter(&_project, self.isTome, &_currentElem, &newPosIntoStruct, goToNext))
	{
		cacheSession++;
		_posElemInStructure = newPosIntoStruct;
		[self updateCT : COM_CT_SELEC];
		[self updateContext];
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
		[self updateCT : COM_CT_SELEC];
		[self changePage:READER_ETAT_DEFAULT];
	}
	
	addRecentEntry(_project, false);
}

- (void) updateCT : (uint) request
{
	CTSelec * tabCT = [[NSApp delegate] CT];
	
	if(request == COM_CT_SELEC)
	{
		_dontGiveACrapAboutCTPosUpdate = true;
		[tabCT selectElem: _project.cacheDBID :self.isTome :_currentElem];
		_dontGiveACrapAboutCTPosUpdate = false;
	}
	else if(request == COM_CT_REFRESH)
	{
		[tabCT refreshCT:NO :_project.cacheDBID];
	}
}

- (void) updateContext
{
	[self flushCache];
	releaseDataReader(&_data);
	
	if(updateIfRequired(&_project, RDB_CTXLECTEUR))
	{
		checkChapitreValable(&_project, NULL);
		checkTomeValable(&_project, NULL);
	}
	
	setLastChapitreLu(_project, self.isTome, _currentElem);
	if(reader_isLastElem(_project, self.isTome, _currentElem))
        [self performSelectorInBackground:@selector(checkIfNewElements) withObject:nil];
	
	_data.pageCourante = 0;
	
	if(configFileLoader(_project, self.isTome, _currentElem, &_data))
		[self failure];
	
	[self changePage:READER_ETAT_DEFAULT];
}

- (void) updateEvnt
{
	//We rebuild the cache from scratch
	NSMutableArray * array = [NSMutableArray arrayWithArray:mainScroller.arrangedObjects];
	
	[array removeAllObjects];
	for(uint i = 0; i <= _data.nombrePageTotale; i++)
		[array addObject:@(i)];
	
	_scrollView = nil;
	
	[array replaceObjectAtIndex:_data.pageCourante withObject:@(_data.pageCourante)];
	mainScroller.arrangedObjects = array;
	
	if(mainScroller != nil)
	{
		MUTEX_LOCK(cacheMutex);
		mainScroller.selectedIndex = _data.pageCourante;
		MUTEX_UNLOCK(cacheMutex);
	}

	[self performSelectorInBackground:@selector(buildCache:) withObject:@(++cacheSession)];
}

- (RakPageScrollView *) getScrollView : (uint) page
{
	NSData * imageData = [self getPage:page];
	
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
	
	internalDeleteCT(_project, self.isTome, _currentElem);
	
	[self updateCT:COM_CT_REFRESH];
	
	getUpdatedCTList(&_project, self.isTome);
	
	if(_posElemInStructure != (self.isTome ? _project.nombreTomesInstalled : _project.nombreChapitreInstalled))
		[self nextChapter];
	else if(_posElemInStructure > 0)
		[self prevChapter];
	else
		[self failure];
}

- (void) addPageToView : (NSImage *) page : (RakPageScrollView *) scrollView
{
	if(page == nil || scrollView == nil)
		return;
	
	scrollView.contentFrame = NSMakeRect(0, 0, page.size.width, page.size.height + READER_PAGE_BORDERS_HIGH);

	//We create the view that si going to be displayed
	NSImageView * pageView = [[[NSImageView alloc] initWithFrame: scrollView.contentFrame] autorelease];
	[pageView setImageAlignment:NSImageAlignCenter];
	[pageView setImageFrameStyle:NSImageFrameNone];
	[pageView setImage:page];
	
	scrollView.documentView = pageView;
	
	[page release];
	
	[self initialPositionning : scrollView];
	
	[scrollView setFrame : scrollView.scrollViewFrame];
	[self updateScrollerAfterResize : scrollView : NSZeroSize];
}

- (void) updateScrollerAfterResize : (RakPageScrollView *) scrollView : (NSSize) previousSize
{
	NSPoint sliderStart = NSMakePoint(0, READER_PAGE_TOP_BORDER);
	
	if(previousSize.height == 0 && previousSize.width == 0)	//NSZeroSize
	{
		if (scrollView.pageTooHigh)
			sliderStart.y = ((NSView*) scrollView.documentView).frame.size.height - scrollView.scrollViewFrame.size.height;
		
		if(scrollView.pageTooLarge)
			sliderStart.x = ((NSView*) scrollView.documentView).frame.size.width - scrollView.scrollViewFrame.size.width;
	}
	else
	{
		sliderStart = [[_scrollView contentView] bounds].origin;
		
		if(scrollView.pageTooHigh)
			sliderStart.y += (previousSize.height - scrollView.scrollViewFrame.size.height) / 2;

		if(scrollView.pageTooLarge)
			sliderStart.x += (previousSize.width - scrollView.scrollViewFrame.size.width) / 2;
	}
	
	[scrollView performSelectorOnMainThread:@selector(enforceScrollerPolicy) withObject:nil waitUntilDone:NO];
	[scrollView.contentView scrollToPoint:sliderStart];
}

#pragma mark - Cache generation

- (void) buildCache : (NSNumber *) session
{
	_cacheBeingBuilt = true;
	
	uint currentSession = [session unsignedIntValue];
	
	if(_data.pageCourante > _data.nombrePageTotale)	//Données hors de nos bornes
	{
		_cacheBeingBuilt = false;
		return;
	}
	
	NSMutableArray * data = [NSMutableArray arrayWithArray:mainScroller.arrangedObjects];
	
	while (currentSession == cacheSession)	//While the active chapter is still the same
	{
		if([data[_data.pageCourante] class] != [RakPageScrollView class])
		{
			[self loadPageCache: _data.pageCourante : currentSession : &data];
		}
		else if([self nbEntryRemaining : data])
		{
			char move = previousMove == READER_ETAT_PREVPAGE ? -1 : 1;	//Next page by default
			uint i, max = _data.nombrePageTotale;

			for(i = 0; i < 5 && _data.pageCourante + i * move <= max; i++)
			{
				if([data[_data.pageCourante + i * move] class] != [RakPageScrollView class])
				{
					[self loadPageCache:_data.pageCourante + i * move :currentSession :&data];
					move = 0;
					break;
				}
			}
			
			if(!move)		//If we found something, we go back to the begining of the loop
				continue;
			
			//We cache the previous page, in the case the user want to go back
			if(_data.pageCourante - move < max && [data[_data.pageCourante - move] class] != [RakPageScrollView class])
			{
				[self loadPageCache:_data.pageCourante - move : currentSession : &data];
			}

			else	//Ok then, we cache everythin after
			{
				for (i = _data.pageCourante; i < max; i++)
				{
					if([data[i] class] != [RakPageScrollView class])
					{
						[self loadPageCache:i :currentSession :&data];
						break;
					}
				}
				
				if(i == max)	//Nothing else to load
				{
					break;
				}
			}
		}
		else
		{
			break;
		}
	}
	
	_cacheBeingBuilt = false;
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

- (void) optimizeCache : (NSMutableArray *) data
{
	uint curPage = _data.pageCourante, objectPage, validFound = 0, invalidFound = 0;
	
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
		
		for(object in freeList)
		{
			[object release];
		}
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

- (void) loadPageCache : (uint) page : (uint) currentSession : (NSMutableArray **) data
{
	RakPageScrollView  *view = [self getScrollView:page];
	
	if(view == nil)			//Loading faillure
	{
		[self failure];
		return;
	}
	
	if(currentSession != cacheSession)	//Didn't changed of chapter since the begining of the loading
	{
		[view release];
		return;
	}
	
	[CATransaction begin];
	[CATransaction setDisableActions:YES];
	
	MUTEX_LOCK(cacheMutex);
	
	*data = [NSMutableArray arrayWithArray:mainScroller.arrangedObjects];
	[*data replaceObjectAtIndex:page withObject:view];
	mainScroller.arrangedObjects = [NSArray arrayWithArray:*data];
	
	MUTEX_UNLOCK(cacheMutex);
	
	[CATransaction commit];
	
	if(page == _data.pageCourante)		//If current page, we update the main scrollview pointer (click management)
		_scrollView = view;
}

#pragma mark - NSPageController interface

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
	}
	
	[view setFrame : container.frame];
	
	if(object == nil || [object class] != [RakPageScrollView class])
	{
		RakGifImageView * placeholder = [[[RakGifImageView alloc] initWithFrame:NSMakeRect(0, 0, loadingPlaceholder.size.width, loadingPlaceholder.size.height)] autorelease];
		[placeholder setImage:loadingPlaceholder];
		[viewController.view addSubview : placeholder];
		
		if(object != nil)
			[placeholder startAnimation];
	}
	else
	{
		[self initialPositionning : object];
		
		if(object.page != _data.pageCourante)
			[self updateScrollerAfterResize : object : NSZeroSize];
		
		[object setFrame:object.scrollViewFrame];
		
		[viewController.view addSubview: object];
		viewController.representedObject = object;
	}
}

- (NSRect) pageController : (NSPageController *) pageController frameForObject : (RakPageScrollView*) object
{
	if(object == nil || [object class] != [RakPageScrollView class])
	{
		NSRect frame;
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
	
	uint requestedPage;
	
	if([object superclass] == [NSNumber class])
		requestedPage = [(NSNumber*) object intValue];
	else
		requestedPage = object.page;
	
	if(requestedPage != _data.pageCourante)
	{
		[self changePage : requestedPage > _data.pageCourante ? READER_ETAT_NEXTPAGE : READER_ETAT_PREVPAGE];
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
	
	PROJECT_DATA localProject = getCopyOfProjectData(_project);
	
	uint nbElemToGrab = checkNewElementInRepo(&localProject, self.isTome, _currentElem);
	
	if(!nbElemToGrab)
		return;
	
	RakArgumentToRefreshAlert * argument = [[RakArgumentToRefreshAlert alloc] autorelease];
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
	MDL * tabMDL = [[NSApp delegate] MDL];
	
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
	
	//We got the data, now, craft the alert
	RakReaderControllerUIQuery *test = [RakReaderControllerUIQuery alloc];
	[test initWithData : tabMDL : _project :self.isTome :selection :nbElemValidated];
}

#pragma mark - Quit

- (void) flushCache
{
	cacheSession++;		//tell the cache to stop
	_flushingCache = true;
	
	if(mainScroller != nil)
	{
		NSMutableArray * array = [NSMutableArray arrayWithArray:mainScroller.arrangedObjects];
		for(id object in array)
		{
			if([object class] == [RakPageScrollView class])
			{
				[object release];
			}
		}
		
		[array removeAllObjects];
		[array insertObject:@(0) atIndex:0];
		
		_scrollView = nil;
		
		mainScroller.selectedIndex = 0;
		mainScroller.arrangedObjects = array;
	}
	
	_flushingCache = false;
}

- (void) deallocInternal
{
	[self flushCache];
	releaseDataReader(&_data);
	
	[mainScroller release];
	
	NSArray * array = [NSArray arrayWithArray:container.subviews], *subArray;

	for(NSView * view in array)	//In theory, it's NSPageView background, so RakGifImageView, inside a superview
	{
		subArray = [NSArray arrayWithArray:view.subviews];
		
		for(NSView * subview in subArray)
		{
			[subview removeFromSuperview];
			[subview dealloc];
		}
		
		[view removeFromSuperview];
	}
}

@end

//Shitty class only used because performSelectorOnMainThread want a class, an not two perfectly fine arguments
@implementation RakArgumentToRefreshAlert

- (void) setData : (PROJECT_DATA *) newData
{
	data = newData;
}

- (PROJECT_DATA *) data
{
	return data;
}

- (void) setNbElem : (uint) newData
{
	nbElem = newData;
}

- (uint) nbElem
{
	return nbElem;
}

@end
