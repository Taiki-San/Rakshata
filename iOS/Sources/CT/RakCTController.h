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

#import "RakCTRowCell.h"

@interface CTSelec : RakTabView
{
	IBOutlet UITabBarItem * _tabBarItem;
	IBOutlet UISegmentedControl * _segmentedControl;
	BOOL savedState[2];
	
	IBOutlet UITableView * _tableView;
	
	PROJECT_DATA _project;
	
	BOOL _isTome;
	uint nbElement;
}

@property (readonly) BOOL isTome;

- (PROJECT_DATA) activeProject;

- (void) updateProject : (uint) cacheDBID : (BOOL)isTome : (uint) element;
- (void) selectElem : (uint) projectID : (BOOL) isTome : (uint) element;

@end
