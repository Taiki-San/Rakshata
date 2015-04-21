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
	LIST_ROW_HEIGHT = 45,
	LIST_ROW_COMPACT_HEIGHT = 25,
	LIST_ROW_COMPACT_WIDTH = 25,
	LIST_ROW_IMAGE_DIAMETER = 40,
	
	OFFSET_IMAGE_ROW = 10,
	OFFSET_TITLE_X = 5,
	OFFSET_TITLE_Y = 22,
	OFFSET_DETAIL_Y = 6
};

@interface RakPrefsRepoListItem()
{
	NSImage * image;
	RakText * title;
	RakClickableText * detail;
	RakRadioButton * activationButton;
	
	BOOL _isRoot;
	BOOL _isCompact;
	BOOL _isDetailColumn;

	REPO_DATA * _repoUsedInDetail;
	
	NSRect imageFrame;
}

@property BOOL highlighted;
@property id __weak responder;

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
		scrollView.backgroundColor = [Prefs getSystemColor:GET_COLOR_BACKGROUND_REPO_LIST :self];
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
	lastTransmittedSelectedRowIndex = VALEUR_FIN_STRUCT;
	
	[super resetSelection:tableView];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return;
	
	scrollView.backgroundColor = [Prefs getSystemColor:GET_COLOR_BACKGROUND_REPO_LIST :nil];
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

	RakPrefsRepoListItem * result = [tableView makeViewWithIdentifier : _identifier owner:self];
	if (result == nil)
	{
		result = [[RakPrefsRepoListItem alloc] initWithRepo : _detailMode : tableColumn == _detailColumn : _rootMode : data : detail];
		if(result != nil)
			result.responder = _responder;
	}
	else
		[result updateContent : _detailMode : tableColumn == _detailColumn :_rootMode :data :detail];
	
	return result;
}

- (void) graphicSelection:(RakPrefsRepoListItem *)view :(BOOL)select
{
	if([view class] == [RakPrefsRepoListItem class])
	{
		view.highlighted = select;
		[view setNeedsDisplay:YES];
	}
}

@end

@implementation RakPrefsRepoListItem

- (instancetype) initWithRepo : (BOOL) isCompact : (BOOL) isDetailColumn : (BOOL) isRoot : (void *) repo : (NSString *) detailString
{
	self = [self initWithFrame:NSMakeRect(0, 0, 300, isCompact ? LIST_ROW_COMPACT_HEIGHT : LIST_ROW_HEIGHT)];
	
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
			activationButton = [[RakRadioButton alloc] init];
			if(activationButton != nil)
			{
				activationButton.target = self;
				activationButton.action = @selector(buttonClicked);
				
				[self addSubview:activationButton];
			}
		}
		else
			activationButton.hidden = NO;
		
		activationButton.state = _repoUsedInDetail->active ? NSOnState : NSOffState;
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

- (void) buttonClicked
{
	[_responder statusTriggered : self : _repoUsedInDetail];
}

- (BOOL) getButtonState
{
	return activationButton.state == NSOnState;
}

- (void) cancelSelection
{
	activationButton.state = NSOnState;
}

#pragma mark - Drawing

- (void) setFrame:(NSRect)frameRect
{
	[super setFrame:frameRect];
	
	if(_isDetailColumn || _wantActivationState)
	{
		frameRect.origin = NSZeroPoint;
		[activationButton setFrameOrigin:NSMakePoint(_isDetailColumn ? 0 : (frameRect.size.width - activationButton.bounds.size.width - (_isRoot ? 0 : 2)), frameRect.size.height / 2 - activationButton.bounds.size.height / 2)];
	}
	
	if(!_isDetailColumn)
	{
		const CGFloat diameter = _isCompact ? LIST_ROW_IMAGE_DIAMETER / 2 : LIST_ROW_IMAGE_DIAMETER;
		
		imageFrame = NSMakeRect(OFFSET_IMAGE_ROW, frameRect.size.height / 2 - diameter / 2, diameter, diameter);
		
		if(_isCompact)
		{
			[title setFrameOrigin:NSMakePoint(NSMaxX(imageFrame) + OFFSET_TITLE_X, _bounds.size.height / 2 - title.bounds.size.height / 2)];
		}
		else
		{
			[title setFrameOrigin:NSMakePoint(NSMaxX(imageFrame) + OFFSET_TITLE_X, OFFSET_TITLE_Y)];
			[detail setFrameOrigin:NSMakePoint(NSMaxX(imageFrame) + OFFSET_TITLE_X, OFFSET_DETAIL_Y)];
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

	[NSGraphicsContext saveGraphicsState];
	
	NSBezierPath *path = [NSBezierPath bezierPathWithRoundedRect:imageFrame
														 xRadius:LIST_ROW_IMAGE_DIAMETER / 2
														 yRadius:LIST_ROW_IMAGE_DIAMETER / 2];
	[path addClip];
	
	[image drawInRect:imageFrame
			 fromRect:NSZeroRect
			operation:NSCompositeSourceOver
			 fraction:1.0];
	
	[NSGraphicsContext restoreGraphicsState];
}

#pragma mark - Color

- (NSColor *) textColor
{
	return [Prefs getSystemColor:GET_COLOR_ACTIVE :nil];
}

- (NSColor *) detailTextColor
{
	return [Prefs getSystemColor:GET_COLOR_SURVOL :nil];
}

- (NSColor *) backgroundColor
{
	return [Prefs getSystemColor:GET_COLOR_BACKGROUND_REPO_LIST_ITEM :nil];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return;
	
	title.textColor = [self textColor];
	detail.textColor = [self detailTextColor];
	
	[self setNeedsDisplay:YES];
}

@end