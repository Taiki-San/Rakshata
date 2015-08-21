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
#define BASE_THUMBS_PATH "thumbs/"

NSDictionary * linearizeContentLine(PROJECT_DATA project, uint projectID, BOOL isTome, void * selection, uint * index);

@implementation RakExportController

+ (NSString *) craftArchiveNameFromPasteboard : (NSPasteboard *) pasteboard
{
	if(pasteboard == nil)
		return nil;

	RakDragItem * item = [[RakDragItem alloc] initWithData: [pasteboard dataForType:PROJECT_PASTEBOARD_TYPE]];
	if(item == nil)
		return nil;

	return [self craftArchiveName:item.project isTome:item.isTome selection:item.selection];
}

+ (NSString *) craftArchiveName : (PROJECT_DATA) project isTome : (BOOL) isTome selection : (uint) selection
{
	NSString * projectPath = [getStringForWchar(project.projectName) stringByTrimmingCharactersInSet:[NSCharacterSet characterSetWithCharactersInString:@"/:"]], * outputPath = nil;

	if(projectPath != nil && [projectPath length] > 0)
	{
		if(selection != INVALID_VALUE)
		{
			if(isTome)
			{
				uint position = getPosForID(project, false, selection);
				if(position == INVALID_VALUE)
					return nil;

				outputPath = getStringForVolumeFull(project.tomesFull[position]);
			}

			else
				outputPath = [NSString stringWithFormat:@"%@ - %@", projectPath, getStringForChapter(selection)];
		}
		else
			outputPath = projectPath;
	}
	else
		outputPath = @EXPORT_GENERIC_NAME;

	return [outputPath stringByAppendingString:@"."ARCHIVE_FILE_EXT];
}

#pragma mark - Compression Job

+ (void) createArchiveFromPasteboard : (NSPasteboard *) pasteboard toPath : (NSString *) path withURL : (NSURL *) url
{
	//We don't want to perform the compression job in the main thread, as it may take quite some time
	if([NSThread isMainThread])
	{
		dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
			[self createArchiveFromPasteboard:pasteboard toPath:path withURL:url];
		});
		return;
	}

	__block RakExportStatusController * UI = nil;
	dispatch_sync(dispatch_get_main_queue(), ^{
		UI = [[RakExportStatusController alloc] init];
	});

	//Ensure we get a directory to write in
	if(path == nil)
	{
		path = [self craftArchiveNameFromPasteboard:pasteboard];

		if(path == nil)
		{
			dispatch_async(dispatch_get_main_queue(), ^{		[UI closeUI];		});
			return;
		}

		path = [NSString stringWithFormat:@"%@/%@", [url path], path];
	}

	//UI side, we only support moving around one project, so we're going to assume that and make our lives easier
	RakDragItem * item = [[RakDragItem alloc] initWithData: [pasteboard dataForType:PROJECT_PASTEBOARD_TYPE]];
	if(item == nil)
	{
		dispatch_async(dispatch_get_main_queue(), ^{		[UI closeUI];		});
		return;
	}

	NSMutableArray * content, * thumbnails;
	NSDictionary * manifest = [self generateManifestForItem:item contentDetail:&content thumbFiles:&thumbnails];
	if(manifest == nil)
	{
		dispatch_async(dispatch_get_main_queue(), ^{		[UI closeUI];		});
		return;
	}

	//Okay, open the zipfile
	zipFile * file = zipOpen([path UTF8String], APPEND_STATUS_CREATE);
	if(file == NULL)
	{
		NSLog(@"Couldn't create the file D:");
		dispatch_async(dispatch_get_main_queue(), ^{		[UI closeUI];		});
		return;
	}

	NSArray * invalidContent = [self insertContent:content ofItem:item intoZipfile:file withUIQuery:UI];

	if([UI haveCanceled])
	{
		dispatch_async(dispatch_get_main_queue(), ^{		[UI closeUI];		});
		closeZip(file);
		remove([path UTF8String]);
		return;
	}
	else
		dispatch_async(dispatch_get_main_queue(), ^{		[UI finishing];		});

	//Fuck, invalid content, we need to update the manifest :X
	if([invalidContent count])
	{
		for(NSDictionary * entry in invalidContent)
			[content removeObject:entry];

		if([content count])
			manifest = @{	RAK_STRING_VERSION 	: @(ARCHIVE_VERSION),
						 	RAK_STRING_METADATA : [manifest objectForKey:RAK_STRING_METADATA],
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
		[self insertThumbnails:thumbnails inZip:file];

		if(!addMemToZip(file, METADATA_FILE, [serializedFile bytes], [serializedFile length]))
		{
			NSLog(@"Couln't output the manifest :/");
			closeZip(file);
			remove([path UTF8String]);
		}
		else
			zipClose(file, "Generated using "PROJECT_NAME" v"PRINTABLE_VERSION);
	}

	[UI closeUI];
}

#pragma mark - Creation of the manifest

+ (NSDictionary *) generateManifestForItem : (RakDragItem *) item contentDetail : (NSMutableArray **) content thumbFiles : (NSArray **) imagesPath
{
	PROJECT_DATA projectData = item.project;

	NSDictionary * project = [self lineariseProject:projectData ofID:0 withImages:imagesPath];
	if(project == nil)
		return nil;

	uint index = 0;

	*content = [self lineariseContent:&projectData ofProjectID:0 fullProject:item.fullProject isTome:item.isTome selection:item.selection index:&index];
	if(*content == nil)
		return nil;

	item.project = projectData;

	//Crafting the final file
	return @{	RAK_STRING_VERSION 	: @(ARCHIVE_VERSION),
				RAK_STRING_METADATA : @{RAK_STRING_METADATA_PROJECT: @[project]},
				RAK_STRING_CONTENT	: *content};
}

+ (NSDictionary *) lineariseProject : (PROJECT_DATA) project ofID : (uint) allocatedID  withImages : (NSArray **) imagesPath
{
	//Build the main metadata section
	NSMutableDictionary * dict = [NSMutableDictionary new];

	[dict setObject:@(allocatedID) forKey:RAK_STRING_METADATA_PROJECTID];
	[dict setObject:getStringForWchar(project.projectName) forKey:RAK_STRING_METADATA_PROJECTNAME];

	if(!isLocalRepo(project.repo))
	{
		[dict setObject:@(project.repo->type) forKey:RAK_STRING_METADATA_REPOTYPE];
		[dict setObject:[NSString stringWithUTF8String:project.repo->URL] forKey:RAK_STRING_METADATA_REPOURL];

		if(!isLocalProject(project))
			[dict setObject:@(project.projectID) forKey:RAK_STRING_METADATA_REPO_PROJID];
	}

	if(project.description[0])
		[dict setObject:getStringForWchar(project.description) forKey:RAK_STRING_METADATA_DESCRIPTION];

	[dict setObject:getStringForWchar(project.authorName) forKey:RAK_STRING_METADATA_AUTHOR];

	if(project.status != STATUS_INVALID)
		[dict setObject:@(project.status) forKey:RAK_STRING_METADATA_STATUS];

	[dict setObject:@(project.category) forKey:RAK_STRING_METADATA_CATEGORY];
	[dict setObject:reverseTag(project) forKey:RAK_STRING_METADATA_TAG_DATA];
	[dict setObject:@(project.rightToLeft) forKey:RAK_STRING_METADATA_RIGHT2LEFT];

	//Okay, now, we want to insert the thumbnails
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
			NSString * outputPath = [NSString stringWithFormat:@BASE_THUMBS_PATH"%d-%d", allocatedID, count++];

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
					[dict setObject:outputPath forKey:sectionName];
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

+ (NSMutableArray *) lineariseContent : (PROJECT_DATA *) project ofProjectID : (uint) projectID fullProject : (BOOL) fullProject isTome : (BOOL) isTome selection : (uint) selection index : (uint *) index
{
	//Ensure we have valid data
	if((fullProject || isTome) && project->tomesInstalled == NULL)
	{
		if(project->tomesFull == NULL)
			getUpdatedTomeList(project, true);
		else
			getTomeInstalled(project, NULL);
	}

	if((fullProject || !isTome) && project->chapitresInstalled == NULL)
	{
		if(project->chapitresFull == NULL)
			getUpdatedChapterList(project, true);
		else
			getChapterInstalled(project, NULL);
	}

	NSMutableArray * output = nil;

	//To insert a full project, we first inject the
	if(fullProject)
	{
		selection = INVALID_VALUE;
		isTome = false;
	}

	//Okay, at this point, we craft the array
	if(selection == INVALID_VALUE)
	{
		uint length = ACCESS_DATA(isTome, project->nombreChapitreInstalled, project->nombreTomesInstalled);
		NSMutableArray * collector = [NSMutableArray new];

		for(uint i = 0; i < length; ++i)
		{
			NSDictionary * dict = linearizeContentLine(*project, projectID, isTome, ACCESS_DATA(isTome,  (void *) &project->chapitresInstalled[i], &project->tomesInstalled[i]), index);
			if(dict != nil)
				[collector addObject:dict];
		}

		if(fullProject)
		{
			length = project->nombreTomesInstalled;
			for(uint i = 0; i < length; ++i)
			{
				NSDictionary * dict = linearizeContentLine(*project, projectID, true, &project->tomesInstalled[i], index);
				if(dict != nil)
					[collector addObject:dict];
			}
		}

		if([collector count] > 0)
			output = collector;
	}
	else
	{
		NSDictionary * dict = linearizeContentLine(*project, projectID, isTome, &selection, index);
		if(dict != nil)
			output = [NSMutableArray arrayWithObject:dict];
	}

	return output;
}

#pragma mark - Insertion into zip

+ (NSArray *) insertContent : (NSMutableArray *) content ofItem : (RakDragItem *) item intoZipfile : (zipFile *) file withUIQuery : (RakExportStatusController *) controller
{
	NSMutableArray * invalidContent = [NSMutableArray new];

	BOOL firstRound = YES;
	controller.nbElementToExport = [content count];
	controller.posInExport = 0;

	for(NSDictionary * entry in content)
	{
		if(controller.haveCanceled)
			break;

		if(firstRound)
			firstRound = NO;
		else
			controller.posInExport++;

		//Load the data so we get a list of files
		DATA_LECTURE entryData;
		BOOL isTome = [objectForKey(entry, RAK_STRING_CONTENT_ISTOME, nil, [NSNumber class]) boolValue];
		uint selection = [objectForKey(entry, RAK_STRING_CONTENT_ID, nil, [NSNumber class]) unsignedIntValue];

		if(!configFileLoader(item.project, isTome, selection, &entryData))
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

		//If it is a volume, we need those metadata
		META_TOME volumeMetadata;
		if(isTome)
		{
			uint posSelectionTome = getPosForID(item.project, true, selection);
			if(posSelectionTome == INVALID_VALUE)
			{
				releaseDataReader(&entryData);
				[invalidContent addObject:entry];
				continue;
			}

			volumeMetadata = item.project.tomesInstalled[posSelectionTome];
		}

		//Okay, all files exists, let's add them to the archive
		NSString * _rootPath = [objectForKey(entry, RAK_STRING_CONTENT_DIRECTORY, nil, [NSString class]) stringByAppendingString:@"/"];
		const char * rootInzipPath = [_rootPath UTF8String];
		const uint rootZipPathLength = [_rootPath length];

		//First, create the directory
		createDirInZip(file, rootInzipPath);

		//Initialize the UI
		controller.nbElementInEntry = entryData.nombrePage;

		//Then add those files
		BOOL error = NO;
		char * outFile, * baseInzipPath = NULL;
		uint baseInzipVolPathLength;
		for(uint pos = 0, basePagePathLength = strlen(entryData.path[0]), chunkCount = 0; pos < entryData.nombrePage && !error; pos++)
		{
			controller.posInEntry = pos;

			if(isTome)
			{
				//We must detect shared chapters and throw them into native/
				if(baseInzipPath == NULL)
				{
					CONTENT_TOME volumeContent = volumeMetadata.details[chunkCount];
					char baseInzipPathTmp[rootZipPathLength + 100], chapterID[32];

					if(volumeContent.ID % 10)
						snprintf(chapterID, sizeof(chapterID), "%d.%d", volumeContent.ID / 10, volumeContent.ID % 10);
					else
						snprintf(chapterID, sizeof(chapterID), "%d", volumeContent.ID / 10);

					if(volumeContent.isPrivate)
						baseInzipVolPathLength = (uint) snprintf(baseInzipPathTmp, sizeof(baseInzipPathTmp), "%s"VOLUME_PREFIX"%d/"CHAPTER_PREFIX"%s/", rootInzipPath, volumeMetadata.ID, chapterID);
					else
						baseInzipVolPathLength = (uint) snprintf(baseInzipPathTmp, sizeof(baseInzipPathTmp), "%s"VOLUME_PREFIX"%d/"VOLUME_PRESHARED_DIR"/"CHAPTER_PREFIX"%s/", rootInzipPath, volumeMetadata.ID, chapterID);

					baseInzipPath = strdup(baseInzipPathTmp);
				}

				char inzipOfFile[baseInzipVolPathLength + 50];
				snprintf(inzipOfFile, sizeof(inzipOfFile), "%s%s", baseInzipPath, &(entryData.nomPages[pos][basePagePathLength + 1]));
				outFile = strdup(inzipOfFile);
			}
			else
			{
				char inzipOfFile[rootZipPathLength + strlen(&(entryData.nomPages[pos][basePagePathLength])) + 2];
				snprintf(inzipOfFile, sizeof(inzipOfFile), "%s%s", rootInzipPath, &(entryData.nomPages[pos][basePagePathLength + 1]));
				outFile = strdup(inzipOfFile);
			}

			//Add the file to the zip, then cleanup
			if(!addFileToZip(file, entryData.nomPages[pos], outFile))
				error = YES;

			free(outFile);

			//If we need to insert the config.dat file
			if(pos + 1 == entryData.nombrePage || entryData.pathNumber[pos] != entryData.pathNumber[pos + 1])
			{
				char pathToConfig[basePagePathLength + 50], inzipOfConfig[rootZipPathLength + 50];
				snprintf(pathToConfig, sizeof(pathToConfig), "%s/"CONFIGFILE, entryData.path[entryData.pathNumber[pos]]);
				snprintf(inzipOfConfig, sizeof(inzipOfConfig), "%s"CONFIGFILE, isTome ? baseInzipPath : rootInzipPath);

				if(!addFileToZip(file, pathToConfig, inzipOfConfig))
					error = YES;

				if(pos + 1 != entryData.nombrePage)
					basePagePathLength = strlen(entryData.path[entryData.pathNumber[pos + 1]]);

				//Volumes use a different root than the rootInzipPath
				if(isTome)
				{
					chunkCount++;
					free(baseInzipPath);
					baseInzipPath = NULL;
				}
			}
		}

		if(error)
			break;
	}

	return [NSArray arrayWithArray:invalidContent];
}

+ (void) insertThumbnails : (NSArray *) thumbnails inZip : (zipFile *) file
{
	if(file == NULL || ![thumbnails count])
		return;

	createDirInZip(file, BASE_THUMBS_PATH);

	for (NSDictionary * entry in thumbnails)
	{
		if(ARE_CLASSES_DIFFERENT(entry, [NSDictionary class]))
			continue;

		addFileToZip(file, [[entry objectForKey:@"origin"] UTF8String], [[entry objectForKey:@"output"] UTF8String]);
	}
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

NSDictionary * linearizeContentLine(PROJECT_DATA project, uint projectID, BOOL isTome, void * selection, uint * index)
{
	NSMutableDictionary * dict = [NSMutableDictionary new];

	[dict setObject:@(projectID) forKey:RAK_STRING_CONTENT_PROJECT];
	[dict setObject:[NSString stringWithFormat:@"%d", (*index)++] forKey:RAK_STRING_CONTENT_DIRECTORY];
	[dict setObject:@(isTome) forKey:RAK_STRING_CONTENT_ISTOME];

	if(isTome)
	{
		META_TOME * tome = selection;
		
		uint pos = getPosForID(project, true, tome->ID);
		if(pos == INVALID_VALUE)
			return nil;
		
		[dict setObject:@(tome->ID) forKey:RAK_STRING_CONTENT_ID];
		
		NSArray * volumeMetadata = recoverVolumeBloc(&(project.tomesInstalled[pos]), 1, project.isPaid);
		if(volumeMetadata != nil && [volumeMetadata count] > 0)
			[dict setObject:[volumeMetadata objectAtIndex:0] forKey:RAK_STRING_CONTENT_VOL_DETAILS];
	}
	else
		[dict setObject:@(* (int *) selection) forKey:RAK_STRING_CONTENT_ID];

	return [NSDictionary dictionaryWithDictionary:dict];
}