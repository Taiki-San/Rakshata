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

- (instancetype) initWithProject : (PROJECT_DATA) project : (BOOL) isTome : (NSRect) parentBounds : (CGFloat) headerHeight : (long [4]) context : (uint) mainThread
{
	[Prefs getPref : PREFS_GET_MAIN_THREAD : &_currentContext];
	
	self = [super initWithFrame : [self frameFromParent : parentBounds : headerHeight]];
	if(self != nil)
	{
		self.autoresizesSubviews = NO;

		[RakDBUpdate registerForUpdate:self :@selector(DBUpdated:)];
		[Prefs registerForChange:self forType:KVO_THEME];
		
		data = getCopyOfProjectData(project);
		
		[self setupButtons:&isTome];
		
		BOOL isCompact = _currentContext == TAB_READER;
		
		//We create views even if there is no content for them
		RakCTSelectionList * view = [[RakCTSelectionList alloc] initWithFrame:self.bounds isCompact:isCompact projectData:data isTome:NO selection:context[0] scrollerPos:context[1]];
		if(view != nil)
		{
			_chapterView = [[RakCTSelectionListContainer alloc] initWithFrame : self.bounds : isCompact : view];
			if(_chapterView != nil)
			{
				if(mainThread == TAB_SERIES || (isTome && isCompact))
				{
					_chapterView.hidden = YES;
					_chapterView.alphaValue = 0;
				}
				
				[self addSubview : _chapterView];
			}
			
			view = nil;
		}
		
		view = [[RakCTSelectionList alloc] initWithFrame:self.bounds isCompact:isCompact projectData:data isTome:YES selection:context[2] scrollerPos:context[3]];
		if(view != nil)
		{
			_volView = [[RakCTSelectionListContainer alloc] initWithFrame : self.bounds : isCompact : view];
			if(_volView != nil)
			{
				if(mainThread == TAB_SERIES || (!isTome && isCompact))
				{
					_volView.hidden = YES;
					_volView.alphaValue = 0;
				}
				
				[self addSubview: _volView];
			}
			
			view = nil;
		}
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(gotClickedTransmitData:) name:CT_CLIC_NOTIFICATION object:nil];
		
		[self updateTitle];
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
	if(!data.isInitialized)
		return nil;
	
	return [NSString stringWithFormat:@"%llu\n%d\n%d\n%d\n%u\n%.0f\n%u\n%.0f", getRepoID(data.repo), data.projectID, data.locale, [_buttons selectedSegment] == 1 ? 1 : 0, [_chapterView getSelectedElement], [_chapterView getSliderPos], [_volView getSelectedElement], [_volView getSliderPos]];
}

- (void) dealloc
{
	[_buttons removeFromSuperview];
	
	[[_chapterView getContent] removeFromSuperviewWithoutNeedingDisplay];
	
	[[_volView getContent] removeFromSuperviewWithoutNeedingDisplay];
	
	releaseCTData(data);
	
	[Prefs deRegisterForChange:self forType:KVO_THEME];
	[RakDBUpdate unRegister : self];
	[[NSNotificationCenter defaultCenter] removeObserver:self];
}

#pragma mark - Size management

- (void) setFrame : (NSRect) parentFrame : (CGFloat) headerHeight
{
	_cachedHeaderHeight = headerHeight;
	
	[super setFrame : [self frameFromParent : parentFrame : headerHeight]];
	
	[_buttons setFrame : _bounds];
	[_chapterView setFrame : _bounds];
	[_volView setFrame : _bounds];
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

- (NSRect) frameFromParent : (NSRect) parentBounds : (CGFloat) headerHeight
{
	if(_currentContext == TAB_READER)
		parentBounds.size.height -= CT_READERMODE_HEIGHT_HEADER_TAB;
	else
		parentBounds.size.height -= headerHeight;
	
	return parentBounds;
}

- (void) drawRect:(NSRect)dirtyRect
{
	if(_currentContext != TAB_READER)
		return [super drawRect:dirtyRect];
	
	[[Prefs getSystemColor:COLOR_CTLIST_BACKGROUND] setFill];
	
	dirtyRect.size = _bounds.size;
	
	NSRectFill(NSMakeRect(0, 0, dirtyRect.size.width, dirtyRect.size.height - CT_READERMODE_HEIGHT_CT_BUTTON - CT_READERMODE_HEIGHT_BORDER_TABLEVIEW + 2));
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	
	[self setNeedsDisplay:YES];
}

#pragma mark - Buttons management

- (void) setupButtons : (BOOL*) isTome
{
	_buttons = [[RakCTCoreViewButtons alloc] initWithFrame : _bounds : @[NSLocalizedString(@"CHAPTERS", nil), NSLocalizedString(@"VOLUMES", nil)]];
	
	if(!data.isInitialized)
	{
		[_buttons setEnabled: NO forSegment:0];
		[_buttons setEnabled: NO forSegment:1];
	}
	else
	{
		if(data.nbChapterInstalled > 0)
		{
			[_buttons setEnabled : YES forSegment:0];
			
			if(!*isTome)
				[_buttons setSelected : YES forSegment:0];
			
			if(data.nbChapterInstalled == 1)
			{
				NSString * name = [_buttons labelForSegment:0];
				[_buttons setLabel:[name substringToIndex:[name length] - 1] forSegment:0];
			}
		}
		else if(!*isTome)	//Si on recoit une demande incohérante
			*isTome = YES;
		
		if(data.nbVolumesInstalled > 0)
		{
			[_buttons setEnabled:YES forSegment:1];
			
			if(*isTome)
				[_buttons setSelected:YES forSegment:1];
			
			if(data.nbVolumesInstalled == 1)
			{
				NSString * name = [_buttons labelForSegment:1];
				[_buttons setLabel:[name substringToIndex:[name length] - 1] forSegment:1];
			}
		}
		else if(*isTome)
		{
			if(data.nbChapterInstalled > 0)
			{
				[_buttons setSelected:YES forSegment:0];
				*isTome = NO;
			}
		}
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
	[self updateTitle];
	
	BOOL isTome = [_buttons selectedSegment] == 1;
	
	if(currentContext == TAB_READER)
	{
		for(NSView * view in @[_buttons, isTome ? _volView : _chapterView])
		{
			view.hidden = NO;
			view.animator.alphaValue = 1;
		}
		
		if(isTome)
			_chapterView.animator.alphaValue = 0;
		else
			_volView.animator.alphaValue = 0;
	}
	else
	{
		if(currentContext == TAB_CT)
		{
			_chapterView.hidden = NO;	_chapterView.animator.alphaValue = 1;
			_volView.hidden = NO;		_volView.animator.alphaValue = 1;
		}
		else
		{
			_chapterView.animator.alphaValue = 0;
			_volView.animator.alphaValue = 0;
		}
		
		_buttons.animator.alphaValue = 0;
	}
	
	if(currentContext != TAB_SERIES)
	{
		_chapterView.compactMode = currentContext == TAB_READER;
		_volView.compactMode = currentContext == TAB_READER;
	}
}

- (void) cleanChangeCurrentContext
{
	for(NSView * view in @[_buttons, _volView, _chapterView])
	{
		if(view.alphaValue == 0)
			view.hidden = YES;
	}
}

#pragma mark - DB update

- (void) DBUpdated : (NSNotification*) notification
{
	if([RakDBUpdate analyseNeedUpdateProject:notification.userInfo :data])
	{
		if(![NSThread isMainThread])
			return [self performSelectorOnMainThread:@selector(DBUpdated:) withObject:notification waitUntilDone:NO];
		
		PROJECT_DATA newData = getProjectByID(data.cacheDBID);
		
		if(newData.isInitialized)
		{
			//We define what changed on the structure, except chapters/volumes
			[(RakChapterView*) self.superview projectDataUpdate:data :newData];
			
			[_chapterView reloadData : newData : NO];
			[_volView reloadData : newData : NO];
			
			releaseCTData(data);
			data = newData;
		}
		else
			[[NSApp delegate] CT].initWithNoContent = YES;
	}
}

#pragma mark - Proxy

- (void) gotClickedTransmitData : (NSNotification *) notification
{
	if(self.dontNotify)
		return;
	
	uint ID;
	NSNumber * _index = [notification.userInfo objectForKey:@"index"], *_isTome = [notification.userInfo objectForKey:@"isTome"], * _installed = [notification.userInfo objectForKey:@"isInstalled"];
	
	if(_index == nil || _isTome == nil || _installed == nil)
		return;
	
	uint index = [_index unsignedIntValue];
	BOOL isTome = [_isTome boolValue], installed = [_installed boolValue];
	
	if(_currentContext == TAB_READER)
	{
		if(isTome && index < data.nbVolumesInstalled)
			ID = data.volumesInstalled[index].ID;
		else if(!isTome && index < data.nbChapterInstalled)
			ID = (uint) data.chaptersInstalled[index];
		else
			return;
	}
	else
	{
		if(isTome && index < data.nbVolumes)
			ID = data.volumesFull[index].ID;
		else if(!isTome && index < data.nbChapter)
			ID = (uint) data.chaptersFull[index];
		else
			return;
	}
	
	self.dontNotify = YES;
	
	if(_currentContext != TAB_READER)	//We have to update the context in the case we moved to compact mode
	{
		if(isTome)
		{
			[_buttons updateSelectionWithoutAnimation:1];
			[_chapterView resetSelection];
		}
		else
		{
			[_buttons updateSelectionWithoutAnimation:0];
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
	}
	
	self.dontNotify = NO;
}

- (void) feedAnimationController : (RakCTAnimationController *) animationController
{
	[animationController addCTContent: _chapterView : _volView];
	[animationController addAction : self];
}

- (void) animationOver : (RakAnimationController *) controller
{
	[self switchIsTome];
}

- (void) switchIsTome
{
	BOOL isTome = [_buttons selectedSegment] != 0;
	
	[_chapterView setWantIsTome : isTome];
	[_volView setWantIsTome : isTome];
}

- (void) selectElem : (uint) projectID : (BOOL) isTome : (uint) element
{
	if(self.dontNotify)
		return;
	
	if(data.cacheDBID != projectID)
	{
		[[[NSApp delegate] CT] updateProject:projectID :isTome :element];
	}
	else
	{
		RakCTSelectionListContainer * tab = isTome ? _volView : _chapterView;
		
		if(tab != nil)
		{
			uint row = [tab getIndexOfElement:element];
			[tab selectIndex:row];
			[tab jumpScrollerToIndex:row];
		}
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
		
		free(data.volumesFull);		data.volumesFull = NULL;
		free(data.volumesInstalled);	data.volumesInstalled = NULL;
		data = getCopyOfProjectData(newData);
		
		[self updateTitle];
	}
	
	//Update views
	BOOL isCompact = _currentContext == TAB_READER, isTome = [_buttons selectedSegment] == 1;
	
	[_chapterView reloadData : data : YES];
	[_volView reloadData : data : YES];
	
	[_buttons setEnabled:(newData.nbChapterInstalled > 0) forSegment:0];
	[_buttons setEnabled:(newData.nbVolumesInstalled > 0) forSegment:1];
	
	if(isCompact)
	{
		//Update focus
		if(isTome && data.nbVolumesInstalled == 0)
		{
			[_chapterView setHidden:NO];
			[_volView setHidden:YES];
			[_buttons setSelectedSegment:0];
		}
		else if(!isTome && data.nbChapterInstalled == 0)
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

- (void) updateTitle
{
	if(_currentContext == TAB_CT)
		[((RakAppDelegate *)[NSApp delegate]).window setProjectTitle:data];
}

@end