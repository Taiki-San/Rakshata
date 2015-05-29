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
	OFFSET_Y_TITLE = 22,
	OFFSET_Y_REPO = 6
};

@interface RakPrefsFavoriteListView : RakListItemView
{
	RakClickableText * repo;
	RakButton * refresh, * read, * remove, * download;
	
	BOOL refreshing;
}

@property PROJECT_DATA project;

- (instancetype) initWithProject : (PROJECT_DATA) project;
- (void) updateContent : (PROJECT_DATA) project;

+ (void) readProject : (PROJECT_DATA) project;

@end

@implementation RakPrefsFavoriteList

- (instancetype) initWithFrame : (NSRect) frame
{
	self = [self init];
	
	if(self != nil)
	{
		lastTransmittedSelectedRowIndex = LIST_INVALID_SELECTION;
		projectDB = getCopyCache(RDB_LOAD_FAVORITE | SORT_NAME | RDB_EXCLUDE_DYNAMIC, &_nbData);
		IDList = getFavoritesID(&lengthList);
		
		if(lengthList != _nbData || IDList == NULL)
		{
			lengthList = _nbData;
			free(IDList);	IDList = NULL;
		}
		
		[RakDBUpdate registerForUpdate:self :@selector(DBUpdated:)];
		
		[self applyContext:frame : selectedRowIndex : -1];

		scrollView.wantsLayer = YES;
		scrollView.layer.cornerRadius = 3;
		scrollView.drawsBackground = YES;
		scrollView.backgroundColor = [Prefs getSystemColor:GET_COLOR_BACKGROUND_REPO_LIST :self];
		
		[self enableDrop];
	}
	
	return self;
}

- (void) dealloc
{
	[RakDBUpdate unRegister:self];
	free(projectDB);
	free(IDList);
}

- (void) DBUpdated : (NSNotification*) notification
{
	//Full reload if we lack too much data
	if(_nbData == 0 || IDList == NULL)
	{
		PROJECT_DATA * old = projectDB;
		
		projectDB = getCopyCache(RDB_LOAD_FAVORITE | SORT_NAME | RDB_EXCLUDE_DYNAMIC, &_nbData);
		[_tableView reloadData];
		
		free(old);
		return;
	}
	
	//We check if stuffs were added/removed
	uint newNbFavorites, * newIDList = getFavoritesID(&newNbFavorites), checkNbFavorites;
	PROJECT_DATA * newFavorites = getCopyCache(RDB_LOAD_FAVORITE | SORT_NAME | RDB_EXCLUDE_DYNAMIC, &checkNbFavorites);
	
	//No more favorites :O
	if(newNbFavorites == 0 || newIDList == NULL || newFavorites == NULL)
	{
		if(lengthList != 0)
			[_tableView removeRowsAtIndexes:[NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, lengthList)] withAnimation:NSTableViewAnimationSlideLeft];
		
		free(IDList);		IDList = NULL;
		free(projectDB);	projectDB = NULL;
		free(newIDList);
		free(newFavorites);

		_nbData = 0;
		return;
	}
	
	//Let's diff that
	uint oldNbData = _nbData, posOld = 0, posNew = 0, nbInsert = 0, insertions[oldNbData], nbRemoval = 0, removal[newNbFavorites];

	while(posOld < lengthList && posNew < newNbFavorites)
	{
		if(IDList[posOld] > newIDList[posNew])
			insertions[nbInsert++] = newIDList[posNew++];
		
		else if(IDList[posOld] < newIDList[posNew])
			removal[nbRemoval++] = IDList[posOld++];
		
		else
		{
			posNew++;
			posOld++;
		}
	}
	
	//Now, apply the changes, first, deletion
	NSMutableIndexSet * indexSet = [NSMutableIndexSet indexSet];

	if(nbRemoval)
	{
		for(posOld = 0; posOld < oldNbData && [indexSet count] < nbRemoval; posOld++)
		{
			for(uint posRemoval = 0; posRemoval < nbRemoval; posRemoval++)
			{
				if(projectDB[posOld].cacheDBID == removal[posRemoval])
				{
					[indexSet addIndex:posOld];
					break;
				}
			}
		}
		
		if([indexSet count])
		{
			[_tableView removeRowsAtIndexes:indexSet withAnimation:NSTableViewAnimationSlideLeft];

			if(nbInsert)
				[indexSet removeAllIndexes];
		}
	}
	
	//We check for updates on what is remaining
	[_tableView enumerateAvailableRowViewsUsingBlock:^(NSTableRowView *rowView, NSInteger row) {
		
		for(RakPrefsFavoriteListView * subview in rowView.subviews)
		{
			if([subview class] == [RakPrefsFavoriteListView class])
			{
				if([RakDBUpdate analyseNeedUpdateProject:notification.userInfo :subview.project])
				{
					PROJECT_DATA tmp = getProjectByIDHelper(subview.project.cacheDBID, false);
					if(tmp.isInitialized && !areProjectsIdentical(subview.project, tmp))
						[subview updateContent:tmp];
				}
			}
		}
	}];
	
	//Now, insertion
	free(projectDB);	free(IDList);
	projectDB = newFavorites;	IDList = newIDList;
	_nbData = newNbFavorites;

	if(nbInsert)
	{
		for(posNew = 0; posNew < _nbData && [indexSet count] < nbInsert; posNew++)
		{
			for(uint posInsert = 0; posInsert < nbInsert; posInsert++)
			{
				if(projectDB[posNew].cacheDBID == insertions[posInsert])
				{
					[indexSet addIndex:posNew];
					break;
				}
			}
		}
		
		if([indexSet count])
			[_tableView insertRowsAtIndexes:indexSet withAnimation:NSTableViewAnimationSlideLeft];
	}
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

- (void) tableViewSelectionDidChange:(NSNotification *)notification
{
	if(selectedRowIndex < _nbData)
		[RakPrefsFavoriteListView readProject:projectDB[selectedRowIndex]];
}

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

#pragma mark - Drop support

- (BOOL) grantDropAuthorization : (RakDragItem *) item
{
	return ![item isFavorite];
}

- (NSDragOperation) operationForContext : (id < NSDraggingInfo >) item : (uint) sourceTab : (NSInteger) suggestedRow : (NSTableViewDropOperation) operation
{
	return [RakDragItem isFavorite:[item draggingPasteboard]] ? NSDragOperationNone : NSDragOperationCopy;
}

- (uint) getSelfCode
{
	return TAB_UNKNOWN;
}

- (BOOL) receiveDrop : (PROJECT_DATA) project : (BOOL) isTome : (int) element : (uint) sender : (NSInteger)row : (NSTableViewDropOperation)operation
{
	setFavorite(&project);
	return YES;
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
		title = [[RakClickableText alloc] initWithText:getStringForWchar(_project.projectName) :[self textColor]];
		if(title != nil)
		{
			title.font = [NSFont fontWithName:[Prefs getFontName:GET_FONT_PREFS_TITLE] size:15];
			
			((RakClickableText *) title).ignoreURL = YES;
			title.clicTarget = self;
			title.clicAction = @selector(read);
			
			[self addSubview:title];
		}
	}
	else
		title.stringValue = getStringForWchar(_project.projectName);
	
	[title sizeToFit];
	
	
	if(repo == nil)
	{
		repo = [[RakClickableText alloc] initWithText:getStringForWchar(_project.repo->name) :[self detailTextColor]];
		if(repo != nil)
		{
			repo.ignoreURL = YES;
			repo.clicTarget = self;
			repo.clicAction = @selector(clickRepo);
			
			[self addSubview:repo];
		}
	}
	else
		repo.stringValue = getStringForWchar(_project.repo->name);
	
	[repo sizeToFit];
	
	if(refresh == nil)
	{
		refresh = [RakButton allocImageWithoutBackground:@"refresh" :RB_STATE_STANDARD :self :@selector(refresh)];
		if(refresh != nil)
		{
			[refresh.cell setActiveAllowed:YES];
			[self addSubview:refresh];
		}
	}
	
	if(read == nil)
	{
		read = [RakButton allocImageWithoutBackground:@"p_voir" :RB_STATE_STANDARD :self :@selector(read)];
		if(read != nil)
		{
			[read.cell setActiveAllowed:NO];
			[self addSubview:read];
		}
	}
	
	if(remove == nil)
	{
		remove = [RakButton allocImageWithoutBackground:@"p_X" :RB_STATE_STANDARD :self :@selector(remove)];
		if(remove != nil)
		{
			[remove.cell setActiveAllowed:NO];
			[self addSubview:remove];
		}
	}
	
	if(download == nil)
	{
		download = [RakButton allocImageWithoutBackground:@"p_dl" :RB_STATE_STANDARD :self :@selector(download)];
		if(download != nil)
		{
			[download setHidden:YES];
			[download.cell setActiveAllowed:NO];
			[self addSubview:download];
		}
	}
}

#pragma mark - UI management

- (void) frameChanged : (NSSize) newSize
{
	[super frameChanged:newSize];
	
	[title setFrameOrigin:NSMakePoint([self titleX], OFFSET_Y_TITLE)];
	[repo setFrameOrigin:NSMakePoint([self titleX], OFFSET_Y_REPO)];
	
	[remove setFrameOrigin:NSMakePoint(newSize.width - 10 - remove.bounds.size.width, newSize.height / 2 - remove.bounds.size.height / 2)];
	[read setFrameOrigin:NSMakePoint(remove.frame.origin.x - 10 - read.bounds.size.width, newSize.height / 2 - read.bounds.size.height / 2)];
	[refresh setFrameOrigin:NSMakePoint(read.frame.origin.x - 10 - refresh.bounds.size.width, newSize.height / 2 - refresh.bounds.size.height / 2)];
	[download setFrameOrigin:NSMakePoint(refresh.frame.origin.x, newSize.height / 2 - download.bounds.size.height / 2)];
}

- (NSColor *) textColor
{
	return [self detailTextColor];
}

#pragma mark - Responder

- (void) clickRepo
{
	uint mainThread;
	[Prefs getPref:PREFS_GET_MAIN_THREAD :&mainThread];
	
	if(mainThread != TAB_SERIES)
	{
		[RakTabView broadcastUpdateFocus:TAB_SERIES];
		
		dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(ANIMATION_DURATION_LONG * 1.5 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
			[self clickRepo];
		});
	}
	else
	{
		[[(RakAppDelegate *) [NSApp delegate] window] makeKeyAndOrderFront:nil];
		[[NSNotificationCenter defaultCenter] postNotificationName:SR_NOTIFICATION_FLUSH object:nil];
		[[NSNotificationCenter defaultCenter] postNotificationName:SR_NOTIFICATION_SOURCE object:getStringForWchar(_project.repo->name) userInfo:@{SR_NOTIF_CACHEID : @(getRepoID(_project.repo)), SR_NOTIF_OPTYPE : @(YES)}];
	}
}

- (void) refresh
{
	//This code should only be initiated from the main thread, not a problem if it comes from the event queue
	if(![NSThread isMainThread] && !refreshing)
	{
		dispatch_sync(dispatch_get_main_queue(), ^{
			[self refresh];
		});
		return;
	}
	
	//No concurent refreshing
	if(refreshing)
		return;
	
	refreshing = YES;
	uint currentID = _project.cacheDBID;
	
	[refresh.cell setState : RB_STATE_HIGHLIGHTED];
	
	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
		
		refreshRepo(_project.repo);
		
		dispatch_async(dispatch_get_main_queue(), ^{
			
			if(currentID == _project.cacheDBID)
				[self refreshCallback];
	
			[refresh.cell setState : RB_STATE_STANDARD];
			refreshing = NO;
		});
	});
}

- (void) refreshCallback
{
	
	NSLog(@"Yay");
}

- (void) read
{
	[[self class] readProject:_project];
}

+ (void) readProject : (PROJECT_DATA) project
{
	[RakTabView broadcastUpdateContext:nil :project :NO :VALEUR_FIN_STRUCT];
	[RakTabView broadcastUpdateFocus:TAB_CT];
	[[(RakAppDelegate *) [NSApp delegate] window] makeKeyAndOrderFront:nil];
}

- (void) remove
{
	setFavorite(&_project);
}

- (void) download
{
	
}

@end