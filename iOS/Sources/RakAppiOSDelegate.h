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

@interface RakAppiOSDelegate : RakAppDelegate <UITabBarControllerDelegate>
{
	UITabBarController * tabBarController;
}

@property (readonly) BOOL hasFocus;
@property (strong, nonatomic) UIWindow *window;

- (void) registerTabBarController : (UITabBarController *) _tabBarController;

- (void) registerSeries : (Series *) series;
- (void) registerCT : (CTSelec *) CT;
- (void) registerMDL : (MDL *) MDL;
- (void) registerReader : (Reader *) reader;

- (UITabBarController *) tabBarController;

@end
