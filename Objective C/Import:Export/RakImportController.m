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

typedef struct
{
	char * directory;
	PROJECT_DATA_EXTRA project;
	bool isTome;
	int contentID;

} PAYLOAD_CAPSULE;

@implementation RakImportController

+ (void) importFile : (NSString *) filename : (BOOL) generatedArchive
{
	if(filename == nil || !generatedArchive)
		return;

	//We first try to open the file, so we can eventually start working with it
	unzFile * file = unzOpen64([filename cStringUsingEncoding:NSUTF8StringEncoding]);
	if(file == NULL)
		return;

	//Locate the metadata
	if(unzLocateFile(file, METADATA_FILE, false) != UNZ_OK)
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

	NSPointerArray * manifest = [self analyzeManifest:[NSData dataWithBytes:contentFile length:contentSize]];

	free(contentFile);
	if(manifest == nil)
	{
		unzClose(file);
		return;
	}

	unzClose(file);
}

+ (NSPointerArray *) analyzeManifest : (NSData *) data
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
	for(NSDictionary * entry in linearizedCatalog)
	{
		if(ARE_CLASSES_DIFFERENT(entry, [NSDictionary class]))
			continue;

		NSNumber * entryID = objectForKey(entry, RAK_STRING_METADATA_PROJECTID, nil, [NSNumber class]);
		if(entryID == nil || [catalog objectForKey:entryID])
			continue;

		PROJECT_DATA_EXTRA * currentProject = calloc(1, sizeof(PROJECT_DATA_EXTRA));
		if(currentProject == NULL)
			continue;

		currentProject->data.projectID = [entryID unsignedIntValue];

		//Couple indentifying a repo
		NSNumber * entryNumber = objectForKey(entry, RAK_STRING_METADATA_REPOTYPE, nil, [NSNumber class]);
		NSString * entryString = objectForKey(entry, RAK_STRING_METADATA_REPOURL, nil, [NSString class]);
		if(entryNumber != nil && [entryNumber unsignedCharValue] <= MAX_TYPE_DEPOT && entryString != nil)
		{
			REPO_DATA * repo = calloc(1, sizeof(REPO_DATA));
			if(repo != NULL)
			{
				strncpy(repo->URL, [entryString cStringUsingEncoding:NSUTF8StringEncoding], REPO_URL_LENGTH - 1);
				repo->URL[REPO_URL_LENGTH - 1] = 0;
				repo->type = [entryNumber unsignedCharValue];

				currentProject->data.repo = repo;
			}
		}

		//Description
		entryString = objectForKey(entry, RAK_STRING_METADATA_DESCRIPTION, nil, [NSString class]);
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
		entryNumber = objectForKey(entry, RAK_STRING_METADATA_STATUS, nil, [NSNumber class]);
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

		//Images
		entryString = objectForKey(entry, RAK_STRING_METADATA_IMG_GRID, nil, [NSString class]);
		if(entryString != nil)
		{
			currentProject->URLImages[0] = strdup([entryString cStringUsingEncoding:NSUTF8StringEncoding]);
			currentProject->haveImages[0] = currentProject->URLImages[0] != NULL;

			entryString = objectForKey(entry, RAK_STRING_METADATA_IMG_GRID_2X, nil, [NSString class]);
			if(entryString != nil)
			{
				currentProject->URLImages[1] = strdup([entryString cStringUsingEncoding:NSUTF8StringEncoding]);
				currentProject->haveImages[1] = currentProject->URLImages[1] != NULL;
			}
		}

		entryString = objectForKey(entry, RAK_STRING_METADATA_IMG_CT, nil, [NSString class]);
		if(entryString != nil)
		{
			currentProject->URLImages[2] = strdup([entryString cStringUsingEncoding:NSUTF8StringEncoding]);
			currentProject->haveImages[2] = currentProject->URLImages[2] != NULL;

			entryString = objectForKey(entry, RAK_STRING_METADATA_IMG_CT_2X, nil, [NSString class]);
			if(entryString != nil)
			{
				currentProject->URLImages[3] = strdup([entryString cStringUsingEncoding:NSUTF8StringEncoding]);
				currentProject->haveImages[3] = currentProject->URLImages[3] != NULL;
			}
		}

		entryString = objectForKey(entry, RAK_STRING_METADATA_IMG_DD, nil, [NSString class]);
		if(entryString != nil)
		{
			currentProject->URLImages[6] = strdup([entryString cStringUsingEncoding:NSUTF8StringEncoding]);
			currentProject->haveImages[6] = currentProject->URLImages[6] != NULL;

			entryString = objectForKey(entry, RAK_STRING_METADATA_IMG_DD_2X, nil, [NSString class]);
			if(entryString != nil)
			{
				currentProject->URLImages[7] = strdup([entryString cStringUsingEncoding:NSUTF8StringEncoding]);
				currentProject->haveImages[7] = currentProject->URLImages[7] != NULL;
			}
		}

		//Add to catalog
		[catalog insertValue:[NSData dataWithBytesNoCopy:currentProject length:sizeof(PROJECT_DATA_EXTRA) freeWhenDone:YES] inPropertyWithKey:[NSString stringWithFormat:@"%u", [entryID unsignedIntValue]]];
	}

	//Okay! Catalog is built, we can start crawling the directory

	NSPointerArray * output = [NSPointerArray pointerArrayWithOptions:NSPointerFunctionsOpaqueMemory];
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
			currentProject = [catalog objectForKey:projectID];
			if(currentProject == nil)	//Project not referenced in the catalog
				continue;
		}

		NSString * dirName = objectForKey(entry, RAK_STRING_CONTENT_DIRECTORY, nil, [NSString class]);
		if(dirName == nil)
			continue;

		NSNumber * isTome = objectForKey(entry, RAK_STRING_CONTENT_ISTOME, nil, [NSNumber class]), * entityID = objectForKey(entry, RAK_STRING_CONTENT_ID, nil, [NSNumber class]);

		if(isTome == nil || entityID == nil)
			continue;

		//Okay, everything is complete, we throw it in a data structre and we're good
		PAYLOAD_CAPSULE * capsule = malloc(sizeof(PAYLOAD_CAPSULE));
		if(capsule == NULL)
			continue;

		capsule->directory = strdup([dirName cStringUsingEncoding:NSUTF8StringEncoding]);
		capsule->project = * (PROJECT_DATA_EXTRA *) [currentProject bytes];
		capsule->isTome = [isTome boolValue];
		capsule->contentID = [entityID intValue];

		//Duplicate images URL, so they can be freeed later
		for(byte i = 0; i < NB_IMAGES; i++)
		{
			if(capsule->project.haveImages[i])
				capsule->project.URLImages[i] = strdup(capsule->project.URLImages[i]);
		}

		[output addPointer:capsule];
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

	return output;
}

@end
