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

@implementation RakAppiOSDelegate

- (BOOL) application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
	[self awakeFromNib];
	
	UIStoryboard * storyboard = [UIStoryboard storyboardWithName:@"Main" bundle:nil];
	navigationController = [storyboard instantiateViewControllerWithIdentifier:@"MainNavigationController"];
	UIViewController *viewController = navigationController.topViewController;
	
	viewController.navigationItem.leftBarButtonItem = viewController.editButtonItem;
	
	return YES;
}

- (BOOL) hasFocus
{
	return RakRealApp.applicationState != UIApplicationStateBackground;
}

- (void) applicationWillTerminate:(UIApplication *)application
{
	[self flushState];
}

- (BOOL)splitViewController:(UISplitViewController *)splitViewController collapseSecondaryViewController:(UIViewController *)secondaryViewController ontoPrimaryViewController:(UIViewController *)primaryViewController {
	return YES;
}


@end
