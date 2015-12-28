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

#pragma mark - Register the tabs controller

- (void) registerTabBarController : (UITabBarController *) _tabBarController
{
	if(tabBarController == nil)
	{
		tabBarController = _tabBarController;
		tabBarController.delegate = self;
	}
}

- (void) registerSeries : (Series *) series
{
	if(tabSerie == nil)
		tabSerie = series;
}

- (Series *) _serie
{
	if(tabSerie == nil)
	{
		UIStoryboard * storyboard = [UIStoryboard storyboardWithName:@"Main" bundle:nil];
		if(storyboard != nil)
			[storyboard instantiateViewControllerWithIdentifier:@"SR"];
	}
	
	return [super _serie];
}

- (void) registerCT : (CTSelec *) CT
{
	if(tabCT == nil)
		tabCT = CT;
}

- (CTSelec *) _CT
{
	if(tabCT == nil)
	{
		UIStoryboard * storyboard = [UIStoryboard storyboardWithName:@"Main" bundle:nil];
		if(storyboard != nil)
			[storyboard instantiateViewControllerWithIdentifier:@"CT"];
	}
	
	return [super _CT];
}

- (void) registerMDL : (MDL *) MDL
{
	if(tabMDL == nil)
		tabMDL = MDL;
}

- (MDL *) _MDL
{
	if(tabMDL == nil)
	{
		UIStoryboard * storyboard = [UIStoryboard storyboardWithName:@"Main" bundle:nil];
		if(storyboard != nil)
			[storyboard instantiateViewControllerWithIdentifier:@"MDL"];
	}

	return [super _MDL];
}

- (void) registerReader : (Reader *) reader
{
	if(tabReader == nil)
		tabReader = reader;
}

- (Reader *) _reader
{
	if(tabReader == nil)
	{
		UIStoryboard * storyboard = [UIStoryboard storyboardWithName:@"Main" bundle:nil];
		if(storyboard != nil)
			[storyboard instantiateViewControllerWithIdentifier:@"RD"];
	}

	return [super _reader];
}

#pragma mark - Tab bar controller

- (UITabBarController *) tabBarController
{
	return tabBarController;
}

- (BOOL)tabBarController:(UITabBarController *)tabBarController shouldSelectViewController:(UIViewController *)viewController
{
	//Warn the RakTabView that it's about to be selected
#if 0
	if([viewController isKindOfClass:[UINavigationController class]])
		viewController = [((UINavigationController *) viewController).viewControllers firstObject];
#endif
	
	if([viewController isKindOfClass:[RakTabView class]])
	{
		[(RakTabView *) viewController viewWillFocus];
		return !((RakTabView *) viewController).initWithNoContent;
	}
	
	return YES;
}

@end
