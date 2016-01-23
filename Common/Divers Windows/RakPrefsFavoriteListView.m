/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

enum
{
	OFFSET_Y_TITLE = 22,
	OFFSET_Y_REPO = 6
};

@implementation RakPrefsFavoriteListView

- (instancetype) initWithProject : (PROJECT_DATA) project andList : (RakPrefsFavoriteList *) mainList
{
	self = [self initWithFrame:NSMakeRect(0, 0, 300, LIST_WIDE_LINE_HEIGHT)];
	
	if(self != nil)
	{
		_mainList = mainList;
		
		[self updateContent:project];
		
		[Prefs registerForChange:self forType:KVO_THEME];
		[RakDBUpdate registerForUpdate:self :@selector(DBUpdate:)];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(refreshCallback) name:NOTIFICATION_FAVS_REPO_UPDATE object:nil];
	}
	
	return self;
}

- (void) dealloc
{
	[Prefs deRegisterForChange:self forType:KVO_THEME];
	[RakDBUpdate unRegister:self];
	[[NSNotificationCenter defaultCenter] removeObserver:self];
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
		repo = [[RakClickableText alloc] initWithText:getRepoName(_project.repo):[self detailTextColor]];
		if(repo != nil)
		{
			repo.ignoreURL = YES;
			repo.clicTarget = self;
			repo.clicAction = @selector(clickRepo);
			
			[self addSubview:repo];
		}
	}
	else
		repo.stringValue = getRepoName(_project.repo);
	
	[repo sizeToFit];
	
	if(refresh == nil)
	{
		refresh = [RakButton allocImageWithoutBackground:@"refresh" :self :@selector(refresh)];
		if(refresh != nil)
		{
			[refresh.cell setActiveAllowed:YES];
			[self addSubview:refresh];
		}
	}
	
	if(read == nil)
	{
		read = [RakButton allocImageWithoutBackground:@"pRead" :self :@selector(read)];
		if(read != nil)
		{
			[read.cell setActiveAllowed:NO];
			[self addSubview:read];
		}
	}
	
	if(remove == nil)
	{
		remove = [RakButton allocImageWithoutBackground:@"pDelete" :self :@selector(remove)];
		if(remove != nil)
		{
			[remove.cell setActiveAllowed:NO];
			[self addSubview:remove];
		}
	}
	
	if(download == nil)
	{
		download = [RakButton allocImageWithoutBackground:@"pDL" :self :@selector(download)];
		if(download != nil)
		{
			[download.cell setActiveAllowed:NO];
			[self addSubview:download];
		}
	}
	
	[self refreshCallback];
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

- (RakColor *) textColor
{
	return [self detailTextColor];
}

#pragma mark - Responder

- (void) clickRepo
{
	if(getMainThread() != TAB_SERIES)
	{
		[RakTabView broadcastUpdateFocus:TAB_SERIES];
		
		dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(ANIMATION_DURATION_LONG * 1.5 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
			[self clickRepo];
		});
	}
	else
	{
		[RakApp.window makeKeyAndOrderFront:nil];
		[[NSNotificationCenter defaultCenter] postNotificationName:SR_NOTIFICATION_FLUSH object:nil];
		[[NSNotificationCenter defaultCenter] postNotificationName:SR_NOTIFICATION_SOURCE object:getRepoName(_project.repo) userInfo:@{SR_NOTIF_CACHEID : @(getRepoID(_project.repo)), SR_NOTIF_OPTYPE : @(YES)}];
	}
}

- (void) read
{
	[[self class] readProject:_project];
}

+ (void) readProject : (PROJECT_DATA) project
{
	[RakTabView broadcastUpdateContext:nil :project :NO :INVALID_VALUE];
	[RakTabView broadcastUpdateFocus:TAB_CT];
	[RakApp.window makeKeyAndOrderFront:nil];
}

- (void) remove
{
	setFavorite(&_project);
}

- (void) download
{
	if(!_project.isInitialized)
		return;
	
	PROJECT_DATA projectPipeline; projectPipeline.isInitialized = false;
	bool isTome;
	uint element;
	
	checkFavoriteUpdate(_project, &projectPipeline, &isTome, &element, false);
	
	if(projectPipeline.isInitialized)
		addElementToMDL(projectPipeline, isTome, element, false);
}

#pragma mark - Refresh management

- (void) DBUpdate : (NSNotification *) notification
{
	if([RakDBUpdate analyseNeedUpdateProject:notification.userInfo :_project])
	{
		[self updateContent:getProjectByID(_project.cacheDBID)];
	}
}

- (void) refresh
{
	//This code should only be initiated from the main thread, not a problem if it comes from the event queue
	if(![NSThread isMainThread])
	{
		dispatch_sync(dispatch_get_main_queue(), ^{
			[self refresh];
		});
		return;
	}
	
	[refresh.cell setState : RB_STATE_HIGHLIGHTED];
	
	//No concurent refreshing
	if(![_mainList tryInsertRepo:getRepoID(_project.repo)])
		return;
	
	refreshing = YES;
	uint currentID = _project.cacheDBID;
	
	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
		
		refreshRepo(_project.repo);
		
		dispatch_async(dispatch_get_main_queue(), ^{
			
			if(currentID != _project.cacheDBID)
				return;
			
			[_mainList removeRepo:getRepoID(_project.repo)];
		});
	});
}

- (void) refreshCallback
{
	[refresh.cell setState : [_mainList isRepoRefreshing:getRepoID(_project.repo)] ? RB_STATE_HIGHLIGHTED : RB_STATE_STANDARD];

	PROJECT_DATA newProject = getProjectByID(_project.cacheDBID);

	if(!newProject.isInitialized)
		return;
	
	releaseCTData(_project);
	_project = newProject;

	BOOL haveFavoriteUpdate = checkFavoriteUpdate(_project, NULL, NULL, NULL, true);
	
	[refresh setHidden:haveFavoriteUpdate];
	[download setHidden:!haveFavoriteUpdate];
}

@end