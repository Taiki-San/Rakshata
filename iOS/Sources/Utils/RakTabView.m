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

@implementation RakTabView

- (void) awakeFromNib
{
	[self configureView];
	[RakApp registerTabBarController:self.tabBarController];
}

- (void) initiateTransition
{
	UITabBarController * tabBarController = [RakApp tabBarController];
	
	if(tabBarController.selectedIndex == tabBarIndex)
		return;
	
	UIView * fromView = tabBarController.selectedViewController.view;
	UIView * toView = tabBarController.viewControllers[tabBarIndex].view;
	
	// Get the size of the view area.
	CGRect viewSize = fromView.frame;
	CGFloat windowWidth = viewSize.size.width;
	BOOL scrollRight = tabBarIndex > tabBarController.selectedIndex;
	
	// Add the to view to the tab bar view.
	[fromView.superview addSubview:toView];
	
	// Position it off screen.
	toView.frame = CGRectMake((scrollRight ? windowWidth : windowWidth), viewSize.origin.y, windowWidth, viewSize.size.height);
	
	[UIView animateWithDuration:0.3
					 animations: ^{

						 // Animate the views on and off the screen. This will appear to slide.
						 fromView.frame = CGRectMake((scrollRight ? -windowWidth : windowWidth), viewSize.origin.y, windowWidth, viewSize.size.height);
						 toView.frame = CGRectMake(0, viewSize.origin.y, windowWidth, viewSize.size.height);
					 }
	 
					 completion:^(BOOL finished) {
						 if (finished)
						 {
							 
							 // Remove the old view from the tabbar view.
							 [fromView removeFromSuperview];
							 tabBarController.selectedIndex = tabBarIndex;
						 }
					 }];
}

@end
