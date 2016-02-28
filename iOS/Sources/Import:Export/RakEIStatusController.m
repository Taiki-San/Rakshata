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

@implementation RakEIStatusController

- (instancetype) init
{
	self = [super init];
	
	if(self != nil)
		_fileURL = [RakApp.currentImportURL copy];
	
	return self;
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
	return @"";
}

- (RakColor *) backgroundColor
{
	return nil;
}

@end