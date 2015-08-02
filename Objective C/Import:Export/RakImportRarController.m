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

@implementation RakImportRarController

- (instancetype) initWithFilename : (NSString *) filename
{
	self = [super init];

	if(self != nil)
	{
		archive = openArchiveFromFile([filename UTF8String]);
		if(archive == NULL)
			return nil;

		archiveFileName = [[filename lastPathComponent] stringByDeletingPathExtension];
	}

	return self;
}

- (void) dealloc
{
	closeArchive(archive);
}

#pragma mark - RakImportIO conformance

- (NSArray * __nullable) getManifest
{
	return nil;
}

- (BOOL) canLocateFile : (NSString * __nonnull) file
{
	return fileExistInArchive(archive, [file UTF8String]);
}

- (void) evaluateItemFromDir : (NSString * __nonnull) dirName withInitBlock : (void (^__nonnull)(uint nbItems))initBlock andWithBlock : (void (^ __nonnull)(id<RakImportIO> __nonnull controller, NSString * __nonnull filename, uint index, BOOL * __nonnull stop))workingBlock
{
	const char * startExpectedPath = [dirName UTF8String];
	uint lengthExpected = strlen(startExpectedPath), nbFileToEvaluate = 0, indexOfFiles[archive->nbFiles];

	//We gather the indexes of the files we'll evaluate
	for(uint pos = 0; pos < archive->nbFiles; pos++)
	{
		if(!strncmp(archive->fileList[pos], startExpectedPath, lengthExpected) && archive->fileList[pos][lengthExpected] != '\0')
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
		void * entry;
		if(rarLocateFile(archive, &entry, archive->fileList[indexOfFiles[pos]]) != ARCHIVE_OK)
			continue;

		archive->cachedEntry = entry;
		workingBlock(self, [NSString stringWithUTF8String:archive->fileList[indexOfFiles[pos]]], pos, &abort);
		archive->cachedEntry = NULL;
	}

}

- (BOOL) copyItemOfName : (NSString * __nonnull) name toDir : (NSString * __nonnull) dirName
{
	NSArray * part = [name componentsSeparatedByString:@"/"];
	NSString * strippedName = [part count] <= 1 ? name : [part objectAtIndex:[part count] - 1];

	return [self copyItemOfName:name toPath:[NSString stringWithFormat:@"%@/%@", dirName, strippedName]];
}

- (BOOL) copyItemOfName : (NSString * __nonnull) name toPath : (NSString * __nonnull) path
{
	return rarExtractOnefile(archive, [name UTF8String], [path UTF8String]);
}

- (BOOL) copyItemOfName : (NSString * __nullable) name toData : (NSData * __nullable * __nonnull) data;
{
	byte * dataBytes = NULL;
	uint64_t length = 0;
	BOOL retValue = rarExtractToMem(archive, [name UTF8String], &dataBytes, &length);

	*data = [NSData dataWithBytes:dataBytes length:length];
	free(dataBytes);

	return retValue && length > 0;
}

- (void) willStartEvaluateFromScratch
{
	rarJumpBackAtBegining(archive);
}

- (IMPORT_NODE) getNode
{
	return importDataForFiles(strdup([archiveFileName UTF8String]), archive->fileList, archive->nbFiles, (__bridge void *) self);
}

@end