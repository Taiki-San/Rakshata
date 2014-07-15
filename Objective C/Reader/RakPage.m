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
	
	if (![self craftPageAndSetupEnv : READER_ETAT_DEFAULT])
		return NO;

	//We create the NSscrollView
	_scrollView = [[NSScrollView alloc] initWithFrame:_scrollViewFrame];
	if(_scrollView == nil)
		return NO;

	[self addPageToView];
		
	//We set preferences ~
	_scrollView.hasVerticalScroller =		_pageTooHigh;
	_scrollView.verticalScroller.alphaValue =	0;
	_scrollView.hasHorizontalScroller =		_pageTooLarge;
	_scrollView.horizontalScroller.alphaValue = 0;
	_scrollView.borderType =				NSNoBorder;
	_scrollView.scrollerStyle =				NSScrollerStyleOverlay;
	_scrollView.drawsBackground =			NO;
	
	[self addSubview:_scrollView];
	[_scrollView release];
	
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
	NSPoint sliders = [[_scrollView contentView] bounds].origin;
	return [NSString stringWithFormat:@"%s\n%d\n%d\n%d\n%d\n%.0f\n%.0f", _project.team->URLRepo, _project.projectID, _currentElem, _isTome ? 1 : 0, _data.pageCourante, sliders.x, sliders.y];
}

/*Handle the position of the whole thing when anything change*/

#pragma mark    -   Position manipulation

- (void) initialPositionning
{
	NSRect tabFrame = [self lastFrame];
	
	_scrollViewFrame = NSZeroRect;

	if(_pageView == nil)
	{
		_pageTooHigh = _pageTooLarge = false;
		_contentFrame = NSZeroRect;
		return;
	}
	
	//Hauteur
	if (tabFrame.size.height < _pageView.frame.size.height)
	{
		_pageTooHigh = true;
		_scrollViewFrame.size.height = tabFrame.size.height;
		_contentFrame.size.height = _pageView.frame.size.height;
	}
	else
	{
		_pageTooHigh = false;
		_scrollViewFrame.origin.y = tabFrame.size.height / 2 - _contentFrame.size.height / 2;
		_scrollViewFrame.size.height = _contentFrame.size.height;
	}
	
	if(!readerMode)	//Dans ce contexte, les calculs de largeur n'ont aucune importance
		return;
	
	//Largeur
	if(tabFrame.size.width < _contentFrame.size.width + 2 * READER_BORDURE_VERT_PAGE)	//	Page trop large
	{
		_pageTooLarge = true;
		_scrollViewFrame.size.width = tabFrame.size.width - 2 * READER_BORDURE_VERT_PAGE;
		_scrollViewFrame.origin.x = READER_BORDURE_VERT_PAGE;
	}
	else
	{
		_pageTooLarge = false;
		_scrollViewFrame.origin.x = tabFrame.size.width / 2 - _contentFrame.size.width / 2;
		_scrollViewFrame.size.width = _contentFrame.size.width;
	}
}

- (void) setFrameInternal : (NSRect) frameRect : (BOOL) isAnimated
{
	if(!readerMode)
	{
		frameRect.size.width = _scrollView.frame.size.width;
		frameRect.origin.x = _scrollView.frame.origin.x;
		frameRect.origin.y = _scrollView.frame.origin.y;
	}
	
	[self initialPositionning];
	[self updateScrollerAfterResize];
	
	if(isAnimated)
		[_scrollView.animator setFrame:_scrollViewFrame];
	else
		[_scrollView setFrame:_scrollViewFrame];
	
	[_pageView setFrameOrigin:_contentFrame.origin];
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

	if(!readerMode || !noDrag)
		fail = true;
	else
	{
		NSPoint mouseLoc = [self convertPoint:[theEvent locationInWindow] fromView:nil];
		
		if(_pageTooHigh)
		{
			mouseLoc.y += [_scrollView.contentView documentRect].size.height - [_scrollView frame].size.height - [_scrollView.contentView documentVisibleRect].origin.y;
			if(mouseLoc.y < READER_PAGE_TOP_BORDER || mouseLoc.y > [(NSView*) _scrollView.documentView frame].size.height - READER_PAGE_BOTTOM_BORDER)
				fail = true;
		}
		
		if(_pageTooLarge)
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

- (void)scrollWheel:(NSEvent *)theEvent
{
	if((_pageTooHigh && [theEvent deltaY]) || (_pageTooLarge && [theEvent deltaX]))
		[super scrollWheel:theEvent];
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
}

- (void) prevPage
{
	[self changePage:READER_ETAT_PREVPAGE];
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
	if(!_pageTooLarge)
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
	if(!_pageTooHigh)
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
	NSPoint point = [[_scrollView contentView] bounds].origin;
	
	[self moveSliderX : newPos.x - point.x];
	[self moveSliderY : newPos.y - point.y];
}

/*Active routines*/

#pragma mark    -   Active routines

- (BOOL) initialLoading : (PROJECT_DATA) dataRequest : (int) elemRequest : (BOOL) isTomeRequest : (int) startPage
{
	_project = getCopyOfProjectData(dataRequest);
	_currentElem = elemRequest;
	_isTome = isTomeRequest;
	
	_prevPage = _pageData = _nextPage = nil;
	_cacheBeingBuilt = false;
	
	updateIfRequired(&_project, RDB_CTXLECTEUR);
	
	getUpdatedCTList(&_project, true);
	getUpdatedCTList(&_project, false);
	
	_posElemInStructure = reader_getPosIntoContentIndex(_project, _currentElem, _isTome);
	if(_posElemInStructure == -1)
	{
		[self failure];
		return NO;
	}
	
	setLastChapitreLu(_project, _isTome, _currentElem);
	if(reader_isLastElem(_project, _isTome, _currentElem))
	{
		[self performSelectorInBackground:@selector(checkIfNewElements) withObject:nil];
	}
	
	if(configFileLoader(_project, _isTome, _currentElem, &_data))
	{
		[self failure];
		return NO;
	}
	
	if(startPage < 0)
		_data.pageCourante = 0;
	else if(startPage < _data.nombrePageTotale)
		_data.pageCourante = startPage;
	else
		_data.pageCourante = _data.nombrePageTotale - 1;
	
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
	
	if(dataPage == (void*) 0x1)
	{
#ifdef DEV_VERSION
		updateChapter(&_data, _currentElem);
		dataPage = loadSecurePage(_data.path[_data.pathNumber[posData]], _data.nomPages[posData], _data.chapitreTomeCPT[_data.pathNumber[posData]], _data.pageCouranteDuChapitre[posData]);
		
		if(dataPage == (void*) 0x1)
			dataPage = NULL;
#endif
	}

	if(dataPage == NULL)
	{
		[self failure];
		return NULL;
	}

	NSData *output = [NSData dataWithBytes:dataPage->data length:dataPage->length];
	
	free(dataPage->data);
	free(dataPage);

	return output;
}

- (void) buildCache
{
	_cacheBeingBuilt = true;
	
	int localCurrentPage = _data.pageCourante;
	
	if(localCurrentPage < 0 || localCurrentPage >= _data.nombrePageTotale)	//Données hors de nos bornes
	{
		_cacheBeingBuilt = false;
		return;
	}
	
	if(_nextPage == nil)
	{
		if (localCurrentPage >= 0 && localCurrentPage < _data.nombrePageTotale)
		{
			_nextPage = [self getPage:localCurrentPage+1];
			[_nextPage retain];
		}
	}

	if(_prevPage == nil)
	{
		if(localCurrentPage > 0)
		{
			_prevPage = [self getPage:localCurrentPage-1];
			[_prevPage retain];
		}
	}
	
	_cacheBeingBuilt = false;
}

- (void) changePage : (byte) switchType
{
	if(switchType == READER_ETAT_NEXTPAGE)
	{
		if(_data.pageCourante+1 > _data.nombrePageTotale)
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
	else if(switchType != READER_ETAT_DEFAULT)
	{
		NSLog(@"Couldn't understand which direction I should move to");
		return;
	}
	
	if([self craftPageAndSetupEnv : switchType])
	{
		[self addPageToView];
		
		//And we update the bar
		[self updatePage:_data.pageCourante : _data.nombrePageTotale];
	}
	else
		[self failure];
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
		[self changePage:READER_ETAT_DEFAULT];
	}
}

- (void) changeChapter : (bool) goToNext
{
	uint newPosIntoStruct = _posElemInStructure;
	
	if(changeChapter(&_project, _isTome, &_currentElem, &newPosIntoStruct, goToNext))
	{
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
	else if(elemRequest == _currentElem && isTomeRequest == _isTome)
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
	NSArray * array = self.superview.subviews;
	
	for (uint i = 0, count = [array count]; i < count; i++)
	{
		if([[array objectAtIndex:i] class] == [CTSelec class])
		{
			if(request == COM_CT_SELEC)
			{
				_dontGiveACrapAboutCTPosUpdate = true;
				[(CTSelec*) [array objectAtIndex:i] selectElem: _project.cacheDBID :_isTome :_currentElem];
				_dontGiveACrapAboutCTPosUpdate = false;
			}
			else if(request == COM_CT_REFRESH)
			{
				[(CTSelec*) [array objectAtIndex:i] refreshCT:NO :_project.cacheDBID];
			}
			return;
		}
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
	
	setLastChapitreLu(_project, _isTome, _currentElem);
	if(reader_isLastElem(_project, _isTome, _currentElem))
        [self performSelectorInBackground:@selector(checkIfNewElements) withObject:nil];
	
	_data.pageCourante = 0;
	
	if(configFileLoader(_project, _isTome, _currentElem, &_data))
		[self failure];
	
	[self changePage:READER_ETAT_DEFAULT];
}

- (BOOL) craftPageAndSetupEnv : (byte) switchType
{
	while(_cacheBeingBuilt)
		usleep(25);
	
	if(switchType == READER_ETAT_DEFAULT)
	{
		//We rebuild the cache from scratch
		if(_prevPage != nil)
		{
			[_prevPage release];
			_prevPage = nil;
		}
		
		if (_pageData != nil)
		{
			[_pageData release];
		}
		
		if(_nextPage != nil)
		{
			[_nextPage release];
			_nextPage = nil;
		}

		_pageData = [self getPage : _data.pageCourante];
		[_pageData retain];
	}
	else if(switchType == READER_ETAT_PREVPAGE)
	{
		if(_nextPage != nil)
			[_nextPage release];
		
		_nextPage = _pageData;
		
		if(_prevPage == nil)
		{
			_pageData = [self getPage : _data.pageCourante];
			[_pageData retain];
		}
		else
		{
			_pageData = _prevPage;
			_prevPage = nil;
		}
	}
	else
	{
		if(_prevPage != nil)
			[_prevPage release];
		
		_prevPage = _pageData;
		
		if(_nextPage == nil)
		{
			_pageData = [self getPage : _data.pageCourante];
			[_pageData retain];
		}
		else
		{
			_pageData = _nextPage;
			_nextPage = nil;
		}
	}
	
	if(_page != nil)
		_pageView.image = nil; //Somehow, it's enough to dealloc the thing
	
	_page = [[NSImage alloc] initWithData : _pageData];
	
	if(_page == nil)
	{
#ifdef DEV_VERSION
		FILE * lol = fopen("lol.png", "wb");
		void * buffer = malloc([_pageData length]);
		if(lol != NULL && buffer != NULL)
		{
			[_pageData getBytes:buffer length:[_pageData length]];
			fwrite(buffer, [_pageData length], 1, lol);
			fclose(lol);
		}
		else if(lol != NULL)
			fclose(lol);
		else
			free(buffer);
#endif
		return false;
	}
	
	[_page setCacheMode:NSImageCacheNever];
	[self performSelectorInBackground:@selector(buildCache) withObject:nil];
	
	return true;
}

- (void) deleteElement
{
	NSAlert * alert = [[[NSAlert alloc] init] autorelease];
	
	[alert setAlertStyle:NSInformationalAlertStyle];
	[alert setMessageText:[NSString stringWithFormat:@"Suppression d'un %s", _isTome ? "tome" : "chapitre"]];
	[alert setInformativeText :[NSString stringWithFormat:@"Attention: vous vous apprêtez à supprimer définitivement un %s, pour le relire, vous aurez à le télécharger de nouveau, en êtes vous sûr?", _isTome ? "tome" : "chapitre"]];
	
	[alert addButtonWithTitle:@"NON!"];
	NSButton * firstButton = [[alert buttons] objectAtIndex:0];
	[firstButton setTitle:@"I want it dead"];
	[alert addButtonWithTitle:@"NON!"];
	
	if([alert runModal] == NSAlertFirstButtonReturn)
	{
		while (_cacheBeingBuilt);
		internalDeleteCT(_project, _isTome, _currentElem);
		
		[self updateCT:COM_CT_REFRESH];
		
		getUpdatedCTList(&_project, _isTome);
		
		if(_posElemInStructure != (_isTome ? _project.nombreTomesInstalled : _project.nombreChapitreInstalled))
			[self nextChapter];
		else if(_posElemInStructure > 0)
			[self prevChapter];
		else
			[self failure];
	}
}

- (void) addPageToView
{
	if(_page == nil)
		return;
	
	NSRect frame = NSMakeRect(0, 0, _page.size.width, _page.size.height + READER_PAGE_BORDERS_HIGH);

	//We create the view that si going to be displayed
	if(_pageView != nil)
	{
		//We reset the scroller first (check commit 5eb6b7fde2db for why this patch is important)
		CGEventRef cgEvent = CGEventCreateScrollWheelEvent(NULL, kCGScrollEventUnitLine, 2, _pageView.frame.size.height, _pageView.frame.size.width);
		NSEvent *theEvent = [NSEvent eventWithCGEvent:cgEvent];
		[super scrollWheel:theEvent];
		CFRelease(cgEvent);
		
		[_pageView setFrame:frame];
		[_pageView setImage:_page];
	}
	else
	{
		_pageView = [[NSImageView alloc] initWithFrame:frame];
		[_pageView setImageAlignment:NSImageAlignCenter];
		[_pageView setImageFrameStyle:NSImageFrameNone];
		[_pageView setImage:_page];
	
		_scrollView.documentView =	_pageView;
		
		[_pageView release];
	}
	_contentFrame = _pageView.frame;
	
	[_page release];
	
	[self initialPositionning];
	
	[_scrollView setFrame:_scrollViewFrame];
	[_pageView setFrameOrigin:_contentFrame.origin];
	
	[self updateScrollerAfterResize];
}

- (void) updateScrollerAfterResize
{
	NSPoint sliderStart;
	
	if (_pageTooHigh)
		sliderStart.y = ((NSView*) _scrollView.documentView).frame.size.height - _scrollViewFrame.size.height;
	else
		sliderStart.y = READER_PAGE_TOP_BORDER;
	
	if(_pageTooLarge)
		sliderStart.x = ((NSView*) _scrollView.documentView).frame.size.width - _scrollViewFrame.size.width;
	else
		sliderStart.x = 0;
	
	_scrollView.hasVerticalScroller = _pageTooHigh;
	_scrollView.verticalScroller.alphaValue =	0;
	_scrollView.hasHorizontalScroller = _pageTooLarge;
	_scrollView.horizontalScroller.alphaValue = 0;
	
	[_scrollView.contentView scrollToPoint:sliderStart];
}

#pragma mark - Checks if new elements to download

- (void) checkIfNewElements
{
	if(_alreadyRefreshed)
		return;
	else
		_alreadyRefreshed = true;
	
	PROJECT_DATA localProject = getCopyOfProjectData(_project);
	
	uint nbElemToGrab = checkNewElementInRepo(&localProject, _isTome, _currentElem);
	
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
	MDL * tabMDL = sharedTabMDL;
	
	if(selection == NULL || tabMDL == nil)
	{
		free(selection);
		return;
	}
	
	if(!_isTome)
	{
		for(nbElemToGrab = localProject.nombreChapitre - nbElemToGrab; nbElemToGrab < localProject.nombreChapitre; nbElemToGrab++)
		{
			if(![tabMDL proxyCheckForCollision :localProject : _isTome :localProject.chapitresFull[nbElemToGrab]])
				selection[nbElemValidated++] = localProject.chapitresFull[nbElemToGrab];
		}
	}
	else
	{
		for(nbElemToGrab = localProject.nombreTomes - nbElemToGrab; nbElemToGrab < localProject.nombreTomes; nbElemToGrab++)
		{
			if(![tabMDL proxyCheckForCollision :localProject : _isTome :localProject.tomesFull[nbElemToGrab].ID])
				selection[nbElemValidated++] = localProject.tomesFull[nbElemToGrab].ID;
		}
	}
	
	//We got the data, now, craft the alert
	RakReaderControllerUIQuery *test = [RakReaderControllerUIQuery alloc];
	[test initWithData :sharedTabMDL : _project :_isTome :selection :nbElemValidated];
}

#pragma mark - Quit

- (void) flushCache
{
	while (_cacheBeingBuilt);
	
	[_prevPage release];		_prevPage = nil;
	[_pageView setImage:nil];	_page = nil;
	[_pageData release];		_pageData = nil;
	[_nextPage release];		_nextPage = nil;
}

- (void) getTheFuckOut
{
	[self flushCache];
	releaseDataReader(&_data);
	
	[_scrollView removeFromSuperview];
	_scrollView.documentView = nil;
	[_scrollView release];
	
	[_pageView release];		_pageView = nil;
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
