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

@implementation MDL (iOS)

- (void) awakeFromNib
{
	[super awakeFromNib];
	
	self.initWithNoContent = YES;
	tabBarIndex = 2;
	[RakApp registerMDL:self];
	
	[self initContent : RakRealApp.savedContext[3]];
}

- (void) viewDidLoad
{
	[super viewDidLoad];
}

#pragma mark - Tableview

- (NSInteger) numberOfSectionsInTableView:(UITableView *)tableView
{
	return 1;
}

- (NSInteger) tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
	if(_tableView == nil)		_tableView = tableView;
	
	uint nbRows = [controller getNbElem:YES];
	
	if(self.initWithNoContent && nbRows != 0)		self.initWithNoContent = NO;

	return (NSInteger) nbRows;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
	uint pos = (NSUInteger) indexPath.row;
	
	DATA_LOADED ** todoList = [controller getData : pos : YES];
	if(todoList == NULL || *todoList == NULL)
		return nil;

	UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"MDLRandoCell" forIndexPath:indexPath];
	if(cell == nil)
		return nil;

	(*todoList)->rowViewResponsible = (__bridge void *)(@(pos));
	cell.textLabel.text = [self labelTextForData:*todoList];
	
	return cell;
}

- (NSString *) labelTextForData : (DATA_LOADED *) data
{
	NSString * localized;
	if(data->listChapitreOfTome != NULL)
	{
		META_TOME tome;
		
		tome.ID = data->identifier;
		tome.readingID = data->tomeID;
		wstrncpy(tome.readingName, MAX_TOME_NAME_LENGTH, data->tomeName);
		tome.readingName[MAX_TOME_NAME_LENGTH] = 0;
		
		localized = getStringForVolumeFull(tome);
	}
	else
		localized = getStringForChapter(data->identifier);
	
	return [NSString stringWithFormat:@"%@ - %@", getStringForWchar(data->datas->projectName), localized];
}

- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath
{
	if (editingStyle == UITableViewCellEditingStyleDelete)
	{
		//	    [self.objects removeObjectAtIndex:(NSUInteger) indexPath.row];
		[tableView deleteRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationFade];
	}
}

#pragma mark - Responder

- (void) refresh
{
	if(![NSThread isMainThread])
		return [self performSelectorOnMainThread:@selector(refresh) withObject:nil waitUntilDone:YES];
	
	[_tableView reloadData];
}

- (void) rowUpdate : (uint) row
{
	if(![NSThread isMainThread])
		return dispatch_sync(dispatch_get_main_queue(), ^{	[self rowUpdate:row];	});
	
	UITableViewCell * cell = [_tableView cellForRowAtIndexPath : [NSIndexPath indexPathForRow:(NSInteger) row inSection:0]];
	
	if(cell != nil)
	{
		cell.textLabel.text = [self labelTextForData:*[controller getData : row : YES]];
		[cell setNeedsDisplay];
	}
	
}

- (void) percentageUpdate : (float) percentage atSpeed : (size_t) speed forObject : (NSNumber *) rowNumber
{
	if(![NSThread isMainThread])
		return dispatch_sync(dispatch_get_main_queue(), ^{	[self percentageUpdate:percentage atSpeed:speed forObject:rowNumber];	});

	NSUInteger row = [rowNumber unsignedIntegerValue];
	UITableViewCell * cell = [_tableView cellForRowAtIndexPath : [NSIndexPath indexPathForRow:(NSInteger) row inSection:0]];
	
	if(cell != nil)
	{
		cell.textLabel.text = [NSString stringWithFormat:@"%@ - %.2f%% - %@", [self labelTextForData:*[controller getData : row : YES]], percentage,
							   [NSString stringWithFormat:@"%@/s", [NSByteCountFormatter stringFromByteCount:(int64_t) speed countStyle:NSByteCountFormatterCountStyleBinary]]];
		[cell setNeedsDisplay];
	}
}

@end