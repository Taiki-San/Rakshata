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

@implementation RakImportController

+ (void) importFile : (id <RakImportIO>) IOController
{
	if(IOController == nil)
		return;

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
}

+ (void) importFile : (id <RakImportIO>) IOController withUI : (RakImportStatusController *) UI
{
	//We first try to open the file, so we can eventually start working with it
	NSArray * manifest = [IOController getManifest];
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
		if(![IOController canLocateFile: item.path])
			continue;

		//At this point, we know two things: the project is valid, exist in the archive
		if([item needMoreData])	//We need to ask extra details to the user
		{
			haveFoundProblems = YES;
			item.issue = IMPORT_PROBLEM_METADATA;
			continue;
		}

		if([item isReadable])
		{
			haveFoundProblems = YES;
			item.issue = IMPORT_PROBLEM_DUPLICATE;
			continue;
		}

		//Well, I guess we can carry on
		if(![item install:IOController withUI:UI])
		{
			haveFoundProblems = YES;
			item.issue = IMPORT_PROBLEM_INSTALL_ERROR;
			continue;
		}

		[item processThumbs:IOController];
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
				[UI addIOController:IOController];
				[UI switchToIssueUI:manifest];
			}
			else
				[UI finishing];
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
