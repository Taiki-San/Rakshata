/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at https://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

@interface Reader()
{
	NSIndexPath * indexToApply, * lastItemRequested;
	
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
	[self sharedInit];
	
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
	
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(refreshViewsIfActive) name:UIApplicationWillEnterForegroundNotification object:nil];
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(appWillResignActive) name:UIApplicationWillResignActiveNotification object:nil];
	
	headerHeight = _navigationBar.bounds.size.height + 20;
	footerHeight = [RakApp tabBarController].tabBar.bounds.size.height;

	containerSize = [UIScreen mainScreen].bounds.size;
	
	_tableView.rowHeight = UITableViewAutomaticDimension;
	_tableView.estimatedRowHeight = 600;
	
	 [[UILabel appearanceWhenContainedInInstancesOfClasses:@[[UITableViewHeaderFooterView class]]] setTextAlignment:NSTextAlignmentCenter];
	
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

- (void) refreshViewsIfActive
{
	if([RakApp tabBarController].selectedViewController == self)
	{
		if(isFullscreen)
		{
			[UIView animateWithDuration:0.01 animations:^{
				[self fullScreenAnimation];
			}];
		}
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
	
	indexToApply = [NSIndexPath indexPathForItem:0 inSection:(NSInteger) _posElemInStructure];
	
	return shouldNotifyBottomBarInitialized;
}

- (void) changeProject:(PROJECT_DATA)projectRequest :(uint)elemRequest :(BOOL)isTomeRequest :(uint)startPage
{
	[super changeProject:projectRequest :elemRequest :isTomeRequest :startPage];
	[_tableView reloadData];
}

- (void) didTransition
{
	lastItemRequested = nil;	//Force the cache to check the image actually being displayed
	goingUp = NO;
}

#pragma mark - Table view

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
	nbCT = 0;

	if(!initialized)
		return (NSInteger) nbCT;
	
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
	if((uint) section >= nbCT)
		return nil;

	if(self.isTome)
		return getStringForVolumeFull(_project.volumesInstalled[section]);
	
	return getStringForChapter(_project.chaptersInstalled[section]);
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
	if((uint) section >= nbCT)
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
	
	return (NSInteger) data.nbPage;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
	UIImageView * imageView = [self loadToCacheForCT:(uint) indexPath.section andPage:(uint) indexPath.row inCache:NULL];
	if(imageView == nil)
		return nil;
	
	if(!_cacheBeingBuilt)
	{
		[self startBuildingCache];
	}
	
	if(lastItemRequested != nil)
		goingUp = lastItemRequested.section > indexPath.section || lastItemRequested.row > indexPath.row;
	lastItemRequested = [indexPath copy];
	
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
	NSLog(@"Received memory warning!");
	//Purge the cache
	@autoreleasepool
	{
		[self purgeCache:YES];
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

	BOOL didHitEndOfCache = NO;
	_cacheBeingBuilt = YES;
	while(session == cacheSession && !didHitEndOfCache)
	{
		if(lastItemRequested == nil)
			lastItemRequested = [[_tableView indexPathsForVisibleRows] firstObject];
		
		//currentIndexPath != lastItemRequested => we changed the page
		NSIndexPath * currentIndexPath = lastItemRequested;

		//We first load the next and previous five pages
		int offset;
		BOOL loadedAnything = NO, inCache;
		for(uint i = 0; i < 10 && currentIndexPath == lastItemRequested && session != cacheSession; i++)
		{
			//Offset = i / 2
			offset = 1 + (i >> 1);

			//We move in the opposite direction if i is odd
			if(goingUp ^ ((i & 1) != 0))
				offset *= -1;

			//loadToCacheForIndex is fairly cheap when hitting the cache, so not a big deal to hit it too often
			if([self loadToCacheForIndex:[self indexPath:currentIndexPath withOffset:offset] inCache:&inCache] != nil)
				loadedAnything |= !inCache;
		}

		//If we need to loop back
		if(loadedAnything || session != cacheSession || currentIndexPath != lastItemRequested)
			continue;
		
		//If we removed stuffs, we have a quick look we don't have to add something from the main loop
		if([self purgeCache:NO])
			continue;
		
		//Well, we're good! (main exit point)
		if(![self haveSpace])
		{
			didHitEndOfCache = YES;
		}
		else if(currentIndexPath == lastItemRequested)
		{
			uint count = 0;
			char direction = goingUp ? -1 : 1;
			offset = direction;
			
			for(; count < 20 && [self haveSpace] && currentIndexPath == lastItemRequested; count++)
			{
				NSIndexPath * workingIndexPath = [self indexPath:currentIndexPath withOffset:offset];
				//Hit the end of what we had to cache for now
				if(workingIndexPath == nil)
				{
					didHitEndOfCache = YES;
					break;
				}
				
				[self loadToCacheForIndex:workingIndexPath inCache:NULL];
				offset += direction;
			}
			
			if(count == 20)
				didHitEndOfCache = YES;
		}
	}
	_cacheBeingBuilt = NO;
}

- (BOOL) haveSpace
{
	return [mainCache count] < 40;
}

- (BOOL) purgeCache : (BOOL) fully
{
	if(mainCache.count == 0)
		return NO;
	
	if(fully)
	{
		@autoreleasepool {
			[mainCache removeAllObjects];
		}
		return YES;
	}

	char direction = goingUp ? -1 : 1;
	NSIndexPath * baseIndexPage = lastItemRequested;
	if(baseIndexPage == nil)
		return [self purgeCache:YES];

	uint rangeToCheck = 20 + 1;
	NSIndexPath * movingIndex = nil, *currentMovingIndex = baseIndexPage;
	//We want to accept as many images in the other direction
	for(byte i = 0; i < 20 && currentMovingIndex != nil; i++)
	{
		currentMovingIndex = [self indexPath:currentMovingIndex withOffset:-direction];
		if(currentMovingIndex != nil)
		{
			movingIndex = currentMovingIndex;
			rangeToCheck++;
		}
	}
	
	if(movingIndex == nil)
		movingIndex = baseIndexPage;
	
	NSMutableArray * validRange = [NSMutableArray array];
	for(uint i = 0, section = INVALID_VALUE, CTID; i < rangeToCheck; i++)
	{
		if(section != (uint) movingIndex.section)
		{
			section = (uint) movingIndex.section;
			CTID = ACCESS_DATA(self.isTome, _project.chaptersInstalled[section], _project.volumesInstalled[section].ID);
		}
		
		[validRange addObject:[self cacheStringForPage:(uint) movingIndex.row inID:CTID]];
		movingIndex = [self indexPath:movingIndex withOffset:direction];
	}
	
	NSMutableArray * allKeys = mainCache.allKeys.mutableCopy;
	[allKeys removeObjectsInArray:validRange];
	
	@autoreleasepool
	{
		[allKeys enumerateObjectsUsingBlock:^(id  _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
			
			UIImageView * view = [mainCache objectForKey:obj];
			if(view != nil && view.superview != nil)
				[view removeFromSuperview];

			[mainCache removeObjectForKey:obj];
		}];
	}
	
	return allKeys.count != 0;
}

- (UIImageView *) loadToCacheForIndex : (NSIndexPath *) indexPath inCache : (BOOL *) inCache
{
	if(indexPath == nil)
		return nil;
	
	return [self loadToCacheForCT:(uint) indexPath.section andPage:(uint) indexPath.row inCache:inCache];
}

- (UIImageView *) loadToCacheForCT : (uint) CT andPage : (uint) page inCache : (BOOL *) inCache
{
	DATA_LECTURE data;
	NSData * metadata = [self getMetadataForSection:CT];
	if(metadata == nil)
		return nil;
	
	[metadata getBytes:&data length:sizeof(DATA_LECTURE)];
	
	if(page >= data.nbPage)
		return nil;
	
	NSString * cacheString = [self cacheStringForPage:page inID:ACCESS_DATA(self.isTome, _project.chaptersInstalled[CT], _project.volumesInstalled[CT].ID)];
	UIImageView * imageView = [mainCache objectForKey:cacheString];
	if(imageView == nil)
	{
#ifdef EXTENSIVE_LOGGING
		NSLog(@"Loading %d (from %d) from %@", page, CT, [NSThread isMainThread] ? @"main thread" : @"caching mechanism");
#endif

		imageView = [self getImage:page :&data :NULL];
		if(imageView == nil)
			return nil;
		
		if(inCache != nil)
			*inCache = NO;
		
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
	
	if([self haveSpace])
		[self startBuildingCache];
}

//We suspend the caching work
- (void) appWillResignActive
{
	if(_cacheBeingBuilt)
		cacheSession++;
}

#pragma mark - Index path utils

- (NSIndexPath *) indexPath :(NSIndexPath *) indexPath withOffset : (int) _offset
{
	BOOL goingBack = _offset < 0;
	uint offset = (uint) (goingBack ? -_offset : _offset), section = (uint) indexPath.section;
	DATA_LECTURE data;
	
	if(goingBack)
	{
		//Going back, but still in the section
		if((uint) indexPath.row >= offset)
			return [NSIndexPath indexPathForRow:indexPath.row + _offset inSection:(int) section];
		
		//We move back from section to section all the way back to zero
		offset -= (uint) indexPath.row;
		while(section-- != 0)
		{
			NSData * metadata = [self getMetadataForSection:section];
			if(metadata == nil)
				return nil;
			
			[metadata getBytes:&data length:sizeof(DATA_LECTURE)];
			
			if(data.nbPage >= offset)
			{
				data.nbPage -= offset;
				return [NSIndexPath indexPathForRow:(int) data.nbPage inSection: (int) section];
			}
			
			offset -= data.nbPage;
		}
	}
	else
	{
		NSData * metadata = [self getMetadataForSection:section];
		if(metadata == nil)
			return nil;
		
		[metadata getBytes:&data length:sizeof(DATA_LECTURE)];

		//Moving forward, but still in the section
		if((uint) indexPath.row + offset < data.nbPage)
			return [NSIndexPath indexPathForRow:indexPath.row + _offset inSection:indexPath.section];
		
		offset -= (data.nbPage - (uint) indexPath.row);
		while(++section < nbCT)
		{
			metadata = [self getMetadataForSection:section];
			if(metadata == nil)
				return nil;
			
			[metadata getBytes:&data length:sizeof(DATA_LECTURE)];
			
			if(data.nbPage >= offset)
				return [NSIndexPath indexPathForRow:(NSInteger) offset inSection:(NSInteger) section];
			
			offset -= data.nbPage;
		}
	}
	
	return nil;
}

- (NSData *) getMetadataForSection : (uint) section
{
	if(section >= nbCT)
		return nil;
	
	uint currentID = ACCESS_DATA(self.isTome, _project.chaptersInstalled[section], _project.volumesInstalled[section].ID);
	NSNumber * sectionID = @(currentID);
	NSData * metadata = [metadataArray objectForKey:sectionID];
	if(metadata == nil)
	{
		DATA_LECTURE data;
		if(!readerConfigFileLoader(_project, self.isTome, currentID, &data))
		{
			NSLog(@"Couldn't load the CT data");
			return nil;
		}
		
		metadata = [NSData dataWithBytes:&data length:sizeof(DATA_LECTURE)];
		if(metadata != nil)
			[metadataArray setObject:metadata forKey:sectionID];
	}
	
	return metadata;
}

- (NSString *) cacheStringForPage : (uint) page inID : (uint) ID
{
	return [NSString stringWithFormat:@"%d-%d", ID, page];
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

- (BOOL) prefersStatusBarHidden
{
	return isFullscreen;
}

- (BOOL)hidesBottomBarWhenPushed
{
	return YES;
}

@end
