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
	CELL_INTERTEXT_OFFSET = 2
};

@implementation Series

- (void) awakeFromNib
{
	[super awakeFromNib];

	tabBarIndex = 0;
	[RakApp registerSeries:self];
}

- (void) viewDidLoad
{
	[super viewDidLoad];
	
	//Load the project data
	contentManager = [[RakSRContentManager alloc] init];

	_tableView.rowHeight = 66;

	//Setup the tab bar
	UIBarButtonItem *button = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemAdd target:self action:@selector(addTriggered)];
	self.navigationItem.leftBarButtonItem = button;
	
	button = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemSearch target:self action:@selector(searchTriggered)];
	self.navigationItem.rightBarButtonItem = button;
}

//- (void) didReceiveMemoryWarning
//{
//	[super didReceiveMemoryWarning];
//}

#pragma mark - Button responder

- (void) addTriggered
{
	NSLog(@"Want to add something");
}

- (void) searchTriggered
{
	NSLog(@"Trigger search!");
}

#pragma mark - Table View

- (void) tableView:(UITableView *)tableView didSelectRowAtIndexPath : (NSIndexPath *) indexPath
{
	PROJECT_DATA * project = [contentManager getDataAtIndex:contentManager.sharedReference[(uint) indexPath.row].index];

	[RakTabView broadcastUpdateContext : self : *project : NO : INVALID_VALUE];
	[RakApp.CT ownFocus];
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
	return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
	return (NSInteger) [contentManager.sharedReference count];
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
	uint pos = (NSUInteger) indexPath.row;
	if(pos >= [contentManager.sharedReference count])
		return nil;

	UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"SRRandoCell" forIndexPath:indexPath];
	if(cell == nil)
		return nil;
	
	PROJECT_DATA * project = [contentManager getDataAtIndex:contentManager.sharedReference[pos].index];
	
	CGFloat baseX = 15, height = cell.bounds.size.height;

	UIImage * image = loadDDThumbnail(*project);
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
		name.text = getStringForWchar(project->projectName);
		[name sizeToFit];
		
		//Configure the author field
		detail.font = [UIFont systemFontOfSize:[UIFont labelFontSize] - 3];
		detail.textColor = [UIColor colorWithDeviceWhite:0 alpha:0.5];
		detail.text = getStringForWchar(project->authorName);
		
		CGFloat offset = CELL_INTERTEXT_OFFSET;

		//Add the source
		if(_shouldDisplaySource && !isLocalProject(*project) && project->repo != NULL)
		{
			detail.numberOfLines = 2;
			detail.text = [NSString stringWithFormat:@"%@\n%@", detail.text, getStringForWchar(project->repo->name)];
			offset = 0;
		}
		
		[detail sizeToFit];
		
		CGFloat nameHeight = name.bounds.size.height + offset, baseY = round((height - (nameHeight + detail.bounds.size.height)) / 2);
		
		name.frameOrigin = CGPointMake(baseX, baseY);
		[cell.contentView addSubview:name];
		
		baseY += nameHeight;
		
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
