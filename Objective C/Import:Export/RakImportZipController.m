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

		//We grab the metadata as we will need them later
		listArchiveContent(archive, &filenames, &nbFiles);

		if(nbFiles == 0)
			return nil;
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

- (NSArray * __nullable) getManifest
{
	return nil;
}

- (BOOL) canLocateFile : (NSString * __nonnull) file
{
	return unzLocateFile(archive, [file UTF8String], true) == UNZ_OK;
}

- (void) evaluateItemFromDir : (NSString * __nonnull) dirName withInitBlock : (void (^)(uint nbItems))initBlock andWithBlock : (void (^)(id<RakImportIO> controller, NSString * filename, uint index, BOOL * stop))workingBlock
{
	const char * startExpectedPath = [dirName UTF8String];
	uint lengthExpected = strlen(startExpectedPath), nbFileToEvaluate = 0, indexOfFiles[nbFiles];

	//We gather the indexes of the files we'll evaluate
	for(uint pos = 0; pos < nbFiles; pos++)
	{
		if(!strncmp(filenames[pos], startExpectedPath, lengthExpected) && filenames[pos][lengthExpected] != '\0')
		{
			indexOfFiles[nbFileToEvaluate++] = pos;
		}
	}

	//Nothing to evaluate...
	if(nbFileToEvaluate == 0)
		return;

	initBlock(nbFileToEvaluate);

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

	extractCurrentfile(archive, NULL, [dirName UTF8String], STRIP_PATH_FIRST, NULL);

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

	extractCurrentfile(archive, NULL, [path UTF8String], STRIP_TRUST_PATH_AS_FILENAME, NULL);

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

	if(!extractToMem(archive, &bytes, &thumbSize))
		return NO;

	*data = [NSData dataWithBytesNoCopy:bytes length:thumbSize freeWhenDone:YES];

	return YES;
}

- (void) willStartEvaluateFromScratch
{
	unzGoToFirstFile(archive);
}

@end