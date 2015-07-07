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

	//UI side, we only support moving around one project, so we're going to assume that and make our lives easier
	RakDragItem * item = [[RakDragItem alloc] initWithData: [pasteboard dataForType:PROJECT_PASTEBOARD_TYPE]];
	if(item == nil)
		return;

	NSMutableArray * content, * thumbnails;
	NSDictionary * manifest = [self generateManifestForItem:item contentDetail:&content thumbFiles:&thumbnails];
	if(manifest == nil)
		return;

	//Okay, open the zipfile
	zipFile * file = zipOpen([path UTF8String], APPEND_STATUS_CREATE);
	if(file == NULL)
	{
		NSLog(@"Couldn't create the file D:");
		return;
	}

	NSArray * invalidContent = [self insertContent:content ofItem:item intoZipfile:file];

	//Fuck, invalid content, we need to update the manifest :X
	if([invalidContent count])
	{
		for(NSDictionary * entry in invalidContent)
			[content removeObject:entry];

		if([content count])
			manifest = @{	RAK_STRING_METADATA : [manifest objectForKey:RAK_STRING_METADATA],
							RAK_STRING_CONTENT	: content};
		else
			manifest = nil;
	}

	if(manifest == nil)
	{
		NSLog(@"No valid selection :/");
		closeZip(file);
		remove([path UTF8String]);
	}

	//Good, now, we can serialize the manifest, add it to the zipfile and call it a day
	NSError * serializationError;
	NSData * serializedFile = [NSJSONSerialization dataWithJSONObject:manifest options:0 error:&serializationError];

	if(serializedFile == nil)
	{
		NSLog(@"%@", serializationError);
		closeZip(file);
		remove([path UTF8String]);
	}
	else
	{
		if(!addMemToZip(file, METADATA_FILE, [serializedFile bytes], [serializedFile length]))
		{
			NSLog(@"Couln't output the manifest :/");
			closeZip(file);
			remove([path UTF8String]);
		}
		else
		{
			zipClose(file, "Generated using "PROJECT_NAME" v"PRINTABLE_VERSION);
		}
	}
}

#pragma mark - Creation of the manifest

+ (NSDictionary *) generateManifestForItem : (RakDragItem *) item contentDetail : (NSMutableArray **) content thumbFiles : (NSArray **) imagesPath
{
	NSDictionary * project = [self lineariseProject:item.project ofID:0 withImages:imagesPath];
	if(project == nil)
		return nil;

	uint index = 0;
	PROJECT_DATA projectData = item.project;

	*content = [self lineariseContent:&projectData ofProjectID:0 fullProject:item.fullProject isTome:item.isTome selection:item.selection index:&index];
	if(*content == nil)
		return nil;

	item.project = projectData;

	//Crafting the final file
	return @{	RAK_STRING_METADATA : @{RAK_STRING_METADATA_PROJECT: project},
				RAK_STRING_CONTENT	: *content};
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

+ (NSMutableArray *) lineariseContent : (PROJECT_DATA *) project ofProjectID : (uint) projectID fullProject : (BOOL) fullProject isTome : (BOOL) isTome selection : (int) selection index : (uint *) index;
{
	//Ensure we have valid data
	if((fullProject || isTome) && project->tomesInstalled == NULL)
	{
		if(project->tomesFull == NULL)
			getUpdatedTomeList(project, true);
		else
			checkTomeValable(project, NULL);
	}

	if((fullProject || !isTome) && project->chapitresInstalled == NULL)
	{
		if(project->chapitresFull == NULL)
			getUpdatedChapterList(project, true);
		else
			checkChapitreValable(project, NULL);
	}

	NSMutableArray * output = nil;

	//To insert a full project, we first inject the
	if(fullProject)
	{
		selection = INVALID_SIGNED_VALUE;
		isTome = false;
	}

	//Okay, at this point, we craft the array
	if(selection == INVALID_SIGNED_VALUE)
	{
		uint length = ACCESS_DATA(isTome, project->nombreChapitreInstalled, project->nombreTomesInstalled);
		NSMutableArray * collector = [NSMutableArray new];

		for(uint i = 0; i < length; ++i)
		{
			NSDictionary * dict = linearizeContentLine(*project, projectID, isTome, ACCESS_DATA(isTome, project->chapitresInstalled[i], project->tomesInstalled[i].ID), index);
			if(dict != nil)
				[collector addObject:dict];
		}

		if(fullProject)
		{
			length = project->nombreTomesInstalled;
			for(uint i = 0; i < length; ++i)
			{
				NSDictionary * dict = linearizeContentLine(*project, projectID, true, project->tomesInstalled[i].ID, index);
				if(dict != nil)
					[collector addObject:dict];
			}
		}

		if([collector count] > 0)
			output = collector;
	}
	else
	{
		NSDictionary * dict = linearizeContentLine(*project, projectID, isTome, selection, index);
		if(dict != nil)
			output = [NSMutableArray arrayWithObject:dict];
	}

	return output;
}

+ (void) addEntryToZip : (PROJECT_DATA) project item : (uint) position isTome : (BOOL) isTome toArchive : (zipFile *) file
{

}

#pragma mark - Insertion into zip

+ (NSArray *) insertContent : (NSMutableArray *) content ofItem : (RakDragItem *) item intoZipfile : (zipFile *) file
{
	NSMutableArray * invalidContent = [NSMutableArray new];

	for(NSDictionary * entry in content)
	{
		//Load the data so we get a list of files
		DATA_LECTURE entryData;
		BOOL isTome = [objectForKey(entry, RAK_STRING_CONTENT_ISTOME, nil, [NSNumber class]) boolValue];

		if(!configFileLoader(item.project, isTome, [objectForKey(entry, RAK_STRING_CONTENT_ID, nil, [NSNumber class]) intValue], &entryData))
		{
			[invalidContent addObject:entry];
			continue;
		}

		//We check all files exists
		BOOL problemDetected = NO;
		for(uint pos = 0; pos < entryData.nombrePage; pos++)
		{
			if(!checkFileExist(entryData.nomPages[pos]))
			{
				problemDetected = YES;
				break;
			}
		}

		if(problemDetected || !entryData.nombrePage)
		{
			releaseDataReader(&entryData);
			[invalidContent addObject:entry];
			continue;
		}

		//Okay, all files exists, let's add them to the archive

		NSString * _basePath = [objectForKey(entry, RAK_STRING_CONTENT_DIRECTORY, nil, [NSString class]) stringByAppendingString:@"/"];
		const char * basePath = [_basePath UTF8String];
		const uint baseZipPathLength = [_basePath length];

		//First, create the directory
		createDirInZip(file, basePath);

		BOOL error = NO;
		char * outFile;
		for(uint pos = 0, basePagePathLength = strlen(entryData.path[0]); pos < entryData.nombrePage && !error; pos++)
		{
			if(isTome)
			{
				//We must detect shared chapters
#warning "to do"
				outFile = NULL;
			}
			else
			{
				char inzipOfConfig[baseZipPathLength + 50];
				snprintf(inzipOfConfig, sizeof(inzipOfConfig), "%s%s", basePath, &(entryData.nomPages[pos][basePagePathLength]));
				outFile = strdup(inzipOfConfig);
			}

			//Add the file to the zip, then cleanup
			if(!addFileToZip(file, entryData.nomPages[pos], outFile))
				error = YES;

			free(outFile);

			//If we need to insert the config.dat file
			if(pos + 1 == entryData.nombrePage || entryData.pathNumber[pos] != entryData.pathNumber[pos + 1])
			{
				char pathToConfig[basePagePathLength + 50], inzipOfConfig[baseZipPathLength + 50];
				snprintf(pathToConfig, sizeof(pathToConfig), "%s/"CONFIGFILE, entryData.path[entryData.pathNumber[pos]]);
				snprintf(inzipOfConfig, sizeof(inzipOfConfig), "%s/"CONFIGFILE, basePath);

				if(!addFileToZip(file, pathToConfig, inzipOfConfig))
					error = YES;

				basePagePathLength = strlen(entryData.path[entryData.pathNumber[pos + 1]]);
			}
		}

		if(error)
		{
			problemDetected = YES;
			break;
		}
	}

	return [NSArray arrayWithArray:invalidContent];
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