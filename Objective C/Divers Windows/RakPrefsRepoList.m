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
	LIST_ROW_IMAGE_DIAMETER = 40,
	
	OFFSET_IMAGE_ROW = 10,
	OFFSET_TITLE_X = 5,
	OFFSET_TITLE_Y = 22,
	OFFSET_DETAIL_Y = 6
};

@interface RakPrefsRepoListItem : NSView
{
	NSImage * image;
	RakText * title;
	RakClickableText * detail;
	
	NSRect imageFrame;
}

@property BOOL highlighted;
@property RakPrefsRepoView * __weak responder;

- (instancetype) initWithRepo : (BOOL) isRoot : (void *) repo : (NSString *) detailString;
- (void) updateContent : (BOOL) isRoot : (void *) repo : (NSString *) detailString;

@end

@implementation RakPrefsRepoList

- (instancetype) initWithFrame : (NSRect) frame
{
	self = [self init];
	
	if(self != nil)
	{
		_nbData = [_responder sizeForMode:_rootMode];
		
		[self applyContext:frame : selectedRowIndex : -1];
		
		scrollView.wantsLayer = YES;
		scrollView.layer.cornerRadius = 3;
		scrollView.drawsBackground = YES;
		scrollView.backgroundColor = [Prefs getSystemColor:GET_COLOR_BACKGROUND_REPO_LIST :self];
	}
	
	return self;
}

- (void) tableViewSelectionDidChange:(NSNotification *)notification
{
	[_responder selectionUpdate:_rootMode :selectedRowIndex];
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
	
	[_tableView removeRowsAtIndexes:[NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, _nbData)] withAnimation:NSTableViewAnimationSlideLeft];
	
	_rootMode = rootMode;
	_nbData = [_responder sizeForMode:_rootMode];
	
	[_tableView insertRowsAtIndexes:[NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, _nbData)] withAnimation:NSTableViewAnimationSlideLeft];
}

#pragma mark - Tableview code

- (NSInteger) numberOfRowsInTableView : (RakTableView *) tableView
{
	return _nbData;
}

- (CGFloat)tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row
{
	return LIST_ROW_HEIGHT;
}

- (NSView*) tableView : (RakTableView *) tableView viewForTableColumn : (NSTableColumn*) tableColumn row : (NSInteger) row
{
	void ** list = [_responder listForMode:_rootMode];
	if(list == NULL || row >= [_responder sizeForMode:_rootMode])
		return nil;
	
	NSString * detail = nil;
	if(!_rootMode && list[row] != NULL)
		detail = [_responder nameOfParent: ((REPO_DATA *) list[row])->parentRepoID];

	RakPrefsRepoListItem * result = [tableView makeViewWithIdentifier : _identifier owner:self];
	if (result == nil)
	{
		result = [[RakPrefsRepoListItem alloc] initWithRepo :_rootMode : list[row] : detail];
		if(result != nil)
			result.responder = _responder;
	}
	else
		[result updateContent:_rootMode :list[row] :detail];
	
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

- (instancetype) initWithRepo : (BOOL) isRoot : (void *) repo : (NSString *) detailString
{
	self = [self initWithFrame:NSMakeRect(0, 0, 300, LIST_ROW_HEIGHT)];
	
	if(self != nil)
	{
		[Prefs getCurrentTheme:self];
		[self updateContent:isRoot :repo :detailString];
	}
	
	return self;
}

- (void) updateContent : (BOOL) isRoot : (void *) repo : (NSString *) detailString
{
	//Image
	image = loadImageForRepo(isRoot, repo);
	
	//Title
	NSString * string = getStringForWchar(isRoot ? ((ROOT_REPO_DATA *) repo)->name : ((REPO_DATA *) repo)->name);
	
	if(title == nil)
	{
		title = [[RakText alloc] initWithText:string :[self textColor]];
		if(title != nil)
		{
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
	if(isRoot)
	{
		if(((ROOT_REPO_DATA *) repo)->nombreSubrepo == 0)
			string = NSLocalizedString(@"PREFS-ROOT-NO-ACTIVE-REPO", nil);
		else if(((ROOT_REPO_DATA *) repo)->nombreSubrepo == 1)
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
		detail.stringValue = string;
}

- (void) respondTo : (RakClickableText *) sender
{
	NSNumber * group = sender.URL;
	
	if(group != nil && [group isKindOfClass:[NSNumber class]])
	{
		[_responder selectionUpdate:YES :[_responder posOfParent:[group unsignedIntValue]]];
	}
}

#pragma mark - Drawing

- (void) setFrame:(NSRect)frameRect
{
	[super setFrame:frameRect];
	
	imageFrame = NSMakeRect(OFFSET_IMAGE_ROW, frameRect.size.height / 2 - LIST_ROW_IMAGE_DIAMETER / 2, LIST_ROW_IMAGE_DIAMETER, LIST_ROW_IMAGE_DIAMETER);
	
	[title setFrameOrigin:NSMakePoint(NSMaxX(imageFrame) + OFFSET_TITLE_X, OFFSET_TITLE_Y)];
	[detail setFrameOrigin:NSMakePoint(NSMaxX(imageFrame) + OFFSET_TITLE_X, OFFSET_DETAIL_Y)];
}

- (void) drawRect:(NSRect)dirtyRect
{
	if(_highlighted)
	{
		[[self backgroundColor] setFill];
		NSRectFill(dirtyRect);
	}

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