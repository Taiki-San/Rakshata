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
	
	tabBarIndex = 1;
	[RakApp registerCT:self];
}

- (void) viewDidLoad
{
    [super viewDidLoad];
}

#pragma mark - Context notification

- (void) updateContextNotification : (PROJECT_DATA) project : (BOOL) isTome : (uint) element
{
	if(!project.isInitialized)
		return;

	_tabBarItem.title = getStringForWchar(project.projectName);
}

@end
