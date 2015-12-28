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

	savedState[0] = savedState[1] = YES;
}

- (void) viewDidLoad
{
    [super viewDidLoad];
	
	[_segmentedControl setTitle:NSLocalizedString(@"CHAPTERS", nil) forSegmentAtIndex:0];
	[_segmentedControl setTitle:NSLocalizedString(@"VOLUMES", nil) forSegmentAtIndex:1];
	
	if(!savedState[0] || !savedState[1])		[self applyState:savedState];
	
	[_segmentedControl addTarget:self action:@selector(isTomeToggled) forControlEvents:UIControlEventValueChanged];
}

#pragma mark - Context notification

- (void) updateContextNotification : (PROJECT_DATA) project : (BOOL) isTome : (uint) element
{
	if(!project.isInitialized && (project.nbChapter != 0 || project.nbVolumes != 0))
		return;
	else if(self.initWithNoContent)
		self.initWithNoContent = NO;
	
	[self _updateProject:getCopyOfProjectData(project) :isTome :element];
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

- (PROJECT_DATA) activeProject
{
	return _project;
}

#pragma mark - Cross tab communication

- (void) updateProject : (uint) cacheDBID : (BOOL)isTome : (uint) element
{
	[self _updateProject:getProjectByID(cacheDBID) :isTome :element];
}

- (void) _updateProject : (PROJECT_DATA) project : (BOOL) isTome : (uint) element
{
	_project = project;
	releaseCTData(_project);
	_tabBarItem.title = getStringForWchar(project.projectName);
	
	[self ensureValidSegmentedControlState];
	[self _isTomeToggledWithAnimation:NO];
}

- (void) selectElem : (uint) projectID : (BOOL) isTome : (uint) element
{
	
}

#pragma mark - Table View

- (void) ensureValidSegmentedControlState
{
	//If there are both chapters and volumes, that's fine
	BOOL shouldSegmentedControlBeActive = _project.nbChapter && _project.nbVolumes;
	
	if(!shouldSegmentedControlBeActive)
	{
		if(_isTome ^ (_project.nbChapter == 0))
		{
			_isTome = !_isTome;
			_segmentedControl.selectedSegmentIndex = _isTome;
		}

		[self applyState: (BOOL [2]) {!_isTome, _isTome}];
	}
	else
		[self applyState: (BOOL [2]) {YES, YES}];
}

- (void) applyState : (BOOL [2]) state
{
	if(_segmentedControl == nil)
	{
		savedState[0] = state[0];
		savedState[1] = state[1];
	}
	else
	{
		[_segmentedControl setEnabled:state[0] forSegmentAtIndex:0];
		[_segmentedControl setEnabled:state[1] forSegmentAtIndex:1];
	}
}

- (void) tableView:(UITableView *)tableView didSelectRowAtIndexPath : (NSIndexPath *) indexPath
{
	uint contentID = indexPath.row;
	
	if(contentID >= ACCESS_DATA(_isTome, _project.nbChapter, _project.nbVolumes))
		return;
	
	contentID =  ACCESS_CT(_isTome, _project.chaptersFull, _project.volumesFull, contentID);

	if(checkReadable(_project, _isTome, contentID))
	{
		[RakTabView broadcastUpdateContext : self : _project : _isTome : contentID];
//		[RakApp.reader ownFocus];
	}
	else
	{
		[RakApp.MDL proxyAddElement:_project isTome:_isTome element:contentID partOfBatch:NO];
	}
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
