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

@implementation RakImportDirController

- (instancetype) initWithDirname : (NSString *) dirname
{
	if([dirname length] == 0)
		return nil;

	self = [super init];

	if(self != nil)
	{
		filenames = listDir([dirname UTF8String], &nbFiles);
		if(filenames == NULL || nbFiles == 0 || nbFiles + 1 < nbFiles)
			return nil;

		//We add the directory to the filename list
		void * tmp = realloc(filenames, (nbFiles + 1) * sizeof(char *));
		if(tmp == NULL)		//Dealloc will clean the memory for us
			return nil;

		//Offset everything
		filenames = tmp;
		for(uint i = nbFiles++; i-- > 0;)
			filenames[i + 1] = filenames[i];
        
		//Add / at the end if needed
		if([dirname characterAtIndex:[dirname length] - 1] != '/')
			dirname = [dirname stringByAppendingString:@"/"];

		filenames[0] = strdup([dirname UTF8String]);
		if(filenames[0] == NULL)
			return nil;

		qsort(filenames, nbFiles, sizeof(char *), strnatcmp);
		archiveFileName = dirname;
	}

	return self;
}

- (instancetype) initWithFilename : (NSString *) filename
{
	if([filename length] == 0 || !checkFileExist([filename UTF8String]))
		return nil;
	
	NSString * dirname = [filename stringByDeletingLastPathComponent];
	if([dirname length] == 0 || !checkDirExist([dirname UTF8String]))
		return nil;
	
	self = [super init];
	
	if(self != nil)
	{
		nbFiles = 1;

		filenames = malloc(sizeof(char *));
		if(filenames == NULL)
			return nil;
		
		filenames[0] = strdup([filename UTF8String]);
		if(filenames[0] == NULL)
			return nil;
		
		archiveFileName = [filename copy];
	}
	
	return self;
}

- (BOOL) acceptPackageInPackage
{
	return YES;
}

- (void) dealloc
{
	if(filenames != NULL)
	{
		while(nbFiles-- > 0)
			free(filenames[nbFiles]);

		free(filenames);
	}
}

#pragma mark - RakImportIO conformance

- (BOOL) canLocateFile : (NSString * __nonnull) file
{
	return checkFileExist([file UTF8String]);
}

- (void) evaluateItem : (RakImportItem * __nonnull) item forDir : (NSString * __nonnull) dirName withInitBlock : (void (^__nonnull)(uint nbItems, uint iteration))initBlock andWithBlock : (void (^ __nonnull)(id<RakImportIO> __nonnull controller, NSString * __nonnull filename, uint index, BOOL * __nonnull stop))workingBlock
{
	if(dirName != nil && ![dirName isDirectory])
		dirName = [dirName stringByDeletingLastPathComponent];

	const char * startExpectedPath = dirName == nil || [dirName length] == 0 ? NULL : [dirName UTF8String];
	uint nbFileToEvaluate, indexOfFiles[nbFiles];

	//We gather the indexes of the files we'll evaluate
	nbFileToEvaluate = [self prepareFilesToUnpack:filenames
							   totalNumberOfFiles:nbFiles
										 fromPath:startExpectedPath
								 writeToIndexList:indexOfFiles
									couldFindADir:NULL];
	
	//Nothing to evaluate...
	if(nbFileToEvaluate == 0)
		return;

	if(initBlock != nil)
		initBlock(nbFileToEvaluate, 0);

	BOOL abort = NO;
	for (uint pos = 0; pos < nbFileToEvaluate && !abort; pos++)
	{

		if(checkFileExist(filenames[indexOfFiles[pos]]))
		{
			NSString * currentFilename = [NSString stringWithUTF8String:filenames[indexOfFiles[pos]]];
			
			if(currentFilename != nil)
				workingBlock(self, currentFilename, pos, &abort);
			
		}
	}
}

- (BOOL) copyItemOfName : (NSString * __nonnull) name toDir : (NSString * __nonnull) dirName
{
	NSData * data = nil;

	if(![self copyItemOfName:name toData:&data])
		return NO;

	return [data writeToFile:[NSString stringWithFormat:@"%@/%@", dirName, [name lastPathComponent]] atomically:NO];
}

- (BOOL) copyItemOfName : (NSString * __nonnull) name toPath : (NSString * __nonnull) path
{
	NSData * data = nil;

	if(![self copyItemOfName:name toData:&data])
		return NO;

	return [data writeToFile:path atomically:NO];
}

- (BOOL) copyItemOfName : (NSString * __nullable) name toData : (NSData * __nullable * __nonnull) data
{
	*data = [NSData dataWithContentsOfFile:name];
	return *data != nil;
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
