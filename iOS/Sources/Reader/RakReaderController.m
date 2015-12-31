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
	NSIndexPath * indexToApply;
	
	BOOL isFullscreen;
	uint nbCT;
	NSMutableDictionary * metadataArray;
	NSMutableDictionary * mainCache;
	
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
	uint CT = (NSUInteger) indexPath.section;
	
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
			NSLog(@"Couldn't load the chapter data");
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
	
	uint page = (NSUInteger) indexPath.row;
	if(page >= data.nbPage)
		return nil;
	
	NSString * cacheString = [NSString stringWithFormat:@"%d-%d", currentID, page];
	UIImageView * imageView = [mainCache objectForKey:cacheString];
	if(imageView == nil)
	{
		imageView = [self getImage:page :&data :NULL];
		if(imageView == nil)
			return nil;
		
		[mainCache setObject:imageView forKey:cacheString];
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
	[[[cell.contentView subviews] lastObject] removeFromSuperview];
	[cell.contentView addSubview:imageView];
	
	frame = cell.frame;
	
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
	
	[coordinator animateAlongsideTransition:^(id<UIViewControllerTransitionCoordinatorContext>  _Nonnull context) {
		[self fullScreenAnimation];
		[_tableView reloadData];
	} completion:^(id<UIViewControllerTransitionCoordinatorContext>  _Nonnull context) {}];
	
	[super viewWillTransitionToSize:size withTransitionCoordinator:coordinator];
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
