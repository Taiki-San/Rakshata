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

@interface RakImportStatusController()
{
	NSArray <RakImportItem *> * manifest;
	IBOutlet UITextField * projectName, * CTID, * volumeName;
	IBOutlet UILabel * contentIDTitle;
}

@end

@implementation RakImportStatusController

- (void) switchToIssueUI : (NSArray *) dataSet
{
	manifest = [self validateMetadata:dataSet];
	if(manifest == nil)
		return;
	
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
	
	[CTID setKeyboardType:UIKeyboardTypeDecimalPad];
	CTID.delegate = (id <UITextFieldDelegate>) self;
	
	UITabBarController * controller = RakApp.tabBarController;
	[controller.viewControllers[controller.selectedIndex] presentViewController:self animated:YES completion:^{}];
}

- (void) viewWillAppear:(BOOL)animated
{
	[self updateCTIDWidth:NO];
}

- (NSArray *) validateMetadata : (NSArray *) dataset
{
	if(dataset == nil || [dataset count] == 0)
		return nil;
	
	bool isLocal;
	uint projectID = INVALID_VALUE;
	NSMutableArray * nonProcessedCollector = [NSMutableArray array];
	for(RakImportItem * item in dataset)
	{
		if(item.issue == IMPORT_PROBLEM_NONE)
			continue;
		
		PROJECT_DATA project = item.projectData.data.project;
		
		//First project we hit
		if(projectID == INVALID_VALUE)
		{
			projectID = project.cacheDBID;
			if(projectID == INVALID_VALUE)	//local project
			{
				isLocal = true;
				projectID = project.projectID;
			}
			else
				isLocal = false;
			
			[nonProcessedCollector addObject:item];
		}
		
		//Same project we already hit
		else if((isLocal && project.projectID == projectID) || (!isLocal && project.cacheDBID == projectID))
		{
			[nonProcessedCollector addObject:item];
		}
		
		//We only support import of a single project at a time for now
		else
		{
#ifdef EXTENSIVE_LOGGING
			NSLog(@"Dropped %@", item.path);
#endif
		}
	}
	
	return [nonProcessedCollector count] == 0 ? nil : nonProcessedCollector;
}

- (NSString *) headerText
{
	return getStringForWchar([manifest firstObject].projectData.data.project.projectName);
}

- (NSString *) archiveName
{
	return [NSString localizedStringWithFormat:NSLocalizedString(@"IMPORT-OF-%@", nil), [self.fileURL lastPathComponent]];
}

- (RakColor *) backgroundColor
{
	return nil;
}

- (NSData *) queryThumbOf : (RakImportItem *) item withIndex : (uint) index
{
	return nil;
}

- (BOOL) reflectMetadataUpdate : (PROJECT_DATA) project withImages : (NSArray *) overridenImages forItem : (RakImportItem *) item
{
	return YES;
}

- (void) postProcessUpdate
{
	
}

- (void) close
{
	
}

#pragma mark - Text fields management

- (IBAction) buttonChanging : (UISegmentedControl *) sender
{
	[self updateCTIDWidth:sender.selectedSegmentIndex != 0];
	[UIView animateWithDuration:0.2
					 animations:^{
						 [self.view layoutIfNeeded]; // Called on parent view
					 }];
}

- (void) updateCTIDWidth : (BOOL) isTome
{
	contentIDTitle.text = [NSString stringWithFormat:@"%@ #", NSLocalizedString(isTome ? @"VOLUME" : @"CHAPTER", nil)];

	const CGFloat width = isTome ? 60 : RakApp.window.frame.size.width - (CTID.frame.origin.x + 15);
	
	for(NSLayoutConstraint * constraint in CTID.constraints)
	{
		if([constraint class] != [NSLayoutConstraint class])
			continue;
		
		constraint.constant = width;
	}
}

//Only really usefull on iPad
- (BOOL)textField:(UITextField *)textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString *)string
{
	if (string.length && textField == CTID)
		return [RakCTFormatter isStringValid:[textField.text stringByReplacingCharactersInRange:range withString:string]];
	
	return YES;
}
@end