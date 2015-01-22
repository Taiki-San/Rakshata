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

@implementation RakCTSelection

- (instancetype) initWithProject : (PROJECT_DATA) project : (BOOL) isTome : (NSRect) parentBounds : (CGFloat) headerHeight : (long [4]) context
{
	[Prefs getPref : PREFS_GET_MAIN_THREAD : &_currentContext];

	self = [super initWithFrame : [self frameFromParent : parentBounds : headerHeight]];
	if (self != nil)
	{
		self.autoresizesSubviews = NO;
		[RakDBUpdate registerForUpdate:self :@selector(DBUpdated:)];
		data = getCopyOfProjectData(project);
		
		[self setupButtons:&isTome];
		
		BOOL isCompact = _currentContext != TAB_CT;
		
		//We create views even if there is no content for them
		RakCTSelectionList * view = [[RakCTSelectionList alloc] initWithFrame:self.bounds isCompact:isCompact projectData:data isTome:false selection:context[0] scrollerPos:context[1]];
		if(view != nil)
		{
			_chapterView = [[RakCTSelectionListContainer alloc] initWithFrame : self.bounds : isCompact : view];
			if(_chapterView != nil)
			{
				_chapterView.hidden = isTome && isCompact;
				[self addSubview : _chapterView];
			}
			
			view = nil;
		}
		
		view = [[RakCTSelectionList alloc] initWithFrame:self.bounds isCompact:isCompact projectData:data isTome:true selection:context[2] scrollerPos:context[3]];
		if(view != nil)
		{
			_volView = [[RakCTSelectionListContainer alloc] initWithFrame : self.bounds : isCompact : view];
			if(_volView != nil)
			{
				_volView.hidden = !isTome && isCompact;
				[self addSubview: _volView];
			}
			
			view = nil;
		}
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(gotClickedTransmitData:) name:@"RakCTSelectedManually" object:nil];
	}
	
	return self;
}

- (void) failure
{
	NSLog(@"Got crappy data D:");
	[_buttons removeFromSuperview];
	
	if(self.superview != nil)
		[self removeFromSuperview];
}

- (NSString *) getContextToGTFO
{
	if(data.repo == NULL)
		return nil;
	
	return [NSString stringWithFormat:@"%s\n%d\n%d\n%u\n%.0f\n%u\n%.0f", data.repo->URL, data.projectID, [_buttons selectedSegment] == 1 ? 1 : 0, (uint)[_chapterView getSelectedElement], [_chapterView getSliderPos], (uint)[_volView getSelectedElement], [_volView getSliderPos]];
}

- (void) dealloc
{
	[_buttons removeFromSuperview];
	
	[[_chapterView getContent] removeFromSuperviewWithoutNeedingDisplay];
	
	[[_volView getContent ] removeFromSuperviewWithoutNeedingDisplay];
	
	releaseCTData(data);
}

#pragma mark - Size management

- (void) setFrame : (NSRect) parentFrame : (CGFloat) headerHeight
{
	_cachedHeaderHeight = headerHeight;
	
	[super setFrame : [self frameFromParent : parentFrame : headerHeight]];

	[_buttons setFrame : self.bounds];
	[_chapterView setFrame : self.bounds];
	[_volView setFrame : self.bounds];
}

- (void) resizeAnimation : (NSRect) parentFrame : (CGFloat) headerHeight
{
	_cachedHeaderHeight = headerHeight;

	NSRect frame = [self frameFromParent : parentFrame : headerHeight];
	[self.animator setFrame:frame];
	
	frame.origin = NSZeroPoint;
	
	[_buttons resizeAnimation : frame];
	[_chapterView resizeAnimation : frame];
	[_volView resizeAnimation : frame];
}

- (void) setFrame:(NSRect)frameRect
{
#ifdef DEV_VERSION
	NSLog(@"Shouldn't have been called");
#endif
	
	[self setFrame : frameRect : _cachedHeaderHeight];
}

- (NSRect) frameFromParent : (NSRect) parentBounds : (CGFloat) headerHeight
{
	if(_currentContext == TAB_READER)
		parentBounds.size.height -= CT_READERMODE_HEIGHT_HEADER_TAB;
	else
		parentBounds.size.height -= headerHeight;
	
	return parentBounds;
}

#pragma mark - Buttons management

- (void) setupButtons : (BOOL*) isTome
{
	_buttons = [[RakCTCoreViewButtons alloc] initWithFrame:self.bounds];

	if(data.nombreChapitreInstalled > 0)
	{
		[_buttons setEnabled : YES forSegment:0];
		
		if(!*isTome)
			[_buttons setSelected : YES forSegment:0];
		
		if(data.nombreChapitreInstalled == 1)
		{
			NSString * name = [_buttons labelForSegment:0];
			[_buttons setLabel:[name substringToIndex:[name length] - 1] forSegment:0];
		}
	}
	else if(!*isTome)	//Si on recoit une demande incohérante
		*isTome = YES;
	
	if(data.nombreTomesInstalled > 0)
	{
		[_buttons setEnabled:YES forSegment:1];
		
		if(*isTome)
			[_buttons setSelected:YES forSegment:1];
		
		if(data.nombreTomesInstalled == 1)
		{
			NSString * name = [_buttons labelForSegment:1];
			[_buttons setLabel:[name substringToIndex:[name length] - 1] forSegment:1];
		}
	}
	else if(*isTome)
	{
		if(data.nombreChapitreInstalled > 0)
		{
			[_buttons setSelected:YES forSegment:0];
			*isTome = NO;
		}
		//Aucun bouton actif jusqu'à que le projet soit changé
	}
	
	if(_currentContext != TAB_READER)
		[_buttons setHidden : YES];
	
	[self addSubview:_buttons];
}

#pragma mark - Properties

- (PROJECT_DATA) currentProject
{
	return data;
}

- (void) setCurrentContext : (uint) currentContext
{
	if(_currentContext == currentContext)
		return;
	
	_currentContext = currentContext;
	BOOL isTome = [_buttons selectedSegment] == 1;
	
	if(currentContext == TAB_READER)
	{
		if(_buttons != nil)
			[_buttons setHidden:NO];
		
		[_chapterView setHidden:isTome];
		[_volView setHidden:!isTome];
	}
	else
	{
		if(_buttons != nil)
			[_buttons setHidden:YES];
	}
	
	if(_chapterView != nil)
	{
		_chapterView.compactMode = currentContext != TAB_CT;
	}
	
	if(_volView != nil)
	{
		_volView.compactMode = currentContext != TAB_CT;
	}
}

- (uint) currentContext
{
	return _currentContext;
}

#pragma mark - DB update

- (void) DBUpdated : (NSNotification*) notification
{
	if([RakDBUpdate analyseNeedUpdateProject:notification.userInfo :data])
	{
		if(![NSThread isMainThread])
		{
			[self performSelectorOnMainThread:@selector(DBUpdated:) withObject:notification waitUntilDone:NO];
			return;
		}
		
		releaseCTData(data);
		PROJECT_DATA newData = getElementByID(data.cacheDBID);
		
		if(newData.isInitialized)
		{
			//We define what changed on the structure, except chapters/volumes
			[(RakChapterView*) self.superview projectDataUpdate:data :newData];
			
			data = newData;
			[_chapterView reloadData : data : NO];
			[_volView reloadData : data : NO];
		}
	}
}


#pragma mark - Proxy

- (void) gotClickedTransmitData : (NSNotification *) notification
{
	if(self.dontNotify)
		return;
	
	int ID;
	NSNumber * _index = [notification.userInfo objectForKey:@"index"], *_isTome = [notification.userInfo objectForKey:@"isTome"], * _installed = [notification.userInfo objectForKey:@"isInstalled"];

	if(_index == nil || _isTome == nil || _installed == nil)
		return;

	uint index = [_index unsignedIntValue];
	BOOL isTome = [_isTome boolValue], installed = [_installed boolValue];
	
	if(_currentContext == TAB_READER)
	{
		if(isTome && index < data.nombreTomesInstalled)
			ID = data.tomesInstalled[index].ID;
		else if(!isTome && index < data.nombreChapitreInstalled)
			ID = data.chapitresInstalled[index];
		else
			return;
	}
	else
	{
		if(isTome && index < data.nombreTomes)
			ID = data.tomesFull[index].ID;
		else if(!isTome && index < data.nombreChapitre)
			ID = data.chapitresFull[index];
		else
			return;
	}
	
	self.dontNotify = YES;
	
	if(_currentContext != TAB_READER)	//We have to update the context in the case we moved to compact mode
	{
		if(isTome)
		{
			_buttons.selectedSegment = 1;
			[_chapterView resetSelection];
		}
		else
		{
			_buttons.selectedSegment = 0;
			[_volView resetSelection];
		}
	}

	if(installed)
	{
		_chapterView.compactMode = YES;
		_volView.compactMode = YES;
		[RakTabView broadcastUpdateContext:self :data :isTome :ID];
	}
	else
	{
		[[[NSApp delegate] MDL] proxyAddElement:data isTome:isTome element:ID partOfBatch:NO];
		[_volView resizeAnimation : self.bounds];	//If MDL is here to stay, we have to update the size of the view below
	}
	
	self.dontNotify = NO;
}

- (void) feedAnimationController : (RakCTAnimationController *) animationController
{
	[animationController addCTContent: _chapterView : _volView];
	[animationController addAction : self : @selector(switchIsTome:)];
}

- (void) switchIsTome : (RakCTCoreViewButtons*) sender
{
	bool isTome;
	if ([sender selectedSegment] == 0)
		isTome = false;
	else
		isTome = true;
	
	if(_chapterView != nil)
		_chapterView.hidden = isTome;
	if(_volView != nil)
		_volView.hidden = !isTome;
}

- (void) selectElem : (uint) projectID : (BOOL) isTome : (int) element
{
	if(data.cacheDBID != projectID || self.dontNotify)
		return;
	
	RakCTSelectionListContainer * tab = nil;
	
	if(isTome)
		tab = _volView;
	else if(!isTome)
		tab = _chapterView;
	
	if(tab != nil)
	{
		uint row = [tab getIndexOfElement:element];
		[tab selectIndex:element];
		[tab jumpScrollerToRow:row];
	}
}

- (BOOL) updateContext : (PROJECT_DATA) newData
{
	if(data.cacheDBID == newData.cacheDBID)
	{
		getUpdatedCTList(&data, true);
		getUpdatedCTList(&data, false);
	}
	else
	{
		if(_chapterView != NULL)	[_chapterView resetSelection];
		if(_volView != NULL)	[_volView resetSelection];
		
		free(data.tomesFull);		data.tomesFull = NULL;
		free(data.tomesInstalled);	data.tomesInstalled = NULL;
		releaseCTData(data);
		data = getCopyOfProjectData(newData);
	}
	
	//Update views
	BOOL isCompact = _currentContext != TAB_CT, isTome = [_buttons selectedSegment] == 1;
	
	[_chapterView reloadData : data : YES];
	[_volView reloadData : data : YES];
	
	[_buttons setEnabled:(newData.nombreChapitreInstalled > 0) forSegment:0];
	[_buttons setEnabled:(newData.nombreTomesInstalled > 0) forSegment:1];

	if(isCompact)
	{
		//Update focus
		if(isTome && data.nombreTomesInstalled == 0)
		{
			[_chapterView setHidden:NO];
			[_volView setHidden:YES];
			[_buttons setSelectedSegment:0];
		}
		else if(!isTome && data.nombreChapitreInstalled == 0)
		{
			[_volView setHidden:NO];
			[_chapterView setHidden:YES];
			[_buttons setSelectedSegment:1];
		}
	}
	else
	{
		[_chapterView setHidden:NO];
		[_volView setHidden:NO];
	}

	return YES;
}

@end