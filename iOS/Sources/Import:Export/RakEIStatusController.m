/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at https://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

@interface RakEIStatusController ()

@property IBOutlet UINavigationItem * header;
@property IBOutlet UILabel * archiveLabel;

@end

@implementation RakEIStatusController

- (instancetype) init
{
	self = [super init];
	
	if(self != nil)
		[self startUI];
	
	return self;
}

- (void) startUI
{
	NSArray * array = [[NSBundle mainBundle] loadNibNamed:@"Import" owner:self options:nil];
	if(array == nil || [array count] == 0)
		return;
	
	self.view = array[0];
	self.header.title = [self headerText];
	self.archiveLabel.text = [self archiveName];
	self.modalPresentationStyle = UIModalPresentationPopover;

	//Increase button size to the maximum
	UIButton * button = [self.header.leftBarButtonItem customView];
	if(button != nil)
	{
		CGRect frame = button.frame;
		UILabel * label = button.titleLabel;
		
		label.text = NSLocalizedString(@"CANCEL", nil);
		[label sizeToFit];
		
		frame.size.width = label.frame.size.width;
		button.frame = frame;
	}
	
	button = [self.header.rightBarButtonItem customView];
	if(button != nil)
	{
		CGRect frame = button.frame;
		UILabel * label = button.titleLabel;
		
		label.text = NSLocalizedString(@"IMPORT-PERFORM", nil);
		[label sizeToFit];
		
		frame.size.width = label.frame.size.width;
		button.frame = frame;
	}
	
	_fileURL = [RakApp.currentImportURL copy];
	
	UITabBarController * controller = RakApp.tabBarController;
	[controller.viewControllers[controller.selectedIndex] presentViewController:self animated:YES completion:^{}];
}

- (void) refreshUI
{
	
}

- (IBAction) closeUI
{
	UITabBarController * controller = RakApp.tabBarController;
	[controller.viewControllers[controller.selectedIndex] dismissViewControllerAnimated:YES completion:^{
		NSError * error = nil;
		[[NSFileManager defaultManager] removeItemAtURL:_fileURL error:&error];
		if(error != nil)
			NSLog(@"Couldn't delete the file to be imported :C %@", error);
	}];
}

- (void) finishing
{
	
}

- (NSString *) headerText
{
	return @"Mass Effect : Inquisition is an excessive name";
}

- (NSString *) archiveName
{
	return @"Archive name yay";
}

- (RakColor *) backgroundColor
{
	return nil;
}

@end