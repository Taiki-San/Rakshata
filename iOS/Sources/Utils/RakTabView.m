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
	CGRect viewSize = fromView.frame, newView = toView.frame, workingFrame = newView;
	BOOL scrollRight = tabBarIndex > tabBarController.selectedIndex;
	const CGFloat windowWidth = scrollRight ? viewSize.size.width : -viewSize.size.width;
	
	// Add the to view to the tab bar view.
	[fromView.superview addSubview:toView];
	
	if(newView.origin.y == 0)
		newView.origin.y = workingFrame.origin.y = 20;
	
	// Position it off screen.
	workingFrame.origin.x += windowWidth;
	toView.frame = workingFrame;
	
	[UIView animateWithDuration:0.3
					 animations: ^{
						 // Animate the views on and off the screen. This will appear to slide.
						 fromView.frame = CGRectMake(-windowWidth, viewSize.origin.y, windowWidth, viewSize.size.height);
						 toView.frame = newView;
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
