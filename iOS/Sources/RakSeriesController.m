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
	CELL_INTERTEXT_OFFSET = 0
};

@implementation RakSeriesController

- (void)viewDidLoad
{
	[super viewDidLoad];
	
	_tableView.rowHeight = 70;

	//Load the project data
	projects = getCopyCache(RDB_LOADALL | SORT_NAME, &nbProject);

	UIBarButtonItem *addButton = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemAdd target:self action:@selector(addTriggered)];
	self.navigationItem.leftBarButtonItem = addButton;
	
	UIBarButtonItem *searchButton = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemSearch target:self action:@selector(searchTriggered)];
	self.navigationItem.rightBarButtonItem = searchButton;
}

- (void)didReceiveMemoryWarning
{
	[super didReceiveMemoryWarning];
}

#pragma mark - Button responder

- (void) addTriggered
{
	NSLog(@"Want to add something");
}

- (void) searchTriggered
{
	NSLog(@"Trigger search!");
}

#pragma mark - Segues

- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
	if ([[segue identifier] isEqualToString:@"showDetail"])
	{
//	    NSIndexPath *indexPath = [self.tableView indexPathForSelectedRow];
//	    NSDate *object = self.objects[indexPath.row];
//	    DetailViewController *controller = (DetailViewController *)[[segue destinationViewController] topViewController];
//	    [controller setDetailItem:object];
//	    controller.navigationItem.leftBarButtonItem = self.splitViewController.displayModeButtonItem;
//	    controller.navigationItem.leftItemsSupplementBackButton = YES;
	}
}

#pragma mark - Table View

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
	return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
	return nbProject;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
	UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"Cell" forIndexPath:indexPath];
	
	uint pos = (NSUInteger) indexPath.row;
	if(pos >= nbProject)
		return nil;
	
	CGFloat baseX = 15, height = cell.bounds.size.height;

	UIImage * image = loadDDThumbnail(projects[pos]);
	if(image != nil)
	{
		UIImageView * imageView = [[UIImageView alloc] initWithFrame:CGRectMake(baseX, height / 2 - 22.5, 45, 45)];
		if(imageView != nil)
		{
			baseX *= 2;
			baseX += 45;
			
			imageView.image = image;
			imageView.layer.cornerRadius = 22;
			imageView.clipsToBounds = YES;
			
			[cell.contentView addSubview:imageView];
		}
	}
	
	UILabel * name = [[UILabel alloc] init];
	UILabel * detail = [[UILabel alloc] init];
	
	if(name != nil && detail != nil)
	{
		//Configure the name
		name.font = [UIFont boldSystemFontOfSize:[UIFont labelFontSize]];
		name.text = getStringForWchar(projects[pos].projectName);
		[name sizeToFit];
		
		//Configure the author field
		detail.font = [UIFont systemFontOfSize:[UIFont labelFontSize] - 2];
		detail.textColor = [UIColor colorWithDeviceWhite:0 alpha:0.5];
		detail.text = getStringForWchar(projects[pos].authorName);

		//Add the source
		if(!isLocalProject(projects[pos]) && projects[pos].repo != NULL)
		{
			detail.numberOfLines = 2;
			detail.text = [NSString stringWithFormat:@"%@\n%@", detail.text, getStringForWchar(projects[pos].repo->name)];
		}
		
		[detail sizeToFit];
		
		CGFloat cumulatedHeight = name.bounds.size.height + CELL_INTERTEXT_OFFSET + detail.bounds.size.height;
		
		CGFloat margin = round((height - cumulatedHeight) / 2), baseY = margin;
		
		name.frameOrigin = CGPointMake(baseX, baseY);
		[cell.contentView addSubview:name];
		
		baseY += name.bounds.size.height + CELL_INTERTEXT_OFFSET;
		
		detail.frameOrigin = CGPointMake(baseX, baseY);
		[cell.contentView addSubview:detail];
	}
	
	return cell;
}

- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath
{
	if (editingStyle == UITableViewCellEditingStyleDelete)
	{
//	    [self.objects removeObjectAtIndex:(NSUInteger) indexPath.row];
	    [tableView deleteRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationFade];
	}
}

@end
