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

@implementation CTSelec

- (void) awakeFromNib
{
	[super awakeFromNib];

	self.initWithNoContent = YES;
	tabBarIndex = 1;
	[RakApp registerCT:self];
}

- (void) viewDidLoad
{
    [super viewDidLoad];
	
	[_segmentedControl addTarget:self action:@selector(isTomeToggled) forControlEvents:UIControlEventValueChanged];
}

#pragma mark - Context notification

- (void) updateContextNotification : (PROJECT_DATA) project : (BOOL) isTome : (uint) element
{
	if(!project.isInitialized && (project.nbChapter != 0 || project.nbVolumes != 0))
		return;
	else if(self.initWithNoContent)
		self.initWithNoContent = NO;
	
	_project = project;
	_tabBarItem.title = getStringForWchar(project.projectName);
	
	[self ensureValidSegmentedControlState];
	[self _isTomeToggledWithAnimation:NO];
}

#pragma mark - Context management

- (BOOL) isTome
{
	return _segmentedControl.selectedSegmentIndex == 1;
}

- (void) isTomeToggled
{
	_isTome = self.isTome;
	[self _isTomeToggledWithAnimation:YES];
}

- (void) _isTomeToggledWithAnimation : (BOOL) withAnimation
{
	nbElement = ACCESS_DATA(self.isTome, _project.nbChapter, _project.nbVolumes);
	[_tableView reloadData];
}

#pragma mark - Table View

- (void) ensureValidSegmentedControlState
{
	//If there are both chapters and volumes, that's fine
	BOOL shouldSegmentedControlBeActive = _project.nbChapter != 0 && _project.nbVolumes;
	
	_segmentedControl.enabled = shouldSegmentedControlBeActive;
	
	if(!shouldSegmentedControlBeActive)
	{
		if(_isTome ^ (_project.nbChapter == 0))
		{
			_isTome = !_isTome;
			_segmentedControl.selectedSegmentIndex = _isTome;
		}
	}
}

- (void) tableView:(UITableView *)tableView didSelectRowAtIndexPath : (NSIndexPath *) indexPath
{
	uint contentID = indexPath.row;
	
	if(contentID >= ACCESS_DATA(_isTome, _project.nbChapter, _project.nbVolumes))
		return;
	
	contentID =  ACCESS_CT(_isTome, _project.chaptersFull, _project.volumesFull, contentID);
	
//	[RakTabView broadcastUpdateContext : self : _project : isTome : contentID];
//	[RakApp.CT ownFocus];
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
	return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
	return nbElement;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
	uint pos = (NSUInteger) indexPath.row;
	if(pos >= nbElement)
		return nil;
	
	UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"CTRandoCell" forIndexPath:indexPath];
	if(cell == nil)
		return nil;
	
	cell.textLabel.text = _isTome ? getStringForVolumeFull(_project.volumesFull[pos]) : getStringForChapter(_project.chaptersFull[pos]);
	
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