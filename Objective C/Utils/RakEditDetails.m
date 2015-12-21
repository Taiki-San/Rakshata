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

@implementation RakEditDetails

- (instancetype) initWithProject : (PROJECT_DATA) project
{
	self = [self autoInitWithMetadata : project];
	
	[Prefs registerForChange:self forType:KVO_THEME];
	
	if(self != nil)
		[self setupUIMetadata];

	return self;
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] == [Prefs class] && [keyPath isEqualToString:KVO_THEME])
		coreview.backgroundColor = [self backgroundColor];
}

- (void) dealloc
{
	[Prefs deRegisterForChange:self forType:KVO_THEME];
}

#pragma mark - Create the UI

- (NSString *) stringTitleMetadata
{
	return NSLocalizedString(@"PROJ-MENU-EDIT-SERIE", nil);
}

- (void) queryUser
{
	if(![NSThread isMainThread])
	{
		[self performSelectorOnMainThread:@selector(queryUser) withObject:nil waitUntilDone:NO];
		return;
	}
	
	queryWindow = [[RakSheetWindow alloc] initWithContentRect:(NSRect) {{200, 200}, _bounds.size} styleMask:0 backing:NSBackingStoreBuffered defer:YES];
	if(queryWindow != nil)
	{
		coreview = [[RakSheetView alloc] initWithFrame:_bounds];
		if(coreview != nil)
		{
			coreview.anchor = self;
			coreview.backgroundColor = [self backgroundColor];
			[coreview addSubview:self];
			
			queryWindow.backgroundColor = [RakColor clearColor];
			queryWindow.contentView = coreview;
				
			[[(RakAppDelegate *) [NSApp delegate] window] beginSheet:queryWindow completionHandler:^(NSModalResponse returnCode) {}];
		}
	}
}

#pragma mark - Color

- (RakColor *) backgroundColor
{
	return [Prefs getSystemColor:COLOR_EXPORT_BACKGROUND];
}

#pragma mark - Control

- (void) close
{
	[[(RakAppDelegate *) NSApp.delegate window] endSheet:queryWindow];
}

- (void) validateMetadata
{
	NSArray * array;
	
	PROJECT_DATA_PARSED parsedProject = getParsedProjectByID(_project.cacheDBID);
	if(!parsedProject.project.isInitialized)
		return [self close];
	
	_project = parsedProject.project;

	PROJECT_DATA project = [self exfilterProject:&array];
	if(project.isInitialized)
	{
		//Install the overriden images
		for(NSDictionary * dict in array)
		{
			byte retinaCode = [[dict objectForKey:@"code"] unsignedCharValue], standardCode = retinaCode - retinaCode % 2;
			RakImage * baseImage = [dict objectForKey:@"data"];
			
			NSSize retinaSize = thumbSizeForID(retinaCode), standardSize = thumbSizeForID(standardCode);
			
			exportImageToPath(baseImage, standardSize, retinaSize, [NSString stringWithUTF8String:getPathToIconsOfProject(project, retinaCode).string]);
			exportImageToPath(baseImage, standardSize, standardSize, [NSString stringWithUTF8String:getPathToIconsOfProject(project, standardCode).string]);
		}
		parsedProject.project = project;
		updateCache(parsedProject, RDB_UPDATE_ID, 0);
		syncCacheToDisk(SYNC_PROJECTS);
		notifyUpdateProject(project);
	}
	
	releaseParsedData(parsedProject);
	[self close];
}

@end