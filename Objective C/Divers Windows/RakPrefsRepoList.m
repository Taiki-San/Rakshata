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
	LIST_ROW_HEIGHT = LIST_WIDE_LINE_HEIGHT,
	LIST_ROW_COMPACT_HEIGHT = 25,
	LIST_ROW_COMPACT_WIDTH = 25,
	
	OFFSET_TITLE_Y = 22,
	OFFSET_DETAIL_Y = 6
};

@interface RakPrefsRepoListItemView()
{
	RakClickableText * detail;
	
	BOOL _isCompact;
	BOOL _isDetailColumn;
}

@property BOOL highlighted;

@end

@interface RakPrefsRepoList()
{
	BOOL refreshing;
	
	uint lastTransmittedSelectedRowIndex;
	
	NSTableColumn * _detailColumn;
}

@end

@implementation RakPrefsRepoList

- (instancetype) initWithFrame : (NSRect) frame
{
	self = [self init];
	
	if(self != nil)
	{
		isListOfRepo = YES;
		_nbData = [_responder sizeForMode:_rootMode];
		
		[self applyContext:frame : selectedRowIndex : -1];
		
		if(_detailMode)
		{
			_detailColumn = [[NSTableColumn alloc] init];
			
			if(_detailColumn != nil)
			{
				((NSTableColumn *) [_tableView.tableColumns firstObject]).width -= LIST_ROW_COMPACT_WIDTH;
				_detailColumn.width = LIST_ROW_COMPACT_WIDTH;
				[_tableView addTableColumn:_detailColumn];
			}
		}
		
		lastTransmittedSelectedRowIndex = LIST_INVALID_SELECTION;
		
		scrollView.wantsLayer = YES;
		scrollView.layer.cornerRadius = 3;
		scrollView.drawsBackground = YES;
		scrollView.backgroundColor = [Prefs getSystemColor:COLOR_BACKGROUND_REPO_LIST :nil];
	}
	
	return self;
}

- (void) tableViewSelectionDidChange:(NSNotification *)notification
{
	if(!refreshing)
	{
		if(selectedRowIndex != lastTransmittedSelectedRowIndex)
		{
			lastTransmittedSelectedRowIndex = selectedRowIndex;
			[_responder selectionUpdate:_rootMode :selectedRowIndex];
		}
		else
		{
			[scrollView.nextResponder mouseUp:nil];
		}
	}
}

- (void) resetSelection:(NSTableView *)tableView
{
	lastTransmittedSelectedRowIndex = INVALID_VALUE;
	
	[super resetSelection:tableView];
}

- (Class) contentClass
{
	return [RakPrefsRepoListItemView class];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	
	scrollView.backgroundColor = [Prefs getSystemColor:COLOR_BACKGROUND_REPO_LIST :nil];
	[scrollView setNeedsDisplay:YES];
}

#pragma mark - Switch code

- (void) setRootMode:(BOOL)rootMode
{
	if(rootMode == _rootMode)
		return;
	else if(_tableView == nil)
	{
		_rootMode = rootMode;
		return;
	}
	
	[self reloadContent : rootMode];
}

- (void) reloadContent : (BOOL) rootMode
{
	refreshing = YES;
	
	[_tableView removeRowsAtIndexes:[NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, _nbData)] withAnimation:NSTableViewAnimationSlideLeft];
	
	_rootMode = rootMode;
	_nbData = [_responder sizeForMode:_rootMode];
	
	[_tableView insertRowsAtIndexes:[NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, _nbData)] withAnimation:NSTableViewAnimationSlideLeft];
	
	refreshing = NO;
}

- (void) removeContentAtLine :(uint) line
{
	if(line < _nbData)
	{
		[_tableView removeRowsAtIndexes:[NSIndexSet indexSetWithIndex:line] withAnimation:NSTableViewAnimationSlideLeft];

		_nbData--;
	}
}

- (void) updateContentAtLine :(uint) line
{
	if(line < _nbData)
	{
		refreshing = YES;

		[_tableView removeRowsAtIndexes:[NSIndexSet indexSetWithIndex:line] withAnimation:NSTableViewAnimationSlideLeft];
		[_tableView insertRowsAtIndexes:[NSIndexSet indexSetWithIndex:line] withAnimation:NSTableViewAnimationSlideLeft];

		refreshing = NO;
	}
}

#pragma mark - Tableview code

- (NSInteger) numberOfRowsInTableView : (RakTableView *) tableView
{
	return _nbData;
}

- (CGFloat) tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row
{
	return _detailMode ? LIST_ROW_COMPACT_HEIGHT : LIST_ROW_HEIGHT;
}

- (NSView*) tableView : (RakTableView *) tableView viewForTableColumn : (NSTableColumn*) tableColumn row : (NSInteger) row
{
	void * data = [_responder dataForMode:_rootMode index:row];
	if(data == NULL)
		return nil;
	
	NSString * detail = nil;
	if(!_rootMode && !_detailMode)
		detail = [_responder nameOfParent: ((REPO_DATA *) data)->parentRepoID];
	
	RakPrefsRepoListItemView * result = [tableView makeViewWithIdentifier : _identifier owner:self];
	if(result == nil)
	{
		result = [[RakPrefsRepoListItemView alloc] initWithRepo : _detailMode : tableColumn == _detailColumn : _rootMode : data : detail];
		if(result != nil)
			result.responder = _responder;
	}
	else
		[result updateContent : _detailMode : tableColumn == _detailColumn :_rootMode :data :detail];
	
	return result;
}

- (void) graphicSelection:(RakPrefsRepoListItemView *)view :(BOOL)select
{
	if([view class] == [RakPrefsRepoListItemView class])
	{
		view.highlighted = select;
		[view setNeedsDisplay:YES];
	}
}

@end

@implementation RakPrefsRepoListItemView

- (instancetype) initWithRepo : (BOOL) isCompact : (BOOL) isDetailColumn : (BOOL) isRoot : (void *) repo : (NSString *) detailString
{
	self = [self initWithFrame:NSMakeRect(0, 0, haveFixedWidth ? self.fixedWidth : 300, isCompact ? LIST_ROW_COMPACT_HEIGHT : LIST_ROW_HEIGHT)];
	
	if(self != nil)
	{
		[Prefs getCurrentTheme:self];
		[self updateContent:isCompact :isDetailColumn :isRoot :repo :detailString];
	}
	
	return self;
}

- (void) updateContent : (BOOL) isCompact : (BOOL) isDetailColumn : (BOOL) isRoot : (void *) repo : (NSString *) detailString
{
	_isRoot = isRoot;
	_isCompact = isCompact;
	_isDetailColumn = isDetailColumn;
	
	if(isDetailColumn || _wantActivationState)
	{
		if(!_wantActivationState)
		{
			image = nil;
			title.hidden = YES;
			detail.hidden = YES;
		}
		
		if(repo == NULL)
			return;
		
		_repoUsedInDetail = repo;
		
		if(activationButton == nil)
		{
			activationButton = [[RakSwitchButton alloc] init];
			if(activationButton != nil)
			{
				activationButton.target = self;
				activationButton.action = @selector(buttonClicked);
				
				[self addSubview:activationButton];
			}
		}
		else
			activationButton.hidden = NO;
		
		[self refreshButtonState];
	}
	
	if(!isDetailColumn)
	{
		title.hidden = NO;
		detail.hidden = NO;
		
		if(!_wantActivationState)
			activationButton.hidden = YES;
		
		//Image
		image = loadImageForRepo(isRoot, repo);
		
		//Title
		NSString * string = getStringForWchar(isRoot ? ((ROOT_REPO_DATA *) repo)->name : ((REPO_DATA *) repo)->name);
		
		if(title == nil)
		{
			title = [[RakText alloc] initWithText:string :[self textColor]];
			if(title != nil)
			{
				if(!_isCompact)
					title.font = [NSFont fontWithName:[Prefs getFontName:GET_FONT_PREFS_TITLE] size:15];
				[title sizeToFit];
				
				[self addSubview:title];
			}
		}
		else
		{
			title.stringValue = string;
			[title sizeToFit];
		}
		
		//Detail
		if(!_isCompact)
		{
			if(isRoot)
			{
				uint nombreSubrepo = 0;
				
				for(uint i = 0, length = ((ROOT_REPO_DATA *) repo)->nombreSubrepo; i < length; i++)
				{
					if(((ROOT_REPO_DATA *) repo)->subRepo[i].active)
						nombreSubrepo++;
				}
				
				if(nombreSubrepo == 0)
					string = NSLocalizedString(@"PREFS-ROOT-NO-ACTIVE-REPO", nil);
				else if(nombreSubrepo == 1)
					string = NSLocalizedString(@"PREFS-ROOT-ONE-ACTIVE-REPO", nil);
				else
					string = [NSString localizedStringWithFormat:NSLocalizedString(@"PREFS-ROOT-%zu-ACTIVE-REPO", nil), ((ROOT_REPO_DATA *) repo)->nombreSubrepo];
			}
			else
			{
				if(detailString == nil)
					string = NSLocalizedString(@"PREFS-ROOT-NO-GROUP", nil);
				else
					string = [NSString localizedStringWithFormat:NSLocalizedString(@"PREFS-ROOT-GROUPS-%@", nil), detailString];
			}
			
			if(detail == nil)
			{
				detail = [[RakClickableText alloc] initWithText:string :[self detailTextColor] responder:self];
				
				if(detail != nil)
				{
					if(!isRoot)
						detail.URL = @(((REPO_DATA *) repo)->parentRepoID);
					
					detail.font = [NSFont fontWithName:[Prefs getFontName:GET_FONT_STANDARD] size:12];
					[detail sizeToFit];
					
					[self addSubview:detail];
				}
			}
			else
			{
				detail.hidden = NO;
				detail.stringValue = string;
			}
		}
		else
			detail.hidden = YES;
	}
}

- (void) respondTo : (RakClickableText *) sender
{
	NSNumber * group = sender.URL;
	
	if(group != nil && [group isKindOfClass:[NSNumber class]])
	{
		[_responder selectionUpdate:YES :[_responder posOfParent:[group unsignedIntValue]]];
	}
}

- (void) dealloc
{
	[Prefs deRegisterForThemeChanges:self];
}

#pragma mark - Button management

- (void) buttonClicked
{
	[_responder statusTriggered : self : _repoUsedInDetail];
}

- (BOOL) getButtonState
{
	return activationButton.state != NSOffState;
}

- (void) refreshButtonState
{
	activationButton.state = _repoUsedInDetail->active ? NSOnState : NSOffState;
}

- (void) cancelSelection
{
	activationButton.state = NSOnState;
}

#pragma mark - Frame forcing & drawing

- (CGFloat) imageDiameter
{
	return _isCompact ? [super imageDiameter] / 2 : [super imageDiameter];
}

- (void) frameChanged : (NSSize) newSize
{
	if(_isDetailColumn || _wantActivationState)
	{
		[activationButton setFrameOrigin:NSMakePoint(_isDetailColumn ? 0 : (newSize.width - activationButton.bounds.size.width - (_isRoot ? 0 : 2)), newSize.height / 2 - activationButton.bounds.size.height / 2)];
	}
	
	if(!_isDetailColumn)
	{
		[super frameChanged:newSize];
		
		if(_isCompact)
		{
			[title setFrameOrigin:NSMakePoint([self titleX], _bounds.size.height / 2 - title.bounds.size.height / 2)];
		}
		else
		{
			[title setFrameOrigin:NSMakePoint([self titleX], OFFSET_TITLE_Y)];
			[detail setFrameOrigin:NSMakePoint([self titleX], OFFSET_DETAIL_Y)];
		}
	}
}

- (void) drawRect:(NSRect)dirtyRect
{
	if(_highlighted)
	{
		[[self backgroundColor] setFill];
		NSRectFill(dirtyRect);
	}
	
	if(_isDetailColumn)
		return;
	
	[super drawRect:dirtyRect];
}

#pragma mark - Color

- (NSColor *) backgroundColor
{
	return [Prefs getSystemColor:COLOR_BACKGROUND_REPO_LIST_ITEM :nil];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	
	detail.textColor = [self detailTextColor];
	[self setNeedsDisplay:YES];
}

@end