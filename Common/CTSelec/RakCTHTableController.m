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
	BOTTOM_BORDER = 5,
	
	ROW_CHAPTER = 0,
	ROW_VOLUME = 1,
	ROW_STATUS = 2,
	ROW_CATEGORY = 3,
	ROW_TAGS = 4,
	ROW_PAID = 5,
	ROW_DRM = 6
};

#define WIDTH_TITLE_COLUMN (7.0f / 20.0f)
#define WIDTH_CONTENT_COLUMN (11.0f / 20.0f)

@implementation RakCTHTableController

- (instancetype) initWithProject : (PROJECT_DATA) project frame : (NSRect) frame
{
	self = [self init];
	
	if(self != nil)
	{
		[self analyseCurrentProject : project];
		[self craftTableView:frame];
		[Prefs registerForChange:self forType:KVO_THEME];
	}
	
	return self;
}

- (void) dealloc
{
	[Prefs deRegisterForChange:self forType:KVO_THEME];
}

#pragma mark - Interface

- (void) updateProject : (PROJECT_DATA) project
{
	[self analyseCurrentProject : project];
	[self refreshLayout];
}

- (CGFloat) baseX
{
	return _scrollView.frame.origin.x;
}

- (CGFloat) rawBaseX : (NSRect) frameRect
{
	return [self frameFromParent:frameRect].origin.x;
}

#pragma mark - Data tools

- (void) analyseCurrentProject : (PROJECT_DATA) project
{
	if(!project.isInitialized)
	{
		numberOfRows = 0;
	}
	else
	{
		cacheID = project.cacheDBID;
		
		numberOfChapters = project.nbChapter;
		numberOfChaptersInstalled = project.nbChapterInstalled;
		
		numberOfVolumes = project.nbVolumes;
		numberOfVolumesInstalled = project.nbVolumesInstalled;
		
		status = project.status;
		category = project.category;
		tag = project.mainTag;
		
		paidContent = project.isPaid;
		DRM = project.haveDRM;
		
		numberOfRows = 5 + (numberOfChapters != 0) + (numberOfVolumes != 0);
	}
	
	if(_tableView != nil)
		[_tableView reloadData];
}

- (void) setFrame : (NSRect) frameRect
{
	[self resizeScrollView : [self frameFromParent:frameRect] : NO];
}

- (void) resizeAnimation : (NSRect) frameRect
{
	[self resizeScrollView : [self frameFromParent:frameRect] : YES];
}

- (void) refreshLayout
{
	NSRect frame = self.scrollView.frame;
	
	//From time to time, reloadData may increase our y coordinate
	frame.origin.y = BOTTOM_BORDER;
	
	[self resizeScrollView : frame : NO];
}

- (void) resizeScrollView : (NSRect) newFrame : (BOOL) animated
{
	CGFloat tableHeight = _tableView.bounds.size.height;
	
	if(tableHeight < newFrame.size.height)
	{
		self.scrollView.scrollingDisabled = YES;
		
		newFrame.origin.y += newFrame.size.height / 2 - tableHeight / 2;
		newFrame.size.height = tableHeight;
	}
	else
	{
		[_tableView scrollRowToVisible:0];
		self.scrollView.scrollingDisabled = NO;
	}
		
	if(animated)
		[self.scrollView setFrameAnimated : newFrame];
	else
		[self.scrollView setFrame : newFrame];
	
	//Force columns width
	[_tableView tableColumnWithIdentifier : RCTH_TITLE_ID].width = newFrame.size.width * WIDTH_TITLE_COLUMN;
	[_tableView tableColumnWithIdentifier : RCTH_DETAILS_ID].width = newFrame.size.width * WIDTH_CONTENT_COLUMN;
}

#pragma mark - UI tools

- (void) craftTableView : (NSRect) frame
{
	if(self.scrollView != nil)
		return;
	
	frame = [self frameFromParent:frame];
	
	//Init at height = 1 in order to be sure that tableview height is not related to scrollview height
	self.scrollView = [[RakListScrollView alloc] initWithFrame: NSMakeRect(0, 0, frame.size.width, 1)];
	if(self.scrollView == nil)
		return;
	
	[self.scrollView.verticalScroller setAlphaValue:0];
	
	_tableView = [[NSTableView alloc] init];
	if(_tableView == nil)
	{
		self.scrollView = nil;
		return;
	}
	
	//View configuration
	_tableView.wantsLayer = NO;
	_tableView.autoresizesSubviews = NO;
	[self.scrollView setDocumentView:_tableView];
	
	//Tableview configuration
	[_tableView setHeaderView:nil];
	[_tableView setBackgroundColor:[RakColor clearColor]];
	[_tableView setSelectionHighlightStyle:NSTableViewSelectionHighlightStyleNone];
	[_tableView setFocusRingType:NSFocusRingTypeNone];
	[_tableView setAllowsMultipleSelection:NO];
	
	//TableView columns init
	NSTableColumn * titles = [[NSTableColumn alloc] initWithIdentifier:RCTH_TITLE_ID];
	titles.width = _tableView.frame.size.width * WIDTH_TITLE_COLUMN;
	[_tableView addTableColumn:titles];
	
	titles = [[NSTableColumn alloc] initWithIdentifier:RCTH_DETAILS_ID];
	titles.width = _tableView.frame.size.width * WIDTH_CONTENT_COLUMN;
	[_tableView addTableColumn:titles];
	
	//End of setup
	[_tableView setDelegate:self];
	[_tableView setDataSource:self];
	[_tableView scrollRowToVisible:0];
	
	//Update positions once the table was populated
	[self performSelectorOnMainThread:@selector(refreshLayout) withObject:nil waitUntilDone:NO];
}

- (NSRect) frameFromParent : (NSRect) parentBounds
{
	parentBounds.origin.y = BOTTOM_BORDER;
	parentBounds.origin.x = parentBounds.size.width / 2;
	
	parentBounds.size.width -= parentBounds.origin.x;
	parentBounds.size.height -= 2 * BOTTOM_BORDER;
	
	return parentBounds;
}

- (RakColor *) textColor : (BOOL) title
{
	return [Prefs getSystemColor:COLOR_CTHEADER_FONT];
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	
	[_tableView reloadData];
}

- (BOOL) isTitleColumn : (NSTableColumn *) column
{
	return [[column identifier] isEqualToString:RCTH_TITLE_ID];
}

#pragma mark - Delegate

- (BOOL) tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)row
{
	if(numberOfChapters == 0)
		row++;
	if(numberOfVolumes == 0 && row > 0)
		row++;
	
	switch (row)
	{
		case ROW_CATEGORY:
		{
			uint ID = _getFromSearch(NULL, PULL_SEARCH_CATID, &category);
			
			if(ID != UINT_MAX)
			{
				[[NSNotificationCenter defaultCenter] postNotificationName:SR_NOTIFICATION_TYPE object:getStringForWchar(getCatNameForCode(category)) userInfo:@{SR_NOTIF_CACHEID : @(ID), SR_NOTIF_OPTYPE : @(YES)}];
				[RakApp.serie ownFocus];
			}
			break;
		}
			
		case ROW_TAGS:
		{
			uint ID = _getFromSearch(NULL, PULL_SEARCH_TAGID, &tag);
			
			if(ID != UINT_MAX)
			{
				[[NSNotificationCenter defaultCenter] postNotificationName:SR_NOTIFICATION_TAG object:getStringForWchar(getTagNameForCode(tag)) userInfo:@{SR_NOTIF_CACHEID : @(ID), SR_NOTIF_OPTYPE : @(YES)}];
				[RakApp.serie ownFocus];
			}
			
			break;
		}
	}
	
	return NO;
}

#pragma mark - Data source

- (NSInteger) numberOfRowsInTableView : (RakTableView *) tableView	{	return numberOfRows;	}

- (NSTableRowView *)tableView:(NSTableView *)tableView rowViewForRow:(NSInteger)row
{
	NSTableRowView * output = [[NSTableRowView alloc] init];
	
	output.autoresizesSubviews = NO;
	output.wantsLayer = NO;
	
	return output;
}

- (RakView*) tableView : (RakTableView *) tableView viewForTableColumn : (NSTableColumn*) tableColumn row : (NSInteger) row
{
	// Get an existing view with the identifier if it exists
	RakText *result = [tableView makeViewWithIdentifier:@"PFUDOR" owner:self];
	BOOL isTitleColumn = [self isTitleColumn:tableColumn];
	
	if(result == nil)
	{
		result = [[RakText alloc] init];
		
		[result setFont:[NSFont fontWithName:[Prefs getFontName:GET_FONT_STANDARD] size:13]];
		[result setIdentifier: @"PFUDOR"];
	}
	
	[result setTextColor : [self textColor : isTitleColumn]];
	[result setAlignment : isTitleColumn ? NSTextAlignmentLeft : NSTextAlignmentRight];
	[result setDrawsBackground:NO];
	
	return result;
}

- (NSString*) tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)rowIndex
{
	NSString * ret_value = nil;
	BOOL titleColumn = [self isTitleColumn:tableColumn];
	
	if(numberOfChapters == 0)
		rowIndex++;
	if(numberOfVolumes == 0 && rowIndex > 0)
		rowIndex++;
	
	switch (rowIndex)
	{
		case ROW_CHAPTER:
		{
			if(titleColumn)
				ret_value = [NSString stringWithFormat:NSLocalizedString(@"CHAPTER%c", nil), numberOfChapters == 1 ? '\0' : 's'];
			else
			{
				if(numberOfChapters == numberOfChaptersInstalled)
				{
					if(numberOfChaptersInstalled == 0)
						ret_value = [NSString stringWithFormat:NSLocalizedString(@"CT-%zu-NO-INSTALLED", nil), numberOfChapters];
					else if(numberOfChaptersInstalled == 1)
						ret_value = [NSString stringWithFormat:NSLocalizedString(@"CT-%zu-ONLY-ONE-AND-INSTALLED", nil), numberOfChapters];
					else
						ret_value = [NSString stringWithFormat:NSLocalizedString(@"CT-%zu-ALL-INSTALLED", nil), numberOfChapters];
				}
				else
				{
					if(numberOfChaptersInstalled <= 1)
						ret_value = [NSString stringWithFormat:NSLocalizedString(@"CT-%zu-ONLY-%zu-INSTALLED", nil), numberOfChapters, numberOfChaptersInstalled];
					else
						ret_value = [NSString stringWithFormat:NSLocalizedString(@"CT-%zu-ONLY-%zu-INSTALLED-SEVERAL", nil), numberOfChapters, numberOfChaptersInstalled];
				}
			}
			break;
		}
			
		case ROW_VOLUME:
		{
			if(titleColumn)
				ret_value = [NSString stringWithFormat:@"Tome%c", numberOfVolumes == 1 ? '\0' : 's'];
			else
			{
				if(numberOfVolumes == numberOfVolumesInstalled)
				{
					if(numberOfVolumesInstalled == 0)
						ret_value = [NSString stringWithFormat:NSLocalizedString(@"CT-%zu-NO-INSTALLED", nil), numberOfVolumes];
					else if(numberOfVolumesInstalled == 1)
						ret_value = [NSString stringWithFormat:NSLocalizedString(@"CT-%zu-ONLY-ONE-AND-INSTALLED", nil), numberOfVolumes];
					else
						ret_value = [NSString stringWithFormat:NSLocalizedString(@"CT-%zu-ALL-INSTALLED", nil), numberOfVolumes];
				}
				else
				{
					if(numberOfVolumesInstalled <= 1)
						ret_value = [NSString stringWithFormat:NSLocalizedString(@"CT-%zu-ONLY-%zu-INSTALLED", nil), numberOfVolumes, numberOfVolumesInstalled];
					else
						ret_value = [NSString stringWithFormat:NSLocalizedString(@"CT-%zu-ONLY-%zu-INSTALLED-SEVERAL", nil), numberOfVolumes, numberOfVolumesInstalled];
				}
			}
			break;
		}
			
		case ROW_STATUS:
		{
			if(titleColumn)
				ret_value = NSLocalizedString(@"CT-STATUS", nil);
			else
			{
				if(status == STATUS_OVER)
					ret_value = NSLocalizedString(@"CT-STATUS-OVER", nil);
				else if(status == STATUS_CANCELED)
					ret_value = NSLocalizedString(@"CT-STATUS-CANCELLED", nil);
				else if(status == STATUS_SUSPENDED)
					ret_value = NSLocalizedString(@"CT-STATUS-PAUSE", nil);
				else if(status == STATUS_WIP)
					ret_value = NSLocalizedString(@"CT-STATUS-WIP", nil);
				else if(status == STATUS_ANOUNCED)
					ret_value = NSLocalizedString(@"CT-STATUS-ANNOUNCED", nil);
			}
			break;
		}
		case ROW_CATEGORY:
		{
			if(titleColumn)
				ret_value = NSLocalizedString(@"CT-TYPE", nil);
			else
				ret_value = getStringForWchar(getCatNameForCode(category));
			break;
		}
		case ROW_TAGS:
		{
			if(titleColumn)
				ret_value = NSLocalizedString(@"CT-TAGS", nil);
			else
				ret_value = getStringForWchar(getTagNameForCode(tag));
			break;
		}
		case ROW_PAID:
		{
			if(titleColumn)
				ret_value = NSLocalizedString(@"CT-PAID", nil);
			else
				ret_value = paidContent ? NSLocalizedString(@"YES", nil) : NSLocalizedString(@"NO", nil);
			
			break;
		}
			
		case ROW_DRM:
		{
			if(titleColumn)
				ret_value = NSLocalizedString(@"CT-NO-DRM", nil);
			else
				ret_value = DRM ? NSLocalizedString(@"NO", nil) : NSLocalizedString(@"YES", nil);
			
			break;
		}
	}
	
	return ret_value;
}

@end
