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
		tabBarController = _tabBarController;
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
			tabSerie = [storyboard instantiateViewControllerWithIdentifier:@"SR"];
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
			tabCT = [storyboard instantiateViewControllerWithIdentifier:@"CT"];
	}
	
	return [super _CT];
}

- (MDL *) _MDL
{
	if(tabMDL == nil)
	{
		UIStoryboard * storyboard = [UIStoryboard storyboardWithName:@"Main" bundle:nil];
		if(storyboard != nil)
			tabMDL = [storyboard instantiateViewControllerWithIdentifier:@"MDL"];
	}

	return nil;//[super _MDL];
}

- (Reader *) _reader
{
	if(tabReader == nil)
	{
		UIStoryboard * storyboard = [UIStoryboard storyboardWithName:@"Main" bundle:nil];
		if(storyboard != nil)
			tabReader = [storyboard instantiateViewControllerWithIdentifier:@"RD"];
	}

	return [super _reader];
}

#pragma mark - Access data

- (UITabBarController *) tabBarController
{
	return tabBarController;
}

@end