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

#include "JSONParser.h"

@implementation RakImportZipController

- (instancetype) initWithFilename : (NSString *) filename
{
	self = [self init];

	if(self != nil)
	{
		if(filename == NULL)
			return nil;

		archive = unzOpen64([filename UTF8String]);
		if(archive == NULL)
			return nil;

		archiveFileName = [[filename lastPathComponent] stringByDeletingPathExtension];

		//We grab the metadata as we will need them later
		unzListArchiveContent(archive, &filenames, &nbFiles);

		if(nbFiles == 0)
			return nil;

		qsort(filenames, nbFiles, sizeof(char *), strnatcmp);
	}

	return self;
}

- (void) dealloc
{
	if(filenames != NULL && nbFiles != 0)
	{
		for(uint i = 0; i < nbFiles; i++)
			free(filenames[i]);

		free(filenames);
	}

	if(archive != NULL)
		unzClose(archive);
}

#pragma mark - RakImportIO conformance

- (BOOL) canLocateFile : (NSString * __nonnull) file
{
	if([file isEqualToString: archiveFileName])
		return YES;

	return unzLocateFile(archive, [file UTF8String], true) == UNZ_OK;
}

- (void) evaluateItem : (RakImportItem * __nonnull) item forDir : (NSString * __nonnull) dirName withInitBlock : (void (^__nonnull)(uint nbItems, uint iteration))initBlock andWithBlock : (void (^ __nonnull)(id<RakImportIO> __nonnull controller, NSString * __nonnull filename, uint index, BOOL * __nonnull stop))workingBlock
{
	bool couldFindDirInArray = false;
	const char * startExpectedPath = dirName == nil || [dirName length] == 0 ? NULL : [dirName UTF8String];
	uint lengthExpected = startExpectedPath != NULL ? strlen(startExpectedPath) : 0, nbFileToEvaluate = 0, indexOfFiles[nbFiles];

	//We gather the indexes of the files we'll evaluate
	for(uint pos = 0; pos < nbFiles; pos++)
	{
		if(!isStringLongerOrAsLongThan(filenames[pos], lengthExpected))
			continue;
		
		if(startExpectedPath == NULL || !strncmp(filenames[pos], startExpectedPath, lengthExpected))
		{
			if(filenames[pos][lengthExpected] != '\0')
				indexOfFiles[nbFileToEvaluate++] = pos;
			else if(startExpectedPath != NULL)
				couldFindDirInArray = true;
		}
	}

	//Nothing to evaluate...
	if(nbFileToEvaluate == 0)
	{
		if([self noValidFileFoundForDir:startExpectedPath butFoundInFiles:couldFindDirInArray shouldRedirectTo:&dirName])
			[self evaluateItem : (RakImportItem * __nonnull) item forDir:dirName withInitBlock:initBlock andWithBlock:workingBlock];
		
		return;
	}

	initBlock(nbFileToEvaluate, iterationCounter);
	
	BOOL abort = NO;
	for (uint pos = 0; pos < nbFileToEvaluate && !abort; pos++)
	{
		if(unzLocateFile(archive, filenames[indexOfFiles[pos]], true) != UNZ_OK)
			continue;

		workingBlock(self, [NSString stringWithUTF8String:filenames[indexOfFiles[pos]]], pos, &abort);
	}
}

- (BOOL) copyItemOfName : (NSString * __nonnull) name toDir : (NSString * __nonnull) dirName
{
	//Get current item filename
	char filename[1024] = {0};
	if((unzGetCurrentFileInfo64(archive, NULL, filename, sizeof(filename), NULL, 0, NULL, 0)) != UNZ_OK)
		return NO;

	if(strcmp([name UTF8String], filename) && unzLocateFile(archive, [name UTF8String], true) != UNZ_OK)
		return NO;

	unzExtractCurrentFile(archive, NULL, [dirName UTF8String], STRIP_PATH_ALL, NULL);

	return YES;
}

- (BOOL) copyItemOfName : (NSString * __nonnull) name toPath : (NSString * __nonnull) path
{
	//Get current item filename
	char filename[1024] = {0};
	if((unzGetCurrentFileInfo64(archive, NULL, filename, sizeof(filename), NULL, 0, NULL, 0)) != UNZ_OK)
		return NO;

	if(strcmp([name UTF8String], filename) && unzLocateFile(archive, [name UTF8String], true) != UNZ_OK)
		return NO;

	unzExtractCurrentFile(archive, NULL, [path UTF8String], STRIP_TRUST_PATH_AS_FILENAME, NULL);

	return YES;
}

- (BOOL) copyItemOfName : (NSString * __nullable) name toData : (NSData * __nullable * __nonnull) data;
{
	//Get current item filename
	char filename[1024] = {0};
	if((unzGetCurrentFileInfo64(archive, NULL, filename, sizeof(filename), NULL, 0, NULL, 0)) != UNZ_OK)
		return NO;

	if(strcmp([name UTF8String], filename) && unzLocateFile(archive, [name UTF8String], true) != UNZ_OK)
		return NO;

	byte * bytes = NULL;
	uint64_t thumbSize;

	if(!unzExtractToMem(archive, &bytes, &thumbSize))
		return NO;

	*data = [NSData dataWithBytesNoCopy:bytes length:thumbSize freeWhenDone:YES];

	return YES;
}

- (void) willStartEvaluateFromScratch
{
	unzGoToFirstFile(archive);
}

- (RakImportNode *) getNode
{
	return importDataForFiles([archiveFileName UTF8String], filenames, nbFiles, self);
}

- (void) generateConfigDatInPath : (NSString *) path
{
    createIOConfigDatForData(path, filenames, nbFiles);
}

@end