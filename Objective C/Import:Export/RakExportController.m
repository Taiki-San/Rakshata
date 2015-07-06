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

#include "JSONParser.h"

#define EXPORT_GENERIC_NAME "Archive"

NSDictionary * linearizeContentLine(PROJECT_DATA project, uint projectID, BOOL isTome, int selection, uint * index);

@implementation RakExportController

+ (NSString *) craftArchiveNameFromPasteboard : (NSPasteboard *) pasteboard
{
	if(pasteboard == nil)
		return nil;

	RakDragItem * item = [[RakDragItem alloc] initWithData: [pasteboard dataForType:PROJECT_PASTEBOARD_TYPE]];
	if(item == nil)
		return nil;

	return [@"/" stringByAppendingString:[self craftArchiveName:item.project isTome:item.isTome selection:item.selection]];
}

+ (NSString *) craftArchiveName : (PROJECT_DATA) project isTome : (BOOL) isTome selection : (int) selection
{
	NSString * projectPath = [getStringForWchar(project.projectName) stringByTrimmingCharactersInSet:[NSCharacterSet characterSetWithCharactersInString:@"/:"]], * outputPath = nil;

	if(projectPath != nil && [projectPath length] > 0)
	{
		if(selection != INVALID_SIGNED_VALUE)
		{
			if(isTome)
			{
				uint position = getPosForID(project, false, selection);
				if(position == INVALID_VALUE)
					return nil;

				META_TOME volumeData = project.tomesFull[position];

				if(volumeData.readingName[0] != 0)
					outputPath = [NSString stringWithFormat:@"%@ - %@ ", projectPath, getStringForWchar(volumeData.readingName)];
				else
					outputPath = [NSString stringWithFormat:NSLocalizedString(@"VOLUME-%d", nil), volumeData.readingID];
			}

			else
				outputPath = [NSString stringWithFormat:@"%@ - %@ ", projectPath, [NSString stringWithFormat:NSLocalizedString(@"CHAPTER-%d", nil), selection]];
		}
		else
			outputPath = projectPath;
	}
	else
		outputPath = @EXPORT_GENERIC_NAME;

	return [outputPath stringByAppendingString:@"."ARCHIVE_FILE_EXT];
}

#pragma mark - Compression Job

+ (void) createArchiveFromPasteboard : (NSPasteboard *) pasteboard toPath : (NSString *) path
{
	//Ensure we get a directory to write in
	if(path == nil)
	{
		path = [self craftArchiveNameFromPasteboard:pasteboard];

		if(path == nil)
			return;
	}

	//We don't want to perform the compression job in the main thread, as it may take quite some time
	if([NSThread isMainThread])
	{
		dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
			[self createArchiveFromPasteboard:pasteboard toPath:path];
		});
		return;
	}

	//Okay, open the zipfile, then start analysing the pasteboard
	zipFile * file = zipOpen([path UTF8String], APPEND_STATUS_CREATE);
	if(file == NULL)
	{
		NSLog(@"Couldn't create the file D:");
		return;
	}

	//UI side, we only support moving around one project, so we're going to assume that and make our lives easier
	RakDragItem * item = [[RakDragItem alloc] initWithData: [pasteboard dataForType:PROJECT_PASTEBOARD_TYPE]];
	if(item == nil)
		return;
}

#pragma mark - Creation of the manifest

+ (NSData *) generateManifestForItem : (RakDragItem *) item contentDetail : (NSArray **) content thumbFiles : (NSArray **) imagesPath
{
	NSDictionary * project = [self lineariseProject:item.project ofID:0 withImages:imagesPath];
	if(project == nil)
		return nil;

	uint index = 0;
	*content = [self lineariseContent:item.project ofProjectID:0 fullProject:item.fullProject isTome:item.isTome selection:item.selection index:&index];
	if(*content == nil)
		return nil;

	//Crafting the final file
	NSDictionary * manifest = @{RAK_STRING_METADATA : @{RAK_STRING_METADATA_PROJECT: project},
								RAK_STRING_CONTENT	: *content};
	NSError * serializationError;
	NSData * serializedFile = [NSJSONSerialization dataWithJSONObject:manifest options:0 error:&serializationError];

	if(serializedFile == nil)
		NSLog(@"%@", serializationError);

	return serializedFile;
}

+ (NSDictionary *) lineariseProject : (PROJECT_DATA) project ofID : (uint) allocatedID  withImages : (NSArray **) imagesPath
{
	//Build the main metadata section
	NSMutableDictionary * dict = [NSMutableDictionary new];

	[dict insertValue:@(allocatedID) inPropertyWithKey:RAK_STRING_METADATA_PROJECTID];
	[dict insertValue:getStringForWchar(project.projectName) inPropertyWithKey:RAK_STRING_METADATA_PROJECTNAME];
	[dict insertValue:@(project.repo->type) inPropertyWithKey:RAK_STRING_METADATA_REPOTYPE];
	[dict insertValue:[NSString stringWithUTF8String:project.repo->URL] inPropertyWithKey:RAK_STRING_METADATA_REPOURL];
	[dict insertValue:@(project.projectID) inPropertyWithKey:RAK_STRING_METADATA_REPOURL];

	if(project.description[0])
		[dict insertValue:getStringForWchar(project.description) inPropertyWithKey:RAK_STRING_METADATA_DESCRIPTION];

	[dict insertValue:getStringForWchar(project.authorName) inPropertyWithKey:RAK_STRING_METADATA_AUTHOR];

	if(project.status != STATUS_INVALID)
		[dict insertValue:@(project.status) inPropertyWithKey:RAK_STRING_METADATA_STATUS];

	[dict insertValue:@(project.tagMask) inPropertyWithKey:RAK_STRING_METADATA_TAGMASK];
	[dict insertValue:@(project.rightToLeft) inPropertyWithKey:RAK_STRING_METADATA_RIGHT2LEFT];

	//Okay, now, we want to inser the thumbnails
	//We ask an API to craft for us the expected filenames

	PROJECT_DATA_EXTRA extraProject = getEmptyExtraProject();
	extraProject.data.project = project;
	memset(extraProject.haveImages, 1, sizeof(extraProject.haveImages));
	ICONS_UPDATE * filesPath = generateIconUpdateWorkload(&extraProject, 1);

	//We iterate to see which files exist
	uint count = 0;

	const byte imageID[4] = {THUMBID_SRGRID, THUMBID_HEAD, THUMBID_CT, THUMBID_DD};
	NSMutableArray * imagePathToCopy = [NSMutableArray new];

	while(filesPath != NULL)
	{
		if(filesPath->filename != NULL && checkFileExist(filesPath->filename))
		{
			NSString * outputPath = [NSString stringWithFormat:@"thumbs/%d-%d", allocatedID, count++];

			for(byte pos = 0; pos < 4; ++pos)
			{
				if(filesPath->updateType != imageID[pos])
					continue;

				NSString * sectionName = nil;

				if(pos == 0)
					sectionName = filesPath->isRetina ? RAK_STRING_METADATA_IMG_GRID_2X : RAK_STRING_METADATA_IMG_GRID;
				else if(pos == 1)
					sectionName = filesPath->isRetina ? RAK_STRING_METADATA_IMG_CT_2X : RAK_STRING_METADATA_IMG_CT;
				else if(pos == 3)
					sectionName = filesPath->isRetina ? RAK_STRING_METADATA_IMG_DD_2X : RAK_STRING_METADATA_IMG_DD;

				if(sectionName != nil)
				{
					[dict insertValue:outputPath inPropertyWithKey:sectionName];
					[imagePathToCopy addObject:@{@"origin" : [NSString stringWithUTF8String:filesPath->filename],
												 @"output" : outputPath}];
				}

				break;
			}
		}

		void * tmp = filesPath;
		free(filesPath->filename);
		filesPath = filesPath->next;
		free(tmp);
	}

	*imagesPath = [NSArray arrayWithArray:imagePathToCopy];
	return [NSDictionary dictionaryWithDictionary:dict];
}

+ (NSArray *) lineariseContent : (PROJECT_DATA) project ofProjectID : (uint) projectID fullProject : (BOOL) fullProject isTome : (BOOL) isTome selection : (int) selection index : (uint *) index;
{
	//Ensure we have valid data
	void * freeTable[4] = {NULL};
	if((fullProject || isTome) && project.tomesInstalled == NULL)
	{
		if(project.tomesFull == NULL)
		{
			getUpdatedTomeList(&project, true);
			freeTable[0] = project.tomesFull;
			freeTable[1] = project.tomesInstalled;
		}
		else
		{
			checkTomeValable(&project, NULL);
			freeTable[1] = project.tomesInstalled;
		}
	}

	if((fullProject || !isTome) && project.chapitresInstalled == NULL)
	{
		if(project.chapitresFull == NULL)
		{
			getUpdatedChapterList(&project, true);
			freeTable[2] = project.chapitresFull;
			freeTable[3] = project.chapitresInstalled;
		}
		else
		{
			checkChapitreValable(&project, NULL);
			freeTable[3] = project.chapitresInstalled;
		}
	}

	NSArray * output = nil;

	//To insert a full project, we first inject the
	if(fullProject)
	{
		selection = INVALID_SIGNED_VALUE;
		isTome = false;
	}

	//Okay, at this point, we craft the array
	if(selection == INVALID_SIGNED_VALUE)
	{
		uint length = ACCESS_DATA(isTome, project.nombreChapitreInstalled, project.nombreTomesInstalled);
		NSMutableArray * collector = [NSMutableArray new];

		for(uint i = 0; i < length; ++i)
		{
			NSDictionary * dict = linearizeContentLine(project, projectID, isTome, ACCESS_DATA(isTome, project.chapitresInstalled[i], project.tomesInstalled[i].ID), index);
			if(dict != nil)
				[collector addObject:dict];
		}

		if(fullProject)
		{
			length = project.nombreTomesInstalled;
			for(uint i = 0; i < length; ++i)
			{
				NSDictionary * dict = linearizeContentLine(project, projectID, true, project.tomesInstalled[i].ID, index);
				if(dict != nil)
					[collector addObject:dict];
			}
		}

		if([collector count] > 0)
			output = [NSArray arrayWithArray:collector];
	}
	else
	{
		NSDictionary * dict = linearizeContentLine(project, projectID, isTome, selection, index);
		if(dict != nil)
			output = @[dict];
	}

	//Free the allocated pointers (if any)
	freeTomeList(freeTable[0], project.nombreTomes, true);
	freeTomeList(freeTable[1], project.nombreTomesInstalled, true);
	free(freeTable[2]);
	free(freeTable[3]);

	return output;
}

+ (void) addEntryToZip : (PROJECT_DATA) project item : (uint) position isTome : (BOOL) isTome toArchive : (zipFile *) file
{

}

#pragma mark - Utils

+ (BOOL) onlyHaveOneProject : (PROJECT_DATA *) project nbEntries : (uint) nbEntries
{
	for(uint i = 1; i < nbEntries; i++)
	{
		if(project[i - 1].cacheDBID != project[i].cacheDBID)
			return NO;
	}

	return YES;
}

@end

NSDictionary * linearizeContentLine(PROJECT_DATA project, uint projectID, BOOL isTome, int selection, uint * index)
{
	NSMutableDictionary * dict = [NSMutableDictionary new];

	[dict insertValue:@(projectID) inPropertyWithKey:RAK_STRING_CONTENT_PROJECT];
	[dict insertValue:[NSString stringWithFormat:@"%d", (*index)++] inPropertyWithKey:RAK_STRING_CONTENT_DIRECTORY];
	[dict insertValue:@(isTome) inPropertyWithKey:RAK_STRING_CONTENT_ISTOME];
	[dict insertValue:@(selection) inPropertyWithKey:RAK_STRING_CONTENT_ID];

	if(isTome)
	{
		uint pos = getPosForID(project, true, selection);
		if(pos == INVALID_VALUE)
			return nil;
		else
		{
			NSArray * volumeMetadata = recoverVolumeBloc(&(project.tomesInstalled[pos]), 1, project.isPaid);
			if(volumeMetadata != nil && [volumeMetadata count] > 0)
				[dict insertValue:[volumeMetadata objectAtIndex:0] inPropertyWithKey:RAK_STRING_CONTENT_VOL_DETAILS];
		}
	}

	return [NSDictionary dictionaryWithDictionary:dict];
}