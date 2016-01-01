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

@interface Reader()
{
	NSIndexPath * indexToApply, * lastInsertedToCache;
	
	BOOL isFullscreen;
	uint nbCT;
	NSMutableDictionary * metadataArray;
	NSMutableDictionary * mainCache;
	BOOL goingUp;
	
	CGFloat _cachedHeight, headerHeight, footerHeight;
	CGSize containerSize;
}

@end

@interface RakUITableViewCell : UITableViewCell
@property CGFloat submittedHeight;
@end

@implementation RakUITableViewCell
@end

@implementation Reader

- (void) awakeFromNib
{
	[super awakeFromNib];
	
	self.initWithNoContent = YES;
	self.hidesBottomBarWhenPushed = YES;
	
	metadataArray = [NSMutableDictionary dictionary];
	mainCache = [NSMutableDictionary dictionary];
	
	tabBarIndex = 2;
	[RakApp registerReader:self];

}

- (void) viewDidLoad
{
	[super viewDidLoad];
	
	headerHeight = _navigationBar.bounds.size.height + 20;
	footerHeight = [RakApp tabBarController].tabBar.bounds.size.height;

	containerSize = [UIScreen mainScreen].bounds.size;
	
	_tableView.rowHeight = UITableViewAutomaticDimension;
	_tableView.estimatedRowHeight = 600;
	
	[self updateTableViewFrame];
}

- (void) viewWillFocus
{
	if(indexToApply != nil)
	{
		[_tableView scrollToRowAtIndexPath:indexToApply atScrollPosition:UITableViewScrollPositionTop animated:YES];
		indexToApply = nil;
	}
}

- (void) flushCache
{
	[metadataArray enumerateKeysAndObjectsUsingBlock:^(id  _Nonnull key, NSData *  _Nonnull obj, BOOL * _Nonnull stop)
	{
		DATA_LECTURE data;
		[obj getBytes:&data length:sizeof(DATA_LECTURE)];
		releaseDataReader(&data);
	}];
}

#pragma mark - Context update

- (BOOL) startReading : (PROJECT_DATA) project : (uint) elemToRead : (BOOL) isTome : (uint) startPage
{
	[super startReading:project :elemToRead :isTome :startPage];
	
	BOOL shouldNotifyBottomBarInitialized = [super startReading:project :elemToRead :isTome :startPage];
	
	indexToApply = [NSIndexPath indexPathForItem:0 inSection:_posElemInStructure];
	
	return shouldNotifyBottomBarInitialized;
}

- (void) changeProject:(PROJECT_DATA)projectRequest :(uint)elemRequest :(BOOL)isTomeRequest :(uint)startPage
{
	[super changeProject:projectRequest :elemRequest :isTomeRequest :startPage];
	[_tableView reloadData];
}

#pragma mark - Table view

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
	nbCT = 0;

	if(!initialized)
		return nbCT;
	
	return (NSInteger) (nbCT += ACCESS_DATA(self.isTome, _project.nbChapterInstalled, _project.nbVolumesInstalled));
}

- (NSString *)tableView:(UITableView *)tableView titleForFooterInSection:(NSInteger)section
{
	//Lowercase the first later
	NSString * string = [self tableView:tableView titleForHeaderInSection:section];
	
	string = [string stringByReplacingCharactersInRange:NSMakeRange(0, 1) withString:[string substringToIndex:1].lowercaseString];
	
	return [NSString localizedStringWithFormat:NSLocalizedString(@"READER-END-OF-%@", nil), string];
}

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section
{
	if(section >= nbCT)
		return nil;

	if(self.isTome)
		return getStringForVolumeFull(_project.volumesInstalled[section]);
	
	return getStringForChapter(_project.chaptersInstalled[section]);
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
	if(section >= nbCT)
		return 0;

	DATA_LECTURE data;
	uint currentID = ACCESS_DATA(self.isTome, _project.chaptersInstalled[section], _project.volumesInstalled[section].ID);
	NSNumber * sectionID = @(currentID);
	NSData * metadata = [metadataArray objectForKey:sectionID];
	if(metadata == nil)
	{
		if(!readerConfigFileLoader(_project, self.isTome, currentID, &data))
		{
			NSLog(@"Couldn't load the chapter data");
			return 0;
		}
		
		metadata = [NSData dataWithBytes:&data length:sizeof(DATA_LECTURE)];
		if(metadata == nil)
			return 0;
		
		[metadataArray setObject:metadata forKey:sectionID];
	}
	else
	{
		[metadata getBytes:&data length:sizeof(DATA_LECTURE)];
	}
	
	return data.nbPage;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
	BOOL didHitCache;
	UIImageView * imageView = [self loadToCacheForCT:indexPath.section andPage:indexPath.row inCache:&didHitCache];
	if(imageView == nil)
		return nil;
	
	if(!didHitCache && !_cacheBeingBuilt)
	{
		[self startBuildingCache];
	}
	
	//We have to scale the scrollview to this size;
	CGSize size = imageView.bounds.size;
	CGFloat cellWidth = containerSize.width, ratio = size.width / cellWidth;
	CGRect frame = CGRectMake(0, 2, cellWidth, 4 + size.height / ratio);

	[imageView setFrame:frame];
	_cachedHeight = frame.size.height;

	RakUITableViewCell * cell = [tableView dequeueReusableCellWithIdentifier:@"ReaderRow" forIndexPath:indexPath];
	if(cell == nil)
		return nil;
	
	cell.submittedHeight = _cachedHeight;
	
	frame.size.height += 2 * frame.origin.y;		frame.origin.y = 0;
	frame.size.width += 2 * frame.origin.x;			frame.origin.x = 0;

	cell.frame = frame;
	cell.contentView.frame = frame;
	
	NSArray * subviews = [cell.contentView.subviews copy];
	for(UIView * view in subviews)
		[view removeFromSuperview];
	
	[cell.contentView addSubview:imageView];

	//Hack there because the cell would occasionaly only present a white area, instead of the image :(
	//Various -setNeedDisplay couldn't fix the issue, but if you know better, great, ensure that hitting cache display the picture
	cell.selected = YES;
	cell.selected = NO;
	
	return cell;
}

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath
{
	if(_cachedHeight == 0)
	{
		RakUITableViewCell * cell = [tableView cellForRowAtIndexPath:indexPath];
		if(cell != nil)
			return cell.submittedHeight;
	}
	
	return _cachedHeight;
}

- (BOOL)tableView:(UITableView *)tableView shouldHighlightRowAtIndexPath:(NSIndexPath *)indexPath
{
	return NO;
}

- (void) viewWillTransitionToSize:(CGSize)size withTransitionCoordinator:(id<UIViewControllerTransitionCoordinator>)coordinator
{
	containerSize = size;
	
	NSArray <NSIndexPath *> * indexes = _tableView.indexPathsForVisibleRows;
	
	NSIndexPath * path = [indexes count] == 0 ? nil : indexes[indexes.count / 2];
	
	[coordinator animateAlongsideTransition:^(id<UIViewControllerTransitionCoordinatorContext>  _Nonnull context) {
		[self fullScreenAnimation];
		[_tableView reloadData];
	} completion:^(id<UIViewControllerTransitionCoordinatorContext>  _Nonnull context) {
		if(path != nil)
			[_tableView scrollToRowAtIndexPath:path atScrollPosition:UITableViewScrollPositionTop animated:NO];
	}];
	
	[super viewWillTransitionToSize:size withTransitionCoordinator:coordinator];
}

#pragma mark - Caching

- (void) didReceiveMemoryWarning
{
	//Purge the cache
	@autoreleasepool {
		[mainCache removeAllObjects];
	}
}

- (void) startBuildingCache
{
	uint localCacheSession = ++cacheSession;
	dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
		if(localCacheSession == cacheSession)
			[self buildCache];
	});
}

- (void) buildCache
{
	uint session = cacheSession;

	_cacheBeingBuilt = YES;
	while(session == cacheSession)
	{
		if(lastInsertedToCache == nil)
			lastInsertedToCache = [[_tableView indexPathsForVisibleRows] firstObject];

		NSIndexPath * indexPath = nil;
		uint currentSection, currentPage;
		do
		{
			if(lastInsertedToCache.section != indexPath.section)
			{
#ifdef EXTENSIVE_LOGGING
				NSLog(@"Invalidated the starting point");
#endif
				currentSection = lastInsertedToCache.section;
				currentPage = lastInsertedToCache.row;
				indexPath = lastInsertedToCache;
			}
		} while(session == cacheSession && [self loadToCacheForCT:currentSection andPage:++currentPage inCache:NULL] != nil);
//		char delta = goingUp ? -1 : 1;
//		
//		for(uint i = 0, currentSection = lastInsertedToCache.section, currentPage = lastInsertedToCache.row; i < 10; i++)
//		{
//			/*if(*/[self loadToCacheForCT:currentSection andPage:currentPage + delta inCache:NULL];// == nil)
//		}
		break;
	}
	_cacheBeingBuilt = NO;
}

- (BOOL) haveSpace
{
#warning "Need implementation"
	return YES;//[mainCache count] < 30;
}

- (void) optimizeStorage
{
#warning "Need implementation"
}

- (UIImageView *) loadToCacheForCT : (uint) CT andPage : (uint) page inCache : (BOOL *) inCache
{
	if(CT >= nbCT)
		return nil;
	
	DATA_LECTURE data;
	uint currentID = ACCESS_DATA(self.isTome, _project.chaptersInstalled[CT], _project.volumesInstalled[CT].ID);
	NSNumber * sectionID = @(currentID);
	NSData * metadata = [metadataArray objectForKey:sectionID];
	if(metadata == nil)
	{
		if(!readerConfigFileLoader(_project, self.isTome, currentID, &data))
		{
			NSLog(@"Couldn't load the CT data");
			return nil;
		}
		
		metadata = [NSData dataWithBytes:&data length:sizeof(DATA_LECTURE)];
		if(metadata == nil)
			return nil;
		
		[metadataArray setObject:metadata forKey:sectionID];
	}
	else
	{
		[metadata getBytes:&data length:sizeof(DATA_LECTURE)];
	}
	
	if(page >= data.nbPage)
		return nil;
	
	NSString * cacheString = [NSString stringWithFormat:@"%d-%d", currentID, page];
	UIImageView * imageView = [mainCache objectForKey:cacheString];
	if(imageView == nil)
	{
#ifdef EXTENSIVE_LOGGING
		if([NSThread isMainThread])
		{
			NSLog(@"Main thread missed the cache!");
		}
		else
		{
			NSLog(@"Cache building in the background");
		}
#endif

		imageView = [self getImage:page :&data :NULL];
		if(imageView == nil)
			return nil;
		
		if(inCache != nil)
			*inCache = NO;
		
		if([NSThread isMainThread])
		{
			lastInsertedToCache = [NSIndexPath indexPathForRow:page inSection:CT];

			if(lastInsertedToCache != nil)
				goingUp = lastInsertedToCache.section < CT || lastInsertedToCache.row < page;
		}
		
		[mainCache setObject:imageView forKey:cacheString];
	}
	else if(inCache != nil)
		*inCache = YES;
	
	return imageView;
}

- (void)tableView:(UITableView *)tableView didEndDisplayingCell:(UITableViewCell *)cell forRowAtIndexPath:(NSIndexPath *)indexPath
{
	if(_cacheBeingBuilt)
		return;
	
	[self optimizeStorage];
	if([self haveSpace])
		[self startBuildingCache];
}

#pragma mark - Toggle fullscreen

- (void) updateTableViewFrame
{
	CGRect frame;
	
	if(!isFullscreen)
		frame = CGRectMake(0, headerHeight, containerSize.width, containerSize.height - footerHeight - headerHeight);
	else
		frame = CGRectMake(0, 0, containerSize.width, containerSize.height);
	
	_tableView.frame = frame;
}

- (IBAction)tapDetected : (id) sender
{
	CGRect frame = _tableView.frame;
	
	isFullscreen = !isFullscreen;
	[self setNeedsStatusBarAppearanceUpdate];
	
	_tableView.frame = frame;
	
	[UIView animateWithDuration:0.3 animations:^{	[self fullScreenAnimation];	}];
}

- (void) fullScreenAnimation
{
	UITabBar * bar = [RakApp tabBarController].tabBar;
	CGRect barFrame = bar.frame, navigationFrame = _navigationBar.frame;
	
	if(!isFullscreen)
	{
		navigationFrame.origin.y = 22;
		barFrame.origin.y = containerSize.height - barFrame.size.height;
	}
	else
	{
		navigationFrame.origin.y = -navigationFrame.size.height;
		barFrame.origin.y = containerSize.height;
	}
	
	[self updateTableViewFrame];
	
	bar.frame = barFrame;
	_navigationBar.frame = navigationFrame;
}

- (BOOL)prefersStatusBarHidden
{
	return isFullscreen;
}

- (BOOL)hidesBottomBarWhenPushed
{
	return YES;
}

@end
