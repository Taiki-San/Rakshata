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

	if(_currentContext == TAB_READER && project.nombreChapitreInstalled == 0 && project.nombreTomesInstalled == 0)
		return nil;
	
	self = [super initWithFrame : [self frameFromParent : parentBounds : headerHeight]];
	if (self != nil)
	{
		self.autoresizesSubviews = NO;
		
		data = getCopyOfProjectData(project);
		
		if(![self setupButtons:&isTome])
			return nil;
		
		RakCTSelectionList * view;
		
		if(data.nombreChapitreInstalled > 0)
		{
			view = [[RakCTSelectionList alloc] init : self.frame : data :false : context[0] : context[1]];
			if(view != nil)
			{
				_chapterView = [[RakCTSelectionListContainer alloc] initWithFrame : self.bounds : _currentContext != TAB_CT : view];
				if(_chapterView != nil)
				{
					_chapterView.hidden = isTome;
					[self addSubview : _chapterView];
				}
				
				view = nil;
			}
		}
		
		if(data.nombreTomesInstalled > 0)
		{
			view = [[RakCTSelectionList alloc] init : self.frame : data : true : context[2] : context[3]];
			if(view != nil)
			{
				_volView = [[RakCTSelectionListContainer alloc] initWithFrame : self.bounds : _currentContext != TAB_CT : view];
				if(_volView != nil)
				{
					_volView.hidden = !isTome;
					[self addSubview: _volView];
				}

				view = nil;
			}
		}
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
	if(data.team == NULL)
		return nil;
	
	return [NSString stringWithFormat:@"%s\n%d\n%d\n%ld\n%.0f\n%ld\n%.0f", data.team->URLRepo, data.projectID, [_buttons selectedSegment] == 1 ? 1 : 0, (long)[_chapterView getSelectedElement], [_chapterView getSliderPos], (long)[_volView getSelectedElement], [_volView getSliderPos]];
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

- (BOOL) setupButtons : (BOOL*) isTome
{
	_buttons = [[RakCTCoreViewButtons alloc] initWithFrame:[self bounds]];
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
		else	//Projet illisible
		{
			[self failure];
			return NO;
		}
	}
	
	if(_currentContext != TAB_READER)
		[_buttons setHidden : YES];
	
	[self addSubview:_buttons];
	return YES;
}

#pragma mark - Properties

- (PROJECT_DATA) currentProject
{
	return data;
}

- (void) setCurrentContext : (uint) currentContext
{
	_currentContext = currentContext;
	
	if(currentContext == TAB_READER)
	{
		if(_buttons != nil && _buttons.isHidden)
			[_buttons setHidden:NO];
	}
	else
	{
		if(_buttons != nil && !_buttons.isHidden)
			[_buttons setHidden:YES];
	}
	
	if(_chapterView != nil)
		_chapterView.compactMode = currentContext != TAB_CT;
	
	if(_volView != nil)
		_volView.compactMode = currentContext != TAB_CT;
}

- (uint) currentContext
{
	return _currentContext;
}

#pragma mark - Proxy

- (void) gotClickedTransmitData : (bool) isTome : (uint) index
{
	if(self.dontNotify)
		return;
	
	int ID;
	
	if(isTome && index < data.nombreTomesInstalled)
		ID = data.tomesInstalled[index].ID;
	else if(!isTome && index < data.nombreChapitreInstalled)
		ID = data.chapitresInstalled[index];
	else
		return;
	
	self.dontNotify = YES;
	
	[RakTabView broadcastUpdateContext:self :data :isTome :ID];
	
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

- (void) refreshCTData : (BOOL) checkIfRequired : (uint) ID;
{
	if((checkIfRequired || data.cacheDBID != ID) && (!checkIfRequired || !updateIfRequired(&data, RDB_CTXCT)))
		return;
	
	getUpdatedChapterList(&data, true);
	[_chapterView reloadData : data : data.nombreChapitreInstalled : data.chapitresInstalled : NO];
	
	getUpdatedTomeList(&data, true);
	[_volView reloadData : data : data.nombreTomesInstalled : data.tomesInstalled : NO];
}

- (void) selectElem : (uint) projectID : (BOOL) isTome : (int) element
{
	if(data.cacheDBID != projectID)
		return;
	
	RakCTSelectionListContainer * tab = nil;
	
	if(isTome)
		tab = _volView;
	else if(!isTome)
		tab = _chapterView;
	
	if(tab != nil)
	{
		int row = [tab getIndexOfElement:element];
		[tab selectRow:row];
		[tab jumpScrollerToRow:row];
	}
}

- (BOOL) updateContext : (PROJECT_DATA) newData
{
	//Some danger of TOCTOU around here, mutexes would be great
	
	if(data.cacheDBID == newData.cacheDBID)
	{
		getUpdatedCTList(&data, true);
		getUpdatedCTList(&data, false);
	}
	else
	{
		if(_chapterView != NULL)	[_chapterView resetSelection:nil];
		if(_volView != NULL)	[_volView resetSelection:nil];
		
		releaseCTData(data);
		data = getCopyOfProjectData(newData);
	}
	
	if(!updateIfRequired(&data, RDB_CTXCT))
	{
		getUpdatedChapterList(&data, true);
		getUpdatedTomeList(&data, true);
	}
	
	//No data available
	if(data.nombreChapitreInstalled == 0 && data.nombreTomesInstalled == 0)
	{
		_chapterView.hidden = YES;
		_volView.hidden = YES;
		
		[self failure];
		return NO;
	}
	
	//Update views, create them if required
	RakCTSelectionList * view;
	
	if(data.nombreChapitreInstalled)
	{
		if(_chapterView == nil)
		{
			view = [[RakCTSelectionList alloc] init:self.frame : data : false : -1 : -1];
			if(view != nil)
			{
				_chapterView = [[RakCTSelectionListContainer alloc] initWithFrame : self.bounds : _currentContext != TAB_CT : view];
				if(_chapterView != nil)
					[self addSubview : _chapterView];
				
				view = nil;
			}
		}
		else
			[_chapterView reloadData : data : data.nombreChapitreInstalled : data.chapitresInstalled : YES];
		
		[_buttons setEnabled:YES forSegment:0];
	}
	else
	{
		[_buttons setEnabled:NO forSegment:0];
	}
	
	if(data.nombreTomesInstalled)
	{
		if(_volView == nil)
		{
			view = [[RakCTSelectionList alloc] init:self.frame : data : true : -1 : -1];
			if(view != nil)
			{
				_volView = [[RakCTSelectionListContainer alloc] initWithFrame : self.bounds : _currentContext != TAB_CT : view];
				if(_volView != nil)
				{
					[self addSubview: _volView];
				}
				
				view = nil;
			}
		}
		else
			[_volView reloadData : data : data.nombreTomesInstalled : data.tomesInstalled : YES];
		
		[_buttons setEnabled:YES forSegment:1];
	}
	else
	{
		[_buttons setEnabled:NO forSegment:1];
	}
	
	BOOL isTome = [_buttons selectedSegment] == 1;
	
	[_chapterView setHidden:isTome];
	[_volView setHidden:!isTome];
	
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
		if(![_chapterView isHidden])
			[_chapterView setHidden:YES];
		[_buttons setSelectedSegment:1];
	}
	
	return YES;
}

@end