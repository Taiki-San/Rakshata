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
	IBOutlet UILabel * contentIDTitle, * archiveLabel;
	IBOutlet UISegmentedControl * isTomeSelector;
	IBOutlet UINavigationItem * header;
	
	BOOL didProjectNameChange;
}

@end

@implementation RakImportStatusController

- (void) switchToIssueUI : (NSArray *) dataSet
{
	manifest = [self validateMetadata:dataSet];
	if(manifest == nil || [manifest count] == 0)
	{
		[self deleteImportedFile];
		return;
	}
	
	NSArray * array = [[NSBundle mainBundle] loadNibNamed:@"Import" owner:self options:nil];
	if(array == nil || [array count] == 0)
		return;

	self.view = array[0];
	header.title = [self headerText];
	archiveLabel.text = [self archiveName];
	self.modalPresentationStyle = UIModalPresentationPopover;
	
	//Increase button size to the maximum
	UIButton * button = [header.leftBarButtonItem customView];
	if(button != nil)
	{
		CGRect frame = button.frame;
		UILabel * label = button.titleLabel;
		
		label.text = NSLocalizedString(@"CANCEL", nil);
		[label sizeToFit];
		
		frame.size.width = label.frame.size.width + 15;
		button.frame = frame;
	}
	
	button = [header.rightBarButtonItem customView];
	if(button != nil)
	{
		CGRect frame = button.frame;
		UILabel * label = button.titleLabel;
		
		label.text = NSLocalizedString(@"IMPORT-PERFORM", nil);
		[label sizeToFit];
		
		frame.size.width = label.frame.size.width + 15;
		button.frame = frame;
	}
	
	[CTID setKeyboardType:UIKeyboardTypeDecimalPad];
	CTID.delegate = (id <UITextFieldDelegate>) self;
	
	RakImportItem * item = manifest[0];
	NSString * name = getStringForWchar(item.projectData.data.project.projectName);
	if(name != nil)
		projectName.text = name;
	
	if([manifest count] == 1)
	{
		isTomeSelector.selectedSegmentIndex = item.isTome;
		
		if(item.contentID != INVALID_VALUE)
			CTID.text = getStringForCTID((int) item.contentID);
		
		if(item.isTome)
			volumeName.text = getStringForWchar(item.projectData.data.tomeRemote[0].readingName);
	}
	
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
		if(item.issue == IMPORT_PROBLEM_NONE || item.issue == IMPORT_PROBLEM_DUPLICATE)
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
	
	if(textField == projectName)
		didProjectNameChange = YES;
	
	return YES;
}

#pragma mark - Perform import

- (IBAction) updateWithMetadata
{
	charType * localProjectName = didProjectNameChange ? getStringFromUTF8((const byte *)[projectName.text UTF8String]) : NULL;
	NSMutableArray * cleanItems = [NSMutableArray array];

	for(RakImportItem * item in manifest)
	{
		if(localProjectName != NULL)
			wstrncpy(item.projectData.data.project.projectName, LENGTH_PROJECT_NAME, localProjectName);
		
		item.guessedProject = NO;
		
		if(![item checkDetailsMetadata] && [manifest count] > 1)
		{
			[item refreshState];
			if(item.issue == IMPORT_PROBLEM_NONE)
				[cleanItems addObject:item];
		}
	}
	
	if([cleanItems count] > 0)
	{
		manifest = [manifest mutableCopy];
		[(NSMutableArray *) manifest removeObjectsInArray:cleanItems];
	}
	
	if([manifest count] == 1)
	{
		RakImportItem * item = manifest[0];
		if(![item updateCTIDWith:getNumberForString(CTID.text) tomeName:volumeName.text isTome:isTomeSelector.selectedSegmentIndex != 0])
			return;

		[item refreshState];
	}
	
	syncCacheToDisk(SYNC_PROJECTS);
	[RakDBUpdate postNotificationFullUpdate];
	[self closeUI];
}

@end