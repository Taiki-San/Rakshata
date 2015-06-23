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

enum
{
	PROBLEM_DUPLICATE
};

@implementation RakImportController

+ (void) importFile : (NSString *) filename : (BOOL) generatedArchive
{
	if(filename == nil || !generatedArchive)
		return;

	//We first try to open the file, so we can eventually start working with it
	unzFile * file = unzOpen64([filename UTF8String]);
	if(file == NULL)
		return;

	//Locate the metadata
	if(unzLocateFile(file, METADATA_FILE, true) != UNZ_OK)
	{
		unzClose(file);
		return;
	}

	uint64_t contentSize;
	byte * contentFile;

	if(!extractToMem(file, &contentFile, &contentSize) || contentFile == NULL || contentSize == 0)
	{
		unzClose(file);
		return;
	}

	NSArray * manifest = [self analyzeManifest:[NSData dataWithBytes:contentFile length:contentSize]];

	free(contentFile);
	if(manifest == nil || [manifest count] == 0)
	{
		unzClose(file);
		return;
	}

	//Okay, we iterate through
	NSMutableArray * problems = [NSMutableArray array];
	for(RakImportItem * item in manifest)
	{
		//First, check the file exist in the archive
		if(unzLocateFile(file, [item.path UTF8String], true) != UNZ_OK)
			continue;

		bool locale = item.projectData.data.repo->locale;

		//At this point, we know two things: the project is valid, exist in the archive
		if(!locale && checkReadable(item.projectData.data, item.isTome, item.contentID))
		{
			[problems addObject:@{@"obj" : item, @"reason" : @(PROBLEM_DUPLICATE)}];
		}
		else
		{

		}

		//Well, I guess we can carry on

		if(locale)
		{

		}

	}

	unzClose(file);
}

+ (NSArray *) analyzeManifest : (NSData *) data
{
	NSError * error = nil;
	NSDictionary * dictionnary = [NSJSONSerialization JSONObjectWithData:data options:0 error:&error];

	if(dictionnary == nil)
	{
		NSLog(@"Parse error: %@", error);
		return nil;
	}

	//We check the version is supported
	NSNumber * version = objectForKey(dictionnary, RAK_STRING_VERSION, nil, [NSNumber class]);
	if(version == nil || [version unsignedIntegerValue] > ARCHIVE_VERSION)
	{
		NSLog(@"Unsupported archive, version %@ is higher than the highest supported version %d", version, ARCHIVE_VERSION);
		return nil;
	}
	else
		version = nil;

	//We grab the roots
	NSDictionary * metadata = objectForKey(dictionnary, RAK_STRING_METADATA, nil, [NSDictionary class]);
	NSArray * contentListing = objectForKey(dictionnary, RAK_STRING_CONTENT, nil, [NSArray class]);

	if(metadata == nil || contentListing == nil)
	{
		NSLog(@"Invalid file");
		return nil;
	}
	else
		dictionnary = nil;

	//Okay, by know, the basic structure is okay, we need to build the catalog
	NSArray * linearizedCatalog = objectForKey(metadata, RAK_STRING_METADATA_PROJECT, nil, [NSArray class]);
	if(linearizedCatalog == nil)
	{
		NSLog(@"Invalid catalog");
		return nil;
	}

	//Parse entries
	NSMutableDictionary * catalog = [NSMutableDictionary dictionary];
	NSArray * projectNames = nil;
	for(NSDictionary * entry in linearizedCatalog)
	{
		if(ARE_CLASSES_DIFFERENT(entry, [NSDictionary class]))
			continue;

		//We get the projectID
		NSNumber * entryID = objectForKey(entry, RAK_STRING_METADATA_PROJECTID, nil, [NSNumber class]);
		if(entryID == nil || [catalog objectForKey:entryID])
			continue;

		NSString * entryName = objectForKey(entry, RAK_STRING_METADATA_PROJECTNAME, nil, [NSString class]);
		if(entryName == nil)
			continue;

		PROJECT_DATA_EXTRA * currentProject = calloc(1, sizeof(PROJECT_DATA_EXTRA));
		if(currentProject == NULL)
			continue;

		currentProject->data.projectID = [entryID unsignedIntValue];
		wcsncpy(currentProject->data.projectName, (charType*) [entryName cStringUsingEncoding:NSUTF32StringEncoding], LENGTH_PROJECT_NAME);

		if([self analyseRepo:currentProject :entry :entryName :&projectNames])
			[self analyseMetadata:currentProject :entry];

		[self analyseImages:currentProject :entry];

		//Add to catalog
		[catalog setObject:[NSData dataWithBytesNoCopy:currentProject length:sizeof(PROJECT_DATA_EXTRA) freeWhenDone:YES] forKey:[NSString stringWithFormat:@"%u", [entryID unsignedIntValue]]];
	}

	//Okay! Catalog is built, we can start crawling the directory

	NSMutableArray * output = [NSMutableArray array];
	if(output == nil)
		return nil;

	for(NSDictionary * entry in contentListing)
	{
		if(ARE_CLASSES_DIFFERENT(entry, [NSDictionary class]))
			continue;

		NSData * currentProject;
		NSNumber * projectID = objectForKey(entry, RAK_STRING_CONTENT_PROJECT, nil, [NSNumber class]);
		if(projectID != nil)
		{
			currentProject = [catalog objectForKey:[NSString stringWithFormat:@"%u", [projectID unsignedIntValue]]];
			if(currentProject == nil)	//Project not referenced in the catalog
				continue;
		}
		else
			continue;

		NSString * dirName = objectForKey(entry, RAK_STRING_CONTENT_DIRECTORY, nil, [NSString class]);
		if(dirName == nil)
			continue;

		NSNumber * isTome = objectForKey(entry, RAK_STRING_CONTENT_ISTOME, nil, [NSNumber class]), * entityID = objectForKey(entry, RAK_STRING_CONTENT_ID, nil, [NSNumber class]);

		if(isTome == nil || entityID == nil)
			continue;

		//Okay, everything is complete, we throw it in a data structre and we're good
		RakImportItem * item = [RakImportItem new];
		if(item == nil)
			continue;

		item.path = [dirName stringByAppendingString:@"/"];
		item.projectData = * (PROJECT_DATA_EXTRA *) [currentProject bytes];
		item.isTome = [isTome boolValue];
		item.contentID = [entityID intValue];

		//Duplicate images URL, so they can be freeed later
		for(byte i = 0; i < NB_IMAGES; i++)
		{
			if(item.projectData.haveImages[i])
				item.projectData.URLImages[i] = strdup(item.projectData.URLImages[i]);
		}

		[output addObject:item];
	}

	//Flush the catalog
	[catalog enumerateKeysAndObjectsUsingBlock:^(id  __nonnull key, id  __nonnull obj, BOOL * __nonnull stop) {
		PROJECT_DATA_EXTRA * currentProject = (PROJECT_DATA_EXTRA *) [(NSData *) obj bytes];

		for(byte i = 0; i < NB_IMAGES; i++)
		{
			if(currentProject->haveImages[i])
				free(currentProject->URLImages[i]);
		}
	}];
	[catalog removeAllObjects];

	return [NSArray arrayWithArray:output];
}

#pragma mark - Metadata extraction

+ (BOOL) analyseRepo : (PROJECT_DATA_EXTRA *) currentProject : (NSDictionary *) entry : (NSString *) entryName : (NSArray **) projectNames
{
	NSNumber * entryNumber = objectForKey(entry, RAK_STRING_METADATA_REPOTYPE, nil, [NSNumber class]);
	NSString * entryString = objectForKey(entry, RAK_STRING_METADATA_REPOURL, nil, [NSString class]);
	if(entryNumber != nil && [entryNumber unsignedCharValue] <= MAX_TYPE_DEPOT && entryString != nil)
	{
		REPO_DATA * repo;
		uint64_t repoID = getRepoIndexFromURL([entryString UTF8String]);

		if(repoID == UINT64_MAX)	//Not a known repo
		{
			repo = calloc(1, sizeof(REPO_DATA));
			if(repo != NULL)
			{
				strncpy(repo->URL, [entryString UTF8String], REPO_URL_LENGTH - 1);
				repo->URL[REPO_URL_LENGTH - 1] = 0;
				repo->type = [entryNumber unsignedCharValue];
				repo->locale = true;
			}
		}
		else						//If a known repo
		{
			repo = getRepoForID(repoID);
			if(repo != NULL)
			{
				//If have a indicative project ID
				entryNumber = objectForKey(entry, RAK_STRING_METADATA_REPO_PROJID, nil, [NSNumber class]);
				if(entryNumber != nil)
				{
					PROJECT_DATA * project = _getProjectFromSearch(repoID, [entryNumber unsignedIntValue], false, false);
					if(project != NULL)
					{
						//Oh! The project exist! This is very cool!
						currentProject->data = *project;
						free(project);
						return false;
					}
				}
				else	//Hum, okay, let's see if the title looks like something we know
				{
					if(*projectNames == nil)
						*projectNames = [self buildProjectNamesList];

					if(projectNames != nil)
					{
						NSNumber * repoIDObj = @(repoID);
						__block NSNumber * projectID = nil;

						[*projectNames enumerateObjectsUsingBlock:^(id  __nonnull obj, NSUInteger idx, BOOL * __nonnull stop) {
							if([repoIDObj isEqualToNumber : [obj objectForKey:@"repoID"]] && [entryName caseInsensitiveCompare:[obj objectForKey:@"name"]] == NSOrderedSame)
							{
								projectID = [obj objectForKey:@"ID"];
								*stop = YES;
							}
						}];

						if(projectID != nil)	//Found something, probably that
						{
							currentProject->data = getProjectByIDHelper([projectID unsignedLongLongValue], false);
							return false;
						}
					}
				}
			}
		}

		currentProject->data.repo = repo;
	}

	return true;
}

+ (void) analyseMetadata : (PROJECT_DATA_EXTRA *) currentProject : (NSDictionary *) entry
{
	//Description
	NSString * entryString = objectForKey(entry, RAK_STRING_METADATA_DESCRIPTION, nil, [NSString class]);
	if(entryString != nil)
	{
		wcsncpy(currentProject->data.description, (charType *) [entryString cStringUsingEncoding:NSUTF32StringEncoding], LENGTH_DESCRIPTION - 1);
		currentProject->data.description[LENGTH_DESCRIPTION - 1] = 0;
	}

	//Author
	entryString = objectForKey(entry, RAK_STRING_METADATA_AUTHOR, nil, [NSString class]);
	if(entryString != nil)
	{
		wcsncpy(currentProject->data.authorName, (charType *) [entryString cStringUsingEncoding:NSUTF32StringEncoding], LENGTH_AUTHORS - 1);
		currentProject->data.authorName[LENGTH_AUTHORS - 1] = 0;
	}

	//Status
	NSNumber * entryNumber = objectForKey(entry, RAK_STRING_METADATA_STATUS, nil, [NSNumber class]);
	if(entryNumber != nil)
		currentProject->data.status = [entryNumber unsignedCharValue];

	//TagMask
	entryNumber = objectForKey(entry, RAK_STRING_METADATA_TAGMASK, nil, [NSNumber class]);
	if(entryNumber != nil)
		convertTagMask([entryNumber unsignedLongLongValue], &(currentProject->data.category), &(currentProject->data.tagMask), &(currentProject->data.mainTag));

	//Right to Left
	entryNumber = objectForKey(entry, RAK_STRING_METADATA_ASIANORDER, nil, [NSNumber class]);
	if(entryNumber != nil)
		currentProject->data.rightToLeft = [entryNumber boolValue];
}

+ (void) analyseImages : (PROJECT_DATA_EXTRA *) currentProject : (NSDictionary *) entry
{
	NSString * entryString = objectForKey(entry, RAK_STRING_METADATA_IMG_GRID, nil, [NSString class]);
	if(entryString != nil)
	{
		currentProject->URLImages[0] = strdup([entryString UTF8String]);
		currentProject->haveImages[0] = currentProject->URLImages[0] != NULL;

		entryString = objectForKey(entry, RAK_STRING_METADATA_IMG_GRID_2X, nil, [NSString class]);
		if(entryString != nil)
		{
			currentProject->URLImages[1] = strdup([entryString UTF8String]);
			currentProject->haveImages[1] = currentProject->URLImages[1] != NULL;
		}
	}

	entryString = objectForKey(entry, RAK_STRING_METADATA_IMG_CT, nil, [NSString class]);
	if(entryString != nil)
	{
		currentProject->URLImages[2] = strdup([entryString UTF8String]);
		currentProject->haveImages[2] = currentProject->URLImages[2] != NULL;

		entryString = objectForKey(entry, RAK_STRING_METADATA_IMG_CT_2X, nil, [NSString class]);
		if(entryString != nil)
		{
			currentProject->URLImages[3] = strdup([entryString UTF8String]);
			currentProject->haveImages[3] = currentProject->URLImages[3] != NULL;
		}
	}

	entryString = objectForKey(entry, RAK_STRING_METADATA_IMG_DD, nil, [NSString class]);
	if(entryString != nil)
	{
		currentProject->URLImages[6] = strdup([entryString UTF8String]);
		currentProject->haveImages[6] = currentProject->URLImages[6] != NULL;

		entryString = objectForKey(entry, RAK_STRING_METADATA_IMG_DD_2X, nil, [NSString class]);
		if(entryString != nil)
		{
			currentProject->URLImages[7] = strdup([entryString UTF8String]);
			currentProject->haveImages[7] = currentProject->URLImages[7] != NULL;
		}
	}
}

+ (NSArray *) buildProjectNamesList
{
	uint nbElem;
	PROJECT_DATA * projects = getCopyCache(RDB_EXCLUDE_DYNAMIC, &nbElem);

	if(projects == NULL)
		return nil;

	NSMutableArray * output = [NSMutableArray array];

	for(uint pos = 0; pos < nbElem; pos++)
	{
		if(!projects[pos].isInitialized)
			continue;

		[output addObject:@{@"name" : getStringForWchar(projects[pos].projectName), @"repoID" : @(getRepoID(projects[pos].repo)), @"ID" : @(projects[pos].cacheDBID)}];
	}

	freeProjectData(projects);

	return [NSArray arrayWithArray:output];
}

@end
