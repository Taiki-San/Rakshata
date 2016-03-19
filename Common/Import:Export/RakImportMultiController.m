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

@implementation RakImportMultiController

- (instancetype) initWithRarFilename : (NSString *) filename
{
	return [self initWithInternalFilename:filename withOption:LIB_UNARR];
}

- (instancetype) initWithMultiFilename : (NSString *) filename
{
	return [self initWithInternalFilename:filename withOption:LIB_LIBARCHIVE];
}

- (instancetype) initWithInternalFilename : (NSString *) filename withOption : (byte) option
{
	self = [super init];

	if(self != nil)
	{
		archive = openArchiveFromFile([filename UTF8String], option);
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

- (BOOL) canLocateFile : (NSString * __nonnull) file
{
	return [archiveFileName isEqualToString:file] || fileExistInArchive(archive, [file UTF8String]);
}

- (void) evaluateItem : (RakImportItem * __nonnull) item forDir : (NSString * __nonnull) dirName withInitBlock : (void (^__nonnull)(uint nbItems, uint iteration))initBlock andWithBlock : (void (^ __nonnull)(id<RakImportIO> __nonnull controller, NSString * __nonnull filename, uint index, BOOL * __nonnull stop))workingBlock
{
	bool couldFindDirInArray = false;
	const char * startExpectedPath = dirName == nil || [dirName length] == 0 ? NULL : [dirName UTF8String];
	uint lengthExpected = startExpectedPath != NULL ? strlen(startExpectedPath) : 0, nbFileToEvaluate = 0, indexOfFiles[archive->nbFiles];

	//We gather the indexes of the files we'll evaluate
	for(uint pos = 0; pos < archive->nbFiles; pos++)
	{
		if(!isStringLongerOrAsLongThan(archive->fileList[pos], lengthExpected))
			continue;
		
		if(startExpectedPath == NULL || !strncmp(archive->fileList[pos], startExpectedPath, lengthExpected))
		{
			if(archive->fileList[pos][lengthExpected] != '\0')
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

	initBlock(nbFileToEvaluate, 0);

	BOOL abort = NO;
	for (uint pos = 0; pos < nbFileToEvaluate && !abort; pos++)
	{
		NSString * string = [NSString stringWithUTF8String:archive->fileList[indexOfFiles[pos]]];
		
		if([string isEqualToString:[dirName stringByAppendingString:@"/"]] || !archiveLocateFile(archive, archive->fileList[indexOfFiles[pos]]))
			continue;

		workingBlock(self, string, pos, &abort);
	}
}

- (BOOL) copyItemOfName : (NSString * __nonnull) name toDir : (NSString * __nonnull) dirName
{
	return [self copyItemOfName:name toPath:[NSString stringWithFormat:@"%@/%@", dirName, [name lastPathComponent]]];
}

- (BOOL) copyItemOfName : (NSString * __nonnull) name toPath : (NSString * __nonnull) path
{
	return archiveExtractOnefile(archive, [name UTF8String], [path UTF8String]);
}

- (BOOL) copyItemOfName : (NSString * __nullable) name toData : (NSData * __nullable * __nonnull) data
{
	byte * dataBytes = NULL;
	uint64_t length = 0;
	BOOL retValue = archiveExtractToMem(archive, [name UTF8String], &dataBytes, &length);

	*data = [NSData dataWithBytes:dataBytes length:length];
	free(dataBytes);

	return retValue && length > 0;
}

- (void) willStartEvaluateFromScratch
{
	archive->utils.jump_back_begining(archive);
}

- (RakImportNode *) getNode
{
	return importDataForFiles([archiveFileName UTF8String], archive->fileList, archive->nbFiles, self);
}

- (void) generateConfigDatInPath : (NSString *) path
{
    createIOConfigDatForData(path, archive->fileList, archive->nbFiles);
}

@end