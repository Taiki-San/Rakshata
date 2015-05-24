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

@interface RakPrefsFavoriteListView : RakListItemView
{
	PROJECT_DATA _project;

	RakText * repo;
	RakButton * refresh, * read, * remove;
}

- (instancetype) initWithProject : (PROJECT_DATA) project;
- (void) updateContent : (PROJECT_DATA) project;

@end

@implementation RakPrefsFavoriteList

- (instancetype) initWithFrame : (NSRect) frame
{
	self = [self init];
	
	if(self != nil)
	{
		lastTransmittedSelectedRowIndex = LIST_INVALID_SELECTION;
		projectDB = getCopyCache(RDB_LOAD_FAVORITE | SORT_NAME | RDB_EXCLUDE_DYNAMIC, &_nbData);
		
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
	if(selectedRowIndex != lastTransmittedSelectedRowIndex)
	{
		
		lastTransmittedSelectedRowIndex = selectedColumnIndex;
	}
}

- (void) resetSelection:(NSTableView *)tableView
{
	lastTransmittedSelectedRowIndex = VALEUR_FIN_STRUCT;
	
	[super resetSelection:tableView];
}

- (Class) contentClass
{
	return [RakPrefsRepoListItemView class];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return;
	
	scrollView.backgroundColor = [Prefs getSystemColor:GET_COLOR_BACKGROUND_REPO_LIST :nil];
	
	[super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
}

#pragma mark - Tableview code

- (NSInteger) numberOfRowsInTableView : (RakTableView *) tableView
{
	return _nbData;
}

- (CGFloat) tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row
{
	return LIST_WIDE_LINE_HEIGHT;
}

- (NSView*) tableView : (RakTableView *) tableView viewForTableColumn : (NSTableColumn*) tableColumn row : (NSInteger) row
{
	if(row >= _nbData)
		return nil;

	PROJECT_DATA project = projectDB[row];
	if(!project.isInitialized)
		return nil;
		
	RakPrefsFavoriteListView * result = [tableView makeViewWithIdentifier : _identifier owner:self];
	if (result == nil)
		result = [[RakPrefsFavoriteListView alloc] initWithProject : project];
	else
		[result updateContent : project];
	
	return result;
}

@end

@implementation RakPrefsFavoriteListView

- (instancetype) initWithProject : (PROJECT_DATA) project
{
	self = [self initWithFrame:NSMakeRect(0, 0, 300, LIST_WIDE_LINE_HEIGHT)];
	
	if(self != nil)
	{
		[self updateContent:project];
		
		[Prefs getCurrentTheme:self];
	}
	
	return self;
}

- (void) updateContent : (PROJECT_DATA) project
{
	_project = project;
	
	image = loadDDThumbnail(_project);

	if(title == nil)
	{
		title = [[RakText alloc] initWithText:getStringForWchar(_project.projectName) :[self textColor]];
		if(title != nil)
		{
			title.font = [NSFont fontWithName:[Prefs getFontName:GET_FONT_PREFS_TITLE] size:15];
			[self addSubview:title];
		}
	}
	else
		title.stringValue = getStringForWchar(_project.projectName);
	
	[title sizeToFit];
	
	
	if(repo == nil)
	{
		repo = [[RakText alloc] initWithText:getStringForWchar(_project.repo->name) :[self detailTextColor]];
		if(repo != nil)
			[self addSubview:repo];
	}
	else
		repo.stringValue = getStringForWchar(_project.repo->name);
	
	[repo sizeToFit];
	
//	if(refresh == nil)
//	{
//		
//	}
}

#pragma mark - Update width

- (void) frameChanged : (NSSize) newSize
{
	[super frameChanged:newSize];
	
	[title setFrameOrigin:NSMakePoint([self titleX], 22)];
	[repo setFrameOrigin:NSMakePoint([self titleX], 6)];
}

@end