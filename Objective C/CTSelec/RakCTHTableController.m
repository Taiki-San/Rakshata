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

#define BOTTOM_BORDER 5

@implementation RakCTHTableController

- (instancetype) initWithProject : (PROJECT_DATA) project frame : (NSRect) frame
{
	self = [self init];
	
	if(self != nil)
	{
		[self analyseCurrentProject : project];
		[self craftTableView:frame];
		[Prefs getCurrentTheme:self];
	}
	
	return self;
}

#pragma mark - Interface

- (void) updateProjectWithID : (uint) projectID
{
	PROJECT_DATA project = getElementByID(projectID, RDB_CTXCT);
	
	[self updateProject : project];
	
	releaseCTData(project);
}

- (void) updateProject : (PROJECT_DATA) project
{
	[self analyseCurrentProject : project];
	[self refreshLayout];
}

#pragma mark - Data tools

- (void) analyseCurrentProject : (PROJECT_DATA) project
{
	cacheID = project.cacheDBID;
	
	numberOfChapters = project.nombreChapitre;
	numberOfChaptersInstalled = project.nombreChapitreInstalled;
	
	numberOfVolumes = project.nombreTomes;
	numberOfVolumesInstalled = project.nombreTomesInstalled;
	
	status = project.status;
	type = project.type;
	category = project.category;

	paidContent = !strcmp(project.team->type, TYPE_DEPOT_3);
	DRM = YES;
	
	numberOfRows = 5 + (numberOfChapters != 0) + (numberOfVolumes != 0);
	
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
		
		newFrame.origin.y = newFrame.size.height / 2 - tableHeight / 2;
		newFrame.size.height = tableHeight;
	}
	else
	{
		[_tableView scrollRowToVisible:0];
		self.scrollView.scrollingDisabled = NO;
	}
	
	if(animated)
		[self.scrollView.animator setFrame : newFrame];
	else
		[self.scrollView setFrame : newFrame];
	
	[[_tableView tableColumnWithIdentifier : RCTH_TITLE_ID] setWidth : newFrame.size.width * 2 / 5];
	[[_tableView tableColumnWithIdentifier : RCTH_DETAILS_ID] setWidth : newFrame.size.width * 1 / 2];
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
	[_tableView setBackgroundColor:[NSColor clearColor]];
	[_tableView setSelectionHighlightStyle:NSTableViewSelectionHighlightStyleNone];
	[_tableView setFocusRingType:NSFocusRingTypeNone];
	[_tableView setAllowsMultipleSelection:NO];
	
	//TableView columns init
	NSTableColumn * titles = [[NSTableColumn alloc] initWithIdentifier:RCTH_TITLE_ID];
	[titles setWidth:_tableView.frame.size.width * 2 / 5];
	[_tableView addTableColumn:titles];
	titles = [[NSTableColumn alloc] initWithIdentifier:RCTH_DETAILS_ID];
	[titles setWidth:_tableView.frame.size.width * 1 / 2];
	[_tableView addTableColumn:titles];

	//End of setup
	[_tableView setDelegate:self];
	[_tableView setDataSource:self];
	[_tableView reloadData];
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

- (NSColor *) textColor : (BOOL) title
{
	return [Prefs getSystemColor:GET_COLOR_CTHEADER_FONT : nil];
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return;
	
	[_tableView reloadData];
}

- (BOOL) isTitleColumn : (NSTableColumn *) column
{
	return [[column identifier] isEqualToString:RCTH_TITLE_ID];
}

#pragma mark - Delegate

- (BOOL) tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)row	{	return NO;	}

#pragma mark - Data source

- (NSInteger) numberOfRowsInTableView : (RakTableView *) tableView	{	return numberOfRows;	}

- (NSTableRowView *)tableView:(NSTableView *)tableView rowViewForRow:(NSInteger)row
{
	NSTableRowView * output = [[NSTableRowView alloc] init];
	
	output.autoresizesSubviews = NO;
	output.wantsLayer = NO;
	
	return output;
}

- (NSView*) tableView : (RakTableView *) tableView viewForTableColumn : (NSTableColumn*) tableColumn row : (NSInteger) row
{
	// Get an existing view with the identifier if it exists
	RakText *result = [tableView makeViewWithIdentifier:@"PFUDOR" owner:self];
	BOOL isTitleColumn = [self isTitleColumn:tableColumn];
	
	if (result == nil)
	{
		result = [[RakText alloc] init];
		[result setFrame:NSMakeRect(0, 0, tableColumn.width, 35)];
		
		[result setFont:[NSFont fontWithName:[Prefs getFontName:GET_FONT_STANDARD] size:13]];
		[result setIdentifier: @"PFUDOR"];
	}

	[result setTextColor : [self textColor : isTitleColumn]];
	[result setAlignment : isTitleColumn ? NSLeftTextAlignment : NSRightTextAlignment];
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
		case 0:
		{
			if(titleColumn)
				ret_value = [NSString stringWithFormat:@"Chapitre%c", numberOfChapters == 1 ? '\0' : 's'];
			else
			{
				if(numberOfChapters == numberOfChaptersInstalled)
				{
					if(numberOfChaptersInstalled == 0)
						ret_value = [NSString stringWithFormat:@"%d (aucun installé)", numberOfChapters];
					else if(numberOfChaptersInstalled == 1)
						ret_value = [NSString stringWithFormat:@"%d (installé)", numberOfChapters];
					else
						ret_value = [NSString stringWithFormat:@"%d (tous installés)", numberOfChapters];
				}
				else
					ret_value = [NSString stringWithFormat:@"%d (%d installé%s", numberOfChapters, numberOfChaptersInstalled, numberOfChaptersInstalled <= 1 ? ")" : "s)"];
			}
			break;
		}

		case 1:
		{
			if(titleColumn)
				ret_value = [NSString stringWithFormat:@"Tome%c", numberOfVolumes == 1 ? '\0' : 's'];
			else
			{
				if(numberOfVolumes == numberOfVolumesInstalled)
				{
					if(numberOfVolumesInstalled == 0)
						ret_value = [NSString stringWithFormat:@"%d (aucun installé)", numberOfVolumes];
					else if(numberOfVolumesInstalled == 1)
						ret_value = [NSString stringWithFormat:@"%d (installé)", numberOfVolumes];
					else
						ret_value = [NSString stringWithFormat:@"%d (tous installés)", numberOfVolumes];
				}
				else
					ret_value = [NSString stringWithFormat:@"%d (%d installé%s", numberOfVolumes, numberOfVolumesInstalled, numberOfVolumesInstalled <= 1 ? ")" : "s)"];
			}
			break;
		}

		case 2:
		{
			if(titleColumn)
				ret_value = @"Status";
			else
			{
				if(status == STATUS_OVER)
					ret_value = @"Terminé";
				else if(status == STATUS_CANCELED)
					ret_value = @"Annulé";
				else if(status == STATUS_SUSPENDED)
					ret_value = @"En pause";
				else if(status == STATUS_WIP)
					ret_value = @"En cours";
				else if(status == STATUS_ANOUNCED)
					ret_value = @"Annoncé";
			}
			break;
		}
		case 3:
		{
			if(titleColumn)
				ret_value = @"Type";
			else
			{
				if(type == TYPE_BD)
					ret_value = @"Bande dessinée";
				else if(type == TYPE_MANGA)
					ret_value = @"Manga";
				else if(type == TYPE_MANWA)
					ret_value = @"Manwa";
				else if(type == TYPE_COMIC)
					ret_value = @"Comic";
				else
					ret_value = @"Inconnu";
			}
			break;
		}
		case 4:
		{
			if(titleColumn)
				ret_value = @"Catégorie";
			else
			{
				ret_value = @"Meeeh, later";
			}
			break;
		}
		case 5:
		{
			if(titleColumn)
				ret_value = @"Payant";
			else
				ret_value = paidContent ? @"Oui" : @"Non";
			
			break;
		}
			
		case 6:
		{
			if(titleColumn)
				ret_value = @"Exportable";
			else
				ret_value = DRM ? @"Non" : @"Oui";

			break;
		}
	}
	
	return ret_value;
}

@end
