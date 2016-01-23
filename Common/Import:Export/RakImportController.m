/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

@implementation RakImportController

+ (void) importFile : (NSArray *) IOController
{
	if(IOController == nil || [IOController count] == 0)
		return;

#if !TARGET_OS_IPHONE
	if([NSThread isMainThread])
	{
		RakImportStatusController * UI = [[RakImportStatusController alloc] init];

		dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
			[self importFile:IOController withUI:UI];
		});
	}
	else
	{
		__block RakImportStatusController * UI;

		dispatch_sync(dispatch_get_main_queue(), ^{
			UI = [[RakImportStatusController alloc] init];
		});

		[self importFile:IOController withUI:UI];
	}
#endif
}

+ (void) importFile : (NSArray <RakImportBaseController <RakImportIO> * > *) IOController withUI : (RakImportStatusController *) UI
{
	//We first try to open the file, so we can eventually start working with it
	NSArray * manifest = getManifestForIOs(IOController);
	if(manifest == nil || [manifest count] == 0)
	{
		dispatch_async(dispatch_get_main_queue(), ^{		[UI closeUI];		});
		return;
	}

	//We finish setting up the UI
	BOOL firstPass = YES;
	UI.nbElementToExport = [manifest count];

	//Okay, we iterate through
	BOOL haveFoundProblems = NO;
	for(RakImportItem * item in manifest)
	{
		if(firstPass)
			firstPass = NO;
		else
			UI.posInExport++;

		//First, check the file exist in the archive
		//An empty path mean the lack of structure in the zip
		if(![item.path isEqualToString:@""] && ![item.IOController canLocateFile: item.path])
			continue;

		//At this point, we know two things: the project is valid, exist in the archive
		if(item.issue == IMPORT_PROBLEM_METADATA || [item needMoreData])	//We need to ask extra details to the user
		{
			haveFoundProblems = YES;
			item.issue = IMPORT_PROBLEM_METADATA;
			continue;
		}

		else if(item.issue == IMPORT_PROBLEM_METADATA_DETAILS || item.contentID == INVALID_VALUE)
		{
			haveFoundProblems = YES;
			item.issue = IMPORT_PROBLEM_METADATA_DETAILS;
			continue;
		}

		else if([item isReadable])
		{
			haveFoundProblems = YES;
			item.issue = IMPORT_PROBLEM_DUPLICATE;
			continue;
		}

		//Well, I guess we can carry on
		if(![item installWithUI:UI])
		{
			haveFoundProblems = YES;
			item.issue = IMPORT_PROBLEM_INSTALL_ERROR;
			continue;
		}

		[item processThumbs];
		[item registerProject];

		if([UI haveCanceled])
		{
			//Urk, we have to delete everything...
			for(RakImportItem * itemToDelete in manifest)
			{
				if(UI.posInExport)
					UI.posInExport--;

				[itemToDelete deleteData];

				if(itemToDelete == item)
					break;
			}
		}
	}

	if(![UI haveCanceled])
	{
		dispatch_sync(dispatch_get_main_queue(), ^{
			if(haveFoundProblems)
			{
				UI.posInExport = UI.nbElementToExport;
				[UI switchToIssueUI:manifest];
			}
			else
			{
				[UI finishing];
				[UI closeUI];
			}
		});
	}
	else
		[self postProcessingUI:UI];
}

+ (void) postProcessingUI: (RakImportStatusController *) UI
{
	syncCacheToDisk(SYNC_PROJECTS);
	notifyFullUpdate();

	if([NSThread isMainThread])
		[UI closeUI];
	else
		dispatch_sync(dispatch_get_main_queue(), ^{	[UI closeUI];	});
}

@end
