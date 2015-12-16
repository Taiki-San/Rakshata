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

- (BOOL) initPage : (PROJECT_DATA) dataRequest : (uint) elemRequest : (BOOL) isTomeRequest : (uint) startPage
{
	if(lastKnownMagnification == 0.0)
		lastKnownMagnification = 1.0;
	
	_alreadyRefreshed = NO;
	_dontGiveACrapAboutCTPosUpdate = NO;
	
	if(![self initialLoading:dataRequest :elemRequest :isTomeRequest : startPage])
		return NO;

	addRecentEntry(dataRequest, false);
	
	if(mainScroller == nil)
	{
		mainScroller = [[RakPageController alloc] init];
		mainScroller.view = container;
		mainScroller.delegate = self;
		[self updateProjectReadingOrder];
	}
	
	[self updateEvnt];
	
	return YES;
}

- (NSString *) getContextToGTFO
{
	return [NSString stringWithFormat:@"%llu\n%d\n%d", getRepoID(_project.repo), _project.projectID, _project.locale];
}

- (STATE_DUMP) exportContext
{
	STATE_DUMP state;
	bzero(&state, sizeof(state));
	
	NSPoint sliders;
	if(_scrollView != nil)
		sliders = [_scrollView contentView].bounds.origin;
	else
		sliders = NSMakePoint(CGFLOAT_MAX, CGFLOAT_MAX);
	
	state.cacheDBID = _project.cacheDBID;

	state.isTome = self.isTome != 0;
	state.CTID = _currentElem;
	state.page = _data.pageCourante;
	state.wasLastPage = _data.pageCourante == _data.nbPage - 1;
	state.zoom = saveMagnification && _scrollView != nil ? _scrollView.magnification : 1.0f;
	state.scrollerX = sliders.x;
	state.scrollerY = sliders.y;
	
	state.isInitialized = true;
	
	return state;
}

/*Handle the position of the whole thing when anything change*/

#pragma mark    -   Position manipulation

- (void) initialPositionning : (RakPageScrollView *) scrollView
{
	NSRect tabFrame = [self lastFrame], scrollViewFrame = scrollView.scrollViewFrame;
	
	//Hauteur
	if(tabFrame.size.height < scrollView.contentFrame.size.height)
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
	
	if(self.mainThread == TAB_READER)	//Le seul contexte où les calculs de largeur ont une importance
	{
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
	}
	
	scrollView.scrollViewFrame = scrollViewFrame;
}

- (void) setFrameInternal : (NSRect) frameRect : (BOOL) isAnimated
{
	if(self.mainThread != TAB_READER)
		frameRect.size.width = container.frame.size.width;
	
	[container setFrame:NSMakeRect(0, 0, frameRect.size.width, frameRect.size.height)];
	
	if(_scrollView == nil)
	{
		NSArray * subview = mainScroller.selectedViewController.view.subviews;
		if(subview == nil || [subview count] == 0)
			return;
		
		RakImageView * view = [subview objectAtIndex:0];
		if([view class] == [RakImageView class])
		{
			NSRect frame = view.frame;		//view is smaller than the smallest possible reader, so its h/w won't change
			
			frame.origin.y = frameRect.size.height / 2 - frame.size.height / 2;
			
			if(self.mainThread == TAB_READER)
				frame.origin.x = frameRect.size.width / 2 - frame.size.width / 2;
			
			[view.superview setFrame:frame];
			return;
		}
		else if([view class] != [RakPageScrollView class])
			return;
		
		if(saveMagnification)
		{
			if(_scrollView != nil && [_scrollView class] == [RakPageScrollView class])
				((RakPageScrollView *) view).magnification = lastKnownMagnification = _scrollView.magnification;
			else
				((RakPageScrollView *) view).magnification = lastKnownMagnification;
		}
		
		_scrollView = (id) view;
		[self commitSliderPosIfNeeded];
	}
	
	if(self.mainThread != TAB_READER)
		frameRect.origin = _scrollView.frame.origin;
	
	[_scrollView.superview setFrame:container.frame];
	
	NSSize oldSize = _scrollView.frame.size;
	
	[self initialPositionning : _scrollView];
	[self updateScrollerAfterResize : _scrollView : oldSize];
	
	if(isAnimated)
		[_scrollView.animator setFrame:container.frame];
	else
		[_scrollView setFrame:container.frame];
}

/*Event handling*/

#pragma mark    -   Events

- (void) mouseUp:(NSEvent *)theEvent
{
	BOOL fail = NO;
	
	if(self.mainThread != TAB_READER || !noDrag || _scrollView == nil)
		fail = YES;
	else
	{
		NSPoint mouseLoc = [self convertPoint:[theEvent locationInWindow] fromView:nil];
		
		if(_scrollView.pageTooHigh)
		{
			mouseLoc.y += [_scrollView.contentView documentRect].size.height - [_scrollView frame].size.height - [_scrollView.contentView documentVisibleRect].origin.y;
			if(mouseLoc.y < READER_PAGE_TOP_BORDER || mouseLoc.y > [_scrollView documentViewFrame].size.height - READER_PAGE_BOTTOM_BORDER)
				fail = YES;
		}
		
		if(_scrollView.pageTooLarge)
		{
			mouseLoc.x += [_scrollView.contentView documentRect].size.width - [_scrollView frame].size.width - [_scrollView.contentView documentVisibleRect].origin.x;
			if(mouseLoc.x < READER_BORDURE_VERT_PAGE || mouseLoc.x > [_scrollView documentViewFrame].size.width - READER_BORDURE_VERT_PAGE)
				fail = YES;
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
	unichar     const   code        =   [character length] > 0 ? [character characterAtIndex:0] : '\0';
	BOOL isModPressed = ((RakAppDelegate*)[NSApp delegate]).window.shiftPressed;
	
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
		case NSRightArrowFunctionKey:
		{
			if(mainScroller.flipped ^ (code == NSLeftArrowFunctionKey))
				[self prevPage];
			else
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
			if(character == nil)
				break;
			
			bool handledChar = false;

			const byte keyCode = [theEvent keyCode];
			
			//We get an hardware independant keycode, then check it against a known database
			//http://boredzo.org/blog/archives/2007-05-22/virtual-key-codes
			
			switch (keyCode)
			{
				case 0:			//a
				case 38:		//j
				{
					[self prevPage];
					handledChar = true;
					break;
				}
					
				case 2:			//d
				case 37:		//l
				{
					if(isModPressed && ((RakAppDelegate*)[NSApp delegate]).window.commandPressed)
					{
						[self switchDistractionFree];
					}
					else
					{
						[self nextPage];
					}

					handledChar = true;
					break;
				}
					
				case 3:			//f
				{
					if(((RakAppDelegate*)[NSApp delegate]).window.commandPressed)
						[self.window toggleFullScreen:self];

					handledChar = true;
					break;
				}
					
				case 12:		//q
				case 32:		//u
				{
					[self prevChapter];
					handledChar = true;
					break;
				}
					
				case 14:		//e
				case 31:			//o
				{
					[self nextChapter];
					handledChar = true;
					break;
				}
					
				case 13:		//w
				case 34:		//i
				{
					[self moveSliderY:PAGE_MOVE];
					handledChar = true;
					break;
				}
					
				case 1:			//s
				case 40:		//k
				{
					[self moveSliderY:-PAGE_MOVE];
					handledChar = true;
					break;
				}
					
				case 49:		//space
				{
					[self jumpPressed : isModPressed];
					handledChar = true;
					break;
				}
			}

			if(handledChar)
				break;
			
			const char * string = [character UTF8String];
			char c;
			
			if(string == nil)
				break;
			
			if(string[0] >= 'A' && string[0] <= 'Z')
				c = string[0] + 'a' - 'A';
			else
				c = string[0];

			switch (c)
			{
				//Magnification
					
				case '0':
				{
					if(((RakAppDelegate*)[NSApp delegate]).window.commandPressed)
						_scrollView.animator.magnification = lastKnownMagnification = 1.0;
					break;
				}
					
				case '+':
				{
					if(((RakAppDelegate*)[NSApp delegate]).window.commandPressed && _scrollView.magnification < READER_MAGNIFICATION_MAX)
					{
						lastKnownMagnification = _scrollView.magnification;
						lastKnownMagnification += (lastKnownMagnification > 1.5) ? 0.5f : 0.25f;
						lastKnownMagnification = MIN(lastKnownMagnification, READER_MAGNIFICATION_MAX);
						_scrollView.animator.magnification = lastKnownMagnification;
					}
					break;
				}
					
				case '-':
				{
					if(((RakAppDelegate*)[NSApp delegate]).window.commandPressed && round(_scrollView.magnification * 4) > 1)
					{
						lastKnownMagnification = _scrollView.magnification;
						lastKnownMagnification -= (lastKnownMagnification > 1.5) ? 0.5f : 0.25f;
						lastKnownMagnification = MAX(lastKnownMagnification, READER_MAGNIFICATION_MIN);
						_scrollView.animator.magnification = lastKnownMagnification;
					}
					break;
				}
			}
		}
	}
}

/*Error management*/

#pragma mark    -   Errors

- (void) failure
{
	[self failure : INVALID_VALUE];
}

- (void) failure : (uint) page
{
	if(!self.initWithNoContent)
		NSLog(@"Something went wrong delete?");
	
	if(page != INVALID_VALUE)
	{
		NSRect frame;
		frame.size = loadingFailedPlaceholder.size;
		frame.origin = NSCenterPoint(_bounds, frame);
		
		RakImageView * image = [[RakImageView alloc] initWithFrame : frame];
		[image setImage : loadingFailedPlaceholder];
		
		if(image != nil)
		{
			image.page = page;

			if(![NSThread isMainThread])
			{
				dispatch_sync(dispatch_get_main_queue(), ^{
					[self updatePCState : page : cacheSession : image];
				});
			}
			else
				[self updatePCState : page : cacheSession : image];
		}
	}
	
	cacheSession++;
}

#pragma mark - DB update

- (void) DBUpdated : (NSNotification*) notification
{
	if([RakDBUpdate analyseNeedUpdateProject:notification.userInfo :_project])
	{
		PROJECT_DATA project = getProjectByID(_project.cacheDBID);
		if(project.isInitialized)
		{
			releaseCTData(_project);
			_project = project;
			_posElemInStructure = reader_getPosIntoContentIndex(_project, _currentElem, self.isTome);
			
			[self updateProjectReadingOrder];
			[bottomBar favsUpdated:_project.favoris];
		}
		else if(!checkProjectStillExist(_project.cacheDBID))
		{
			releaseCTData(_project);
			_project = getEmptyProject();
			self.initWithNoContent = YES;
		}
	}
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] == [Prefs class] && [keyPath isEqualToString:[Prefs getKeyPathForCode:KVO_MAGNIFICATION]])
		[Prefs getPref:PREFS_GET_SAVE_MAGNIFICATION:&saveMagnification];
	
	else if([object class] == [Prefs class] && [keyPath isEqualToString:[Prefs getKeyPathForCode:KVO_DIROVERRIDE]])
	{
		[Prefs getPref:PREFS_GET_DIROVERRIDE:&overrideDirection];
		[self updateProjectReadingOrder];
	}
	
	else
		[super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
}

#pragma mark - High level API

- (void) nextPage
{
	[self nextPage:NO];
}

- (BOOL) nextPage : (BOOL) animated
{
	return [self changePage:READER_ETAT_NEXTPAGE:animated];
}

- (void) prevPage
{
	[self prevPage:NO];
}

- (BOOL) prevPage : (BOOL) animated
{
	return [self changePage:READER_ETAT_PREVPAGE : animated];
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
	
	if(move < 0 && point.x <= 0)
		return NO;
	else if(move < 0 && point.x < -move)
		point.x = 0;
	
	else if(move > 0)
	{
		CGFloat basePos = [_scrollView documentViewFrame].size.width - _scrollView.frame.size.width;
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
		CGFloat basePos = round([_scrollView documentViewFrame].size.height - _scrollView.bounds.size.height);
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
	if(_scrollView != nil)
	{
		NSPoint point = _scrollView.contentView.bounds.origin;
		
		[self moveSliderX : newPos.x - point.x];
		[self moveSliderY : newPos.y - point.y];
	}
	else
	{
		_haveScrollerPosToCommit = YES;
		_scrollerPosToCommit = newPos;
	}
}

- (void) commitSliderPosIfNeeded
{
	if(_haveScrollerPosToCommit)
	{
		_haveScrollerPosToCommit = NO;
		[self setSliderPos:_scrollerPosToCommit];
	}
}

/*Active routines*/

#pragma mark    -   Active routines

- (BOOL) initialLoading : (PROJECT_DATA) dataRequest : (uint) elemRequest : (BOOL) isTomeRequest : (uint) startPage
{
	_project = getCopyOfProjectData(dataRequest);
	_currentElem = elemRequest;
	self.isTome = isTomeRequest;
	
	_cacheBeingBuilt = NO;
	
	_posElemInStructure = reader_getPosIntoContentIndex(_project, _currentElem, self.isTome);
	if(_posElemInStructure == INVALID_VALUE)
	{
		[self failure];
		return NO;
	}
	
	if(_project.haveDRM && !preventWindowCaptureForWindow(self.window))
	{
		[self failure];
		return NO;
	}
	
	[self updateTitleBar:_project :isTomeRequest :_posElemInStructure];
	
	if(reader_isLastElem(_project, self.isTome, _currentElem))
	{
		dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
			[self checkIfNewElements];
		});
	}
	
	if(!readerConfigFileLoader(_project, self.isTome, _currentElem, &_data))
	{
		[self failure];
		return NO;
	}
	
	[self updateProjectReadingOrder];
	
	if(startPage == INVALID_VALUE || _data.nbPage == 0)
		_data.pageCourante = 0;
	else if(startPage < _data.nbPage)
		_data.pageCourante = startPage;
	else
		_data.pageCourante = _data.nbPage - 1;
	
	return YES;
}

- (NSData *) getPage : (uint) posData : (DATA_LECTURE*) data
{
	IMG_DATA * dataPage = loadSecurePage(data->path[data->pathNumber[posData]], data->nomPages[posData], data->chapitreTomeCPT[data->pathNumber[posData]], data->pageCouranteDuChapitre[posData]);
	
	if(dataPage == IMGLOAD_INCORRECT_DECRYPTION)
		return nil;
	
	else if(dataPage == IMGLOAD_NEED_CREDENTIALS_MAIL || dataPage == IMGLOAD_NEED_CREDENTIALS_PASS)
	{
		//Incorrect account :X
		if(COMPTE_PRINCIPAL_MAIL != NULL && (!_needPassword || getPassFromCache(NULL)))
			return nil;
		
		if(dataPage == IMGLOAD_NEED_CREDENTIALS_PASS)
			_needPassword = YES;
		
		MUTEX_VAR * lock = [(RakAppDelegate*) [NSApp delegate]sharedLoginMutex : YES];
		
		[self performSelectorOnMainThread:@selector(setWaitingLoginWrapper:) withObject:@(YES) waitUntilDone:NO];
		
		while(COMPTE_PRINCIPAL_MAIL == NULL || (_needPassword && !getPassFromCache(NULL)))
		{
			pthread_cond_wait([(RakAppDelegate*) [NSApp delegate] sharedLoginLock], lock);
		}
		
		pthread_mutex_unlock(lock);
		
		[self performSelectorOnMainThread:@selector(setWaitingLoginWrapper:) withObject:@(NO) waitUntilDone:NO];
		
		return [self getPage : posData : data];
	}
	else if(dataPage == IMGLOAD_NODATA)
		return nil;
	
	NSData *output = [NSData dataWithBytes:dataPage->data length:dataPage->length];
	
	free(dataPage->data);
	free(dataPage);
	
	return output;
}

- (BOOL) changePage : (byte) switchType
{
	return [self changePage:switchType :NO];
}

- (BOOL) changePage : (byte) switchType : (BOOL) animated
{
	if(switchType == READER_ETAT_NEXTPAGE)
	{
		if(_data.pageCourante + 1 >= _data.nbPage)
		{
			return [self changeChapter : YES : YES];
		}
		_data.pageCourante++;
	}
	else if(switchType == READER_ETAT_PREVPAGE)
	{
		if(_data.pageCourante < 1)
		{
			return [self changeChapter : NO : YES];
		}
		_data.pageCourante--;
	}
	
	//We have to change the page ourselves
	if(switchType != READER_ETAT_DEFAULT && mainScroller.patchedSelectedIndex != _data.pageCourante + 1)
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
			[CATransaction begin];
			[CATransaction setDisableActions:YES];
			mainScroller.patchedSelectedIndex = _data.pageCourante + 1;
			[CATransaction commit];
		}
		self.preventRecursion = NO;
		
		MUTEX_UNLOCK(cacheMutex);
	}
	
	previousMove = switchType;
	
	[self updatePage:_data.pageCourante : _data.nbPage];	//And we update the bar
	
	if(switchType == READER_ETAT_DEFAULT)
		[self updateEvnt];
	else
	{
		RakPageScrollView * view = mainScroller.arrangedObjects[[mainScroller getPatchedPosForIndex:_data.pageCourante + 1]];
		if([view class] == [RakPageScrollView class])
		{
			if(saveMagnification)
			{
				if(_scrollView != nil && [_scrollView class] == [RakPageScrollView class])
					view.magnification = lastKnownMagnification = _scrollView.magnification;
				else
					view.magnification = lastKnownMagnification;
			}
			
			_scrollView = view;
			[self commitSliderPosIfNeeded];
		}
		else
			_scrollView = nil;
		
		[self optimizeCache : nil];
	}
	
	return YES;
}

- (void) jumpToPage : (uint) newPage
{
	if(newPage == _data.pageCourante || _data.nbPage == 0)
		return;
	
	else if(newPage >= _data.nbPage)
		newPage = _data.nbPage - 1;
	
	uint pageCourante = _data.pageCourante;
	
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

- (BOOL) changeChapter : (BOOL) goToNext : (BOOL) byChangingPage
{
	uint newPosIntoStruct = _posElemInStructure;
	
	MUTEX_LOCK(cacheMutex);
	
	if(!changeChapter(&_project, self.isTome, &_currentElem, &newPosIntoStruct, goToNext))
	{
		MUTEX_UNLOCK(cacheMutex);
		
		//Trying to go to the next page from the last available page
		if(goToNext && byChangingPage)
		{
			NSInteger count = (NSInteger) [mainScroller.arrangedObjects count];
			if(count > 3 && mainScroller.patchedSelectedIndex == count - 1)
			{
				mainScroller.patchedSelectedIndex = count - 2;
			}
			
#ifdef LEAVE_DISTRACTION_FREE_AT_END
			if(self.distractionFree)
			{
				oldDFState = YES;
				[self switchDistractionFree];
			}
#endif
			
			[self displaySuggestions];
		}
		return NO;
	}
	else
		oldDFState = NO;

	MUTEX_UNLOCK(cacheMutex);
	
	cacheSession++;
	_posElemInStructure = newPosIntoStruct;
	
	[self updateTitleBar:_project :self.isTome :_posElemInStructure];
	[self updateCTTab : NO];
	
	if((goToNext && nextDataLoaded && _nextData.IDDisplayed == _currentElem) || (!goToNext && previousDataLoaded && _previousData.IDDisplayed == _currentElem))
	{
		uint currentPage;
		
		if(goToNext)
		{
			if(previousDataLoaded)
				releaseDataReader(&_previousData);
			
			currentPage = _data.nbPage + 1;
			
			_previousData = _data;
			previousDataLoaded = dataLoaded;
			
			_data = _nextData;
			_data.pageCourante = 0;
			dataLoaded = nextDataLoaded;
			
			nextDataLoaded = NO;
			
		}
		else
		{
			if(nextDataLoaded)
				releaseDataReader(&_nextData);
			
			currentPage = 0;
			
			_nextData = _data;
			nextDataLoaded = dataLoaded;
			
			memcpy(&_data, &_previousData, sizeof(DATA_LECTURE));
			
			if(byChangingPage)
				_data.pageCourante = _data.nbPage - 1;
			else
				_data.pageCourante = 0;
			
			dataLoaded = previousDataLoaded;
			
			previousDataLoaded = NO;
		}
		
		id currentPageView = mainScroller.arrangedObjects[currentPage];
		
		[self updateContext : YES];
		
		if(byChangingPage)
		{
			//We inject the page we already loaded inside mainScroller
			NSMutableArray * array = [mainScroller.arrangedObjects mutableCopy];
			
			[array replaceObjectAtIndex:_data.pageCourante + 1 withObject:currentPageView];
			
			MUTEX_LOCK(cacheMutex);
			mainScroller.arrangedObjects = array;
			_scrollView = currentPageView;
			MUTEX_UNLOCK(cacheMutex);
		}
	}
	else
	{
		if(goToNext && nextDataLoaded)
		{
			nextDataLoaded = NO;
			releaseDataReader(&_nextData);
		}
		else if(!goToNext && previousDataLoaded)
		{
			previousDataLoaded = NO;
			releaseDataReader(&_previousData);
		}
		
		if(dataLoaded)
		{
			dataLoaded = NO;
			releaseDataReader(&_data);
		}
		
		if(!byChangingPage)
			_data.pageCourante = 0;
		
		[self updateContext : NO];
		
		if(byChangingPage)
			[self jumpToPage : _data.nbPage - 1];
	}
	
	return YES;
}

- (void) changeProject : (PROJECT_DATA) projectRequest : (uint) elemRequest : (BOOL) isTomeRequest : (uint) startPage
{
	if(_dontGiveACrapAboutCTPosUpdate)
		return;
	
	BOOL changingProject = projectRequest.cacheDBID != _project.cacheDBID;
	
	if(queryHidden && changingProject)
	{
		free(_queryArrayData);
		_queryArrayData = NULL;
		queryHidden = NO;
	}
	
	if(projectRequest.cacheDBID != _project.cacheDBID)
		_alreadyRefreshed = NO;
	else if(elemRequest == _currentElem && isTomeRequest == self.isTome)
	{
		[self jumpToPage:startPage];
		return;
	}
	
	[self flushCache];
	releaseDataReader(&_data);
	
	if(previousDataLoaded)
	{
		releaseDataReader(&_previousData);
		previousDataLoaded = NO;
	}
	
	if(nextDataLoaded)
	{
		releaseDataReader(&_nextData);
		nextDataLoaded = NO;
	}
	
	if([self initialLoading:projectRequest :elemRequest :isTomeRequest : startPage])
	{
		[self updateCTTab : changingProject];
		[self changePage:READER_ETAT_DEFAULT];
	}
	
	addRecentEntry(_project, false);
}

- (void) updateCTTab : (BOOL) shouldOverwriteActiveProject
{
	CTSelec * tabCT = [(RakAppDelegate*) [NSApp delegate] CT];
	
	if(!shouldOverwriteActiveProject && tabCT.activeProject.cacheDBID != _project.cacheDBID)
		return;
	
	_dontGiveACrapAboutCTPosUpdate = YES;
	[tabCT selectElem: _project.cacheDBID :self.isTome :_currentElem];
	_dontGiveACrapAboutCTPosUpdate = NO;
}

- (void) updateContext : (BOOL) dataAlreadyLoaded
{
	[self flushCache];
	
	if(reader_isLastElem(_project, self.isTome, _currentElem))
	{
		dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
			[self checkIfNewElements];
		});
	}
	
	if(!dataLoaded)
	{
		if(!readerConfigFileLoader(_project, self.isTome, _currentElem, &_data))
		{
			_data.nbPage = 1;
			[self failure : 0];
		}
	}
	
	[self changePage:READER_ETAT_DEFAULT];
}

- (void) updateEvnt
{
	//We rebuild the cache from scratch
	NSMutableArray * array = [NSMutableArray arrayWithArray:mainScroller.arrangedObjects];
	
	[array removeAllObjects];
	
	[array addObject:@(-1)];	//Placeholder for last page of previous chapter
	
	for(uint i = 0; i < _data.nbPage; i++)
		[array addObject:@([mainScroller getPatchedPosForIndex:i])];
	
	[array addObject:@(-2)];	//Placeholder for first page of next chapter
	
	_scrollView = nil;
	
	if(mainScroller != nil)
	{
		MUTEX_LOCK(cacheMutex);

		_flushingCache = YES;
		
		[array replaceObjectAtIndex:[mainScroller getPatchedPosForIndex:_data.pageCourante + 1] withObject:@(_data.pageCourante)];
		
		mainScroller.arrangedObjects = array;
		mainScroller.patchedSelectedIndex = _data.pageCourante + 1;
		
		_flushingCache = NO;

		MUTEX_UNLOCK(cacheMutex);
	}
	
	uint cacheCode = ++cacheSession;
	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
		[self buildCache:@(cacheCode)];
	});
}

- (RakPageScrollView *) getScrollView : (uint) page : (DATA_LECTURE*) data
{
	if(_data.path == NULL || page >= data->nbPage)
		return nil;
	
	NSData * imageData;
	BOOL isPDF = haveSuffixCaseInsensitive(data->nomPages[page], ".pdf");
	//PDF
	if(isPDF)
		imageData = [self getPDF:page :data];
	else
		imageData = [self getPage : page : data];
	
	if(imageData == nil || imageData.length == 0)
		return nil;
	
	NSImage * image = [[NSImage alloc] initWithData : imageData];
	
#ifdef EXTENSIVE_LOGGING
	if(image == nil)
		[imageData writeToFile:@"lol.png" atomically:NO];
#endif
	
	if(image == nil)
		return nil;

	image.cacheMode = NSImageCacheNever;
	
	RakPageScrollView * output = [[RakPageScrollView alloc] init];
	
	[self addPageToView:image :output];
	output.page = page;
	output.isPDF = isPDF;
	
	return output;
}

- (void) deleteElement
{
	cacheSession++;	//Tell the cache system to stop
	
	uint oldCache = _project.cacheDBID;
	
	deleteProject(_project, _currentElem, self.isTome);
	
	if(_project.isInitialized && _posElemInStructure != INVALID_VALUE)
	{
		if(_posElemInStructure != (self.isTome ? _project.nbVolumesInstalled : _project.nbChapterInstalled))
		{
			if(_posElemInStructure > 0)
			{
				_posElemInStructure--;
				return [self nextChapter];
			}
			else
			{
				_posElemInStructure++;
				return [self prevChapter];
			}
		}
		else if(_posElemInStructure > 0)
			return [self prevChapter];
	}

	_data.pageCourante = 0;
	_data.nbPage = 1;
	self.initWithNoContent = YES;
	[self failure : 0];
	mainScroller.patchedSelectedIndex = 1;

	if([[[NSApp delegate] CT] activeProject].cacheDBID != oldCache)
		[[[NSApp delegate] CT] ownFocus];
	else
		[[[NSApp delegate] serie] ownFocus];
}

- (void) addPageToView : (NSImage *) page : (RakPageScrollView *) scrollView
{
	if(page == nil || scrollView == nil)
		return;
	
	NSImageRep *rep = [[page representations] objectAtIndex: 0];
	page.size = NSMakeSize(rep.pixelsWide, rep.pixelsHigh);

	scrollView.contentFrame = NSMakeRect(0, 0, page.size.width, page.size.height + READER_PAGE_BORDERS_HIGH);
	
	//We create the view that si going to be displayed
	NSImageView * pageView = [[NSImageView alloc] initWithFrame: scrollView.contentFrame];

	//	pageView.imageAlignment = NSImageAlignCenter
	pageView.imageFrameStyle = NSImageFrameNone;
	pageView.allowsCutCopyPaste = NO;
	
	pageView.image = page;
	scrollView.documentView = pageView;
	
	[CATransaction begin];
	[CATransaction setDisableActions:YES];
	
	[self initialPositionning : scrollView];
	scrollView.magnification = 1;
	
	[scrollView setFrame : container.bounds];
	[scrollView scrollToBeginningOfDocument];
	
	[CATransaction commit];
}

- (void) updateScrollerAfterResize : (RakPageScrollView *) scrollView : (NSSize) previousSize
{
	NSPoint sliderStart = [_scrollView.contentView bounds].origin;

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
		
		if(withShift ^ !_project.rightToLeft)
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
			else if([self nextPage : YES])
			{
				[self scrollToExtreme : _scrollView : YES];
			}
		}
		else
		{
			//moveSliderX initiate an animation, so those lines have no effect for now...
//			if(withShift)
//				[_scrollView scrollToBottomOfDocument];
//			else
//				[_scrollView scrollToTopOfDocument];
		}
	}
}

#pragma mark - PDF loading

- (NSData *) getPDF : (uint) posData : (DATA_LECTURE *) data
{
	NSString * pageName = [NSString stringWithUTF8String:data->nomPages[posData]];
	
	NSArray * array;
	NSMutableDictionary * dict = data->PDFArrayForNames;
	if(dict == nil || (array = [dict objectForKey:pageName]))
	{
		//Enable us to support encrypted PDF cheapily
		NSData * pdfData = [self getPage:posData :data];
		if(pdfData == nil)
			return nil;
		
		PDFDocument * PDF = [[PDFDocument alloc] initWithData:pdfData];
		if(PDF == nil)
			return nil;
		
		array = [PDF getPages];
		if(array == nil || [array count] == 0)
			return nil;
		
		if(dict == nil)
		{
			dict = [NSMutableDictionary new];
			AntiARCRetain(dict);
			data->PDFArrayForNames = dict;
		}
		
		[dict setObject:array forKey:pageName];
	}
	
	if(data->nameID[posData] >= [array count])
		return nil;
	
	return [[array objectAtIndex:data->nameID[posData]] dataRepresentation];
}

#pragma mark - Cache generation

- (void) buildCache : (NSNumber *) session
{
	_cacheBeingBuilt = YES;
	
	if(mainScroller == nil || _data.pageCourante >= _data.nbPage)	//Données hors de nos bornes
	{
		_cacheBeingBuilt = NO;
		return;
	}
	
	@autoreleasepool
	{
		NSArray * data = nil;
		
		[self _buildCache : [session unsignedIntValue] : &data];

		_cacheBeingBuilt = NO;
		
		[CATransaction begin];
		[CATransaction setDisableActions:YES];
	}
	[CATransaction commit];
}

- (void) _buildCache : (uint) session : (NSArray **) data
{
	if(session == cacheSession)
		workingCacheSession = session;
	
	while(session == cacheSession)	//While the active chapter is still the same
	{
		MUTEX_LOCK(cacheMutex);
		* data = mainScroller.arrangedObjects;
		MUTEX_UNLOCK(cacheMutex);
		
		//Page courante
		if(![self entryValid : *data : _data.pageCourante + 1])
		{
			[self loadPageCache: _data.pageCourante : session];
		}
		
		//Encore de la place dans le cache
		else if([self nbEntryRemaining : *data])
		{
			int move = previousMove == READER_ETAT_PREVPAGE ? -1 : 1, i;	//Next page by default
			uint max = _data.nbPage;
			int64_t basePage = _data.pageCourante;
			
			//_data.pageCourante + i * move is unsigned, so it should work just fine
			for(i = 1; i <= 5 && (move == 1 || basePage > abs(i * move)) && basePage + i * move < max; i++)
			{
				if(![self entryValid : *data : basePage + 1 + i * move])
				{
					[self loadPageCache:basePage + i * move :session];
					move = 0;
					break;
				}
			}
			
			if(!move)		//If we found something, we go back to the begining of the loop
				continue;
			
			else if(i != 5)	//We hit the max
			{
				if([self loadAdjacentChapter : move == 1 : *data : session])
					continue;
			}
			
			//We cache the previous page, in the case the user want to go back
			//First, we check if we are in the general case
			if((move == -1 || basePage > move) && basePage - move < max)
			{
				if(![self entryValid : *data :basePage + 1 - move])
				{
					[self loadPageCache:basePage - move : session];
					continue;
				}
			}
			else	//We are at the begining/end of the chapter
			{
				if([self loadAdjacentChapter : move == -1 : *data : session])
					continue;
			}
			
			//Ok then, we cache everythin after
			for (basePage++; basePage < max; basePage++)
			{
				if(![self entryValid : *data : basePage + 1])
				{
					[self loadPageCache : basePage :session];
					break;
				}
			}
			
			if(basePage == max)	//Nothing else to load
				break;
		}
		else
			break;
	}
}

- (BOOL) loadAdjacentChapter : (BOOL) loadNext : (NSArray *) data : (uint) currentSession
{
	uint nextElement, nextElementPos = _posElemInStructure;
	
	MUTEX_LOCK(cacheMutex);
	//Check if next CT is readable
	if(changeChapter(&_project, self.isTome, &nextElement, &nextElementPos, loadNext))
	{
		MUTEX_UNLOCK(cacheMutex);
		
		DATA_LECTURE newData = loadNext ? _nextData : _previousData;

		//Load next CT data
		if((loadNext && nextDataLoaded) || (!loadNext && previousDataLoaded) || readerConfigFileLoader(_project, self.isTome, nextElement, &newData))
		{
			if(loadNext && ![self entryValid : data : _data.nbPage + 1])
			{
				[self loadPageCache : 0 : &newData : currentSession : _data.nbPage + 1];

				if(!nextDataLoaded && currentSession == cacheSession)
				{
					_nextData = newData;
					nextDataLoaded = YES;
				}

				return YES;
			}
			else if(!loadNext && ![self entryValid : data : 0])
			{
				[self loadPageCache : newData.nbPage - 1 : &newData : currentSession : 0];
				
				if(!previousDataLoaded && currentSession == cacheSession)
				{
					_previousData = newData;
					previousDataLoaded = YES;
				}
				
				return YES;
			}
		}
	}
	else
		MUTEX_UNLOCK(cacheMutex);
	
	return NO;
}

#define NB_ELEM_MAX_IN_CACHE 30			//5 behind, current, 24 ahead

- (uint) nbEntryRemaining : (NSArray *) data
{
	uint nbElemCounted = 0, count = MIN([data count], NB_ELEM_MAX_IN_CACHE);
	
	for(id object in data)
	{
		if([object class] == [RakPageScrollView class])
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
	if(index >= [data count])
		return NO;
	
	Class class = [data[[mainScroller getPatchedPosForIndex:index]] class];
	
	return class == [RakPageScrollView class] || class == [RakImageView class];
}

- (void) optimizeCache : (NSMutableArray *) data
{
	uint curPage = _data.pageCourante + 1, objectPage, validFound = 0, invalidFound = 0;
	
	@autoreleasepool {
		
		NSMutableArray * internalData, *freeList = [NSMutableArray array];
		RakPageScrollView* object;
		
		MUTEX_LOCK(cacheMutex);

		if(data == nil)
			internalData = [NSMutableArray arrayWithArray:mainScroller.arrangedObjects];
		else
			internalData = data;
		
		for(uint pos = 0, relativePos, max = [internalData count]; pos < max; pos++)
		{
			relativePos = [mainScroller getPatchedPosForIndex:pos];
			object = [internalData objectAtIndex:relativePos];
			
			if([object class] == [RakPageScrollView class])
			{
				objectPage = object.page;
				
				if(objectPage < MAX(curPage, 5U) - 5 ||	//Too far behind
				   objectPage > curPage + 24)			//Too far ahead
				{
					[freeList addObject:object];
					[internalData replaceObjectAtIndex:relativePos withObject:@(relativePos)];
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
		
		[CATransaction begin];
		[CATransaction setDisableActions:YES];
	}
	[CATransaction commit];
	
	if(validFound != NB_ELEM_MAX_IN_CACHE && (!_cacheBeingBuilt || workingCacheSession != cacheSession))
	{
		uint cacheCode = ++cacheSession;
		dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
			[self buildCache:@(cacheCode)];
		});
	}
}

- (BOOL) loadPageCache : (uint) page : (uint) currentSession
{
	return [self loadPageCache: page : &_data : currentSession : page + 1];
}

- (BOOL) loadPageCache : (uint) page : (DATA_LECTURE*) dataLecture : (uint) currentSession : (uint) position
{
	BOOL retValue;
	
	@autoreleasepool
	{
		retValue = [self _loadPageCache: page : dataLecture : currentSession : position];
		
		[CATransaction begin];
		[CATransaction setDisableActions:YES];
	}
	
	[CATransaction commit];
	
	return retValue;

}

- (BOOL) _loadPageCache : (uint) page : (DATA_LECTURE*) dataLecture : (uint) currentSession : (uint) position
{
	RakPageScrollView *view = [self getScrollView : page : dataLecture];
	
	if(view == nil)			//Loading failure
	{
		[self failure : position];
		return NO;
	}
	
	if(currentSession != cacheSession)	//Didn't changed of chapter since the begining of the loading
		return NO;
	
	__block BOOL retValue = YES;
	
	dispatch_sync(dispatch_get_main_queue(), ^{
		[self updatePCState : position : currentSession : view];
	});
	
	if(&_data == dataLecture && page == dataLecture->pageCourante)		//If current page, we update the main scrollview pointer (click management)
	{
		if(saveMagnification)
		{
			if(_scrollView != nil && [_scrollView class] == [RakPageScrollView class])
				view.magnification = lastKnownMagnification = _scrollView.magnification;
			else
				view.magnification = lastKnownMagnification;
		}

		_scrollView = view;
		
		dispatch_async(dispatch_get_main_queue(), ^{
			[self commitSliderPosIfNeeded];
		});
	}
	
	return retValue;
}

#pragma mark - NSPageController interface

- (void) updatePCState : (uint) page : (uint) currentCacheSession : (NSView *) view
{
	MUTEX_LOCK(cacheMutex);
	
	NSMutableArray * data = [NSMutableArray arrayWithArray:mainScroller.arrangedObjects];
	
	if(page < [data count] && currentCacheSession == cacheSession)
	{
		[data replaceObjectAtIndex:[mainScroller getPatchedPosForIndex:page] withObject:view];
		mainScroller.arrangedObjects = data;
	}
	
	MUTEX_UNLOCK(cacheMutex);
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

- (void) pageController : (RakPageController *) pageController prepareViewController : (NSViewController *) viewController withObject : (RakPageScrollView*) object
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
		BOOL needEmptyView = NO;
		
		//Detect when we get before the first page of the first chapter/last page of the last chapter
		if([object isKindOfClass:[NSNumber class]])
		{
			int64_t value = [(NSNumber *) object longLongValue];
			
			if((value == -1 && _posElemInStructure == 0) || 	//First page
			   (value == -2 && !changeChapterAllowed(&_project, self.isTome, _posElemInStructure + 1)))	//Last page
			{
				needEmptyView = YES;
			}
		}
		
		//Somehow, the cache isn't running
		if(!self.initWithNoContent && !_flushingCache && (!_cacheBeingBuilt || workingCacheSession != cacheSession))
		{
			uint cacheCode = ++cacheSession;
			dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
				[self buildCache:@(cacheCode)];
			});
		}
		
		NSImage * imagePlaceholder = needEmptyView ? nil : (!self.initWithNoContent ? loadingPlaceholder : loadingFailedPlaceholder);
		
		RakImageView * placeholder = [[RakImageView alloc] initWithFrame:NSMakeRect(0, 0, imagePlaceholder.size.width, imagePlaceholder.size.height)];
		[placeholder setImage:imagePlaceholder];
		
		if([object isKindOfClass:[NSNumber class]])
			placeholder.page = [pageController getPatchedPosForIndex:[(NSNumber *) object unsignedIntValue]];
		else
			placeholder.page = UINT_MAX;
		
		[viewController.view addSubview : placeholder];
		
		[placeholder setFrameOrigin: NSCenterPoint(viewController.view.bounds, placeholder.bounds)];
		
		if(object != nil)
			[placeholder startAnimation];
	}
	else if([object class] == [RakPageScrollView class])
	{
		[self initialPositionning : object];
		
		[object setFrame:container.bounds];
		
		if(!_endingTransition)
		{
			if(saveMagnification)
			{
				if(_scrollView != nil && [_scrollView class] == [RakPageScrollView class])
					lastKnownMagnification = _scrollView.magnification;
				
				object.magnification = lastKnownMagnification;
			}
			else
				object.magnification = 1;
		}
		
		if(object.page != _data.pageCourante)
			[object scrollToBeginningOfDocument];
		
		[viewController.view addSubview: object];
		viewController.representedObject = object;
	}
	else
	{
		[viewController.view addSubview : object];
	}
}

#if 0

- (NSRect) pageController : (NSPageController *) pageController frameForObject : (RakPageScrollView*) object
{
	if(object == nil || [object class] != [RakPageScrollView class])
	{
		NSRect frame;
		
		if([object class] == [RakImageView class])
			frame.size = object.frame.size;
		else
			frame.size = (!self.initWithNoContent ? loadingPlaceholder : loadingFailedPlaceholder).size;
		
		frame.origin.x = container.frame.size.width / 2 - frame.size.width / 2;
		frame.origin.y = container.frame.size.height / 2 - frame.size.height / 2;
		
		return frame;
	}
	
	return container.frame;
}

#endif

- (void) pageController : (RakPageController *) pageController didTransitionToObject : (RakPageScrollView *) object
{
	if(object == nil || _flushingCache)
		return;
	
	//We are to an adjacent chapter page
	uint index = pageController.patchedSelectedIndex;
	
	if(index == 0)
	{
		[self changeChapter : NO : YES];
	}
	else if(index == _data.nbPage + 1)
	{
		[self changeChapter : YES : YES];
	}
	else
	{
		uint requestedPage;
		
		if([object superclass] == [NSNumber class])
			requestedPage = [(NSNumber*) object longLongValue];
		else
			requestedPage = object.page;
		
		if(requestedPage != _data.pageCourante && !self.preventRecursion)
		{
			[self changePage : requestedPage > _data.pageCourante ? READER_ETAT_NEXTPAGE : READER_ETAT_PREVPAGE];
		}
	}
}

- (void)pageControllerDidEndLiveTransition : (RakPageController *) pageController
{
	_endingTransition = YES;
	
	if(saveMagnification && _scrollView != nil && [_scrollView class] == [RakPageScrollView class])
		lastKnownMagnification = _scrollView.magnification;
	
	[pageController completeTransition];
	
	_endingTransition = NO;
	
	//Before the first page
	if(pageController.patchedSelectedIndex == 0 && _posElemInStructure == 0)
		pageController.patchedSelectedIndex = 1;
	
	_scrollView.magnification = saveMagnification ? lastKnownMagnification : 1;

	//After the last page
	if(((NSUInteger) pageController.patchedSelectedIndex) == [pageController.arrangedObjects count] - 1 && _posElemInStructure == (self.isTome ? _project.nbVolumesInstalled : _project.nbChapterInstalled) - 1 && [pageController.arrangedObjects count] > 2)
	{
		pageController.patchedSelectedIndex = (int64_t) [pageController.arrangedObjects count] - 2;
		
#ifdef LEAVE_DISTRACTION_FREE_AT_END
		if(self.distractionFree)
			[self switchDistractionFree];
#endif
	}
}

- (void) updateProjectReadingOrder
{
	BOOL flipped = !overrideDirection && _project.rightToLeft;
	
	mainScroller.flipped = flipped;
	mainScroller.transitionStyle = flipped ? NSPageControllerTransitionStyleStackHistory : NSPageControllerTransitionStyleStackBook;
}

#pragma mark - Checks if new elements to download

- (void) checkIfNewElements
{
	if(_alreadyRefreshed)
		return;
	else
		_alreadyRefreshed = YES;
	
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
	uint * selection = calloc(nbElemToGrab, sizeof(uint));
	MDL * tabMDL = [(RakAppDelegate*) [NSApp delegate]MDL];
	
	if(selection == NULL || tabMDL == nil)
	{
		free(selection);
		return;
	}
	
	if(!self.isTome)
	{
		for(nbElemToGrab = localProject.nbChapter - nbElemToGrab; nbElemToGrab < localProject.nbChapter; nbElemToGrab++)
		{
			if(![tabMDL proxyCheckForCollision :localProject : self.isTome :localProject.chaptersFull[nbElemToGrab]])
				selection[nbElemValidated++] = localProject.chaptersFull[nbElemToGrab];
		}
	}
	else
	{
		for(nbElemToGrab = localProject.nbVolumes - nbElemToGrab; nbElemToGrab < localProject.nbVolumes; nbElemToGrab++)
		{
			if(![tabMDL proxyCheckForCollision :localProject : self.isTome :localProject.volumesFull[nbElemToGrab].ID])
				selection[nbElemValidated++] = localProject.volumesFull[nbElemToGrab].ID;
		}
	}
	
	if(self.mainThread == TAB_READER)
		newStuffsQuery = [[RakReaderControllerUIQuery alloc] initWithData : tabMDL : _project :self.isTome :selection :nbElemValidated];
	else
	{
		_queryArrayData = selection;
		_queryArraySize = nbElemValidated;
		queryHidden = YES;
	}
}

#pragma mark - Display suggestions when done reading stuffs

- (BOOL) shouldDisplaySuggestions
{
	uint posInList = 0, nbElem = self.isTome ? _project.nbVolumes : _project.nbChapter;
	
	if(self.isTome)
		while(posInList < _project.nbVolumes && _project.volumesFull[posInList++].ID != _currentElem);
	else
		while(posInList < _project.nbChapter && _project.chaptersFull[posInList++] != _currentElem);
	
	MDL * tabMDL = [[NSApp delegate] MDL];
	for(; posInList < nbElem; posInList++)
	{
		//Shouldn't display if a following chapter is pending download
		if([tabMDL proxyCheckForCollision :_project : self.isTome :ACCESS_DATA(self.isTome, _project.chaptersFull[posInList], _project.volumesFull[posInList].ID)])
			return NO;
	}

	return YES;
}

- (void) displaySuggestions
{
	if(![self shouldDisplaySuggestions])
		return;
	
	[bottomBar displaySuggestionsForProject:_project withOldDFState:oldDFState];
}

#pragma mark - Quit

- (void) flushCache
{
	cacheSession++;		//tell the cache to stop
	oldDFState = NO;
	
	if(_project.isInitialized)
		insertCurrentState(_project, [self exportContext]);
	
	if(mainScroller != nil)
	{
		MUTEX_LOCK(cacheMutex);
		
		_flushingCache = YES;

		NSMutableArray * array = [NSMutableArray arrayWithArray:mainScroller.arrangedObjects];
		
		[array removeAllObjects];
		[array insertObject:@(0) atIndex:0];
		
		_scrollView = nil;
		
		mainScroller.patchedSelectedIndex = 0;
		mainScroller.arrangedObjects = array;
		
		_flushingCache = NO;
		
		MUTEX_UNLOCK(cacheMutex);
	}
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

