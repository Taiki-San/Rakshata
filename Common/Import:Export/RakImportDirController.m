/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
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
		if([dirname UTF8String][[dirname length] - 1] != '/')
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
		filenames = malloc(2 * sizeof(char *));
		if(filenames == NULL)
			return nil;
		
		if(![dirname isDirectory])
			dirname = [dirname stringByAppendingString:@"/"];
		
		filenames[0] = strdup([dirname UTF8String]);
		if(filenames[0] == NULL)
			return nil;
		
		filenames[1] = strdup([filename UTF8String]);
		if(filenames[1] == NULL)
		{
			free(filenames);
			return nil;
		}
		
		nbFiles = 2;
		
		archiveFileName = dirname;
	}
	
	return self;
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

- (void) evaluateItemFromDir : (NSString * __nonnull) dirName withInitBlock : (void (^__nonnull)(uint nbItems, BOOL wantBroadWriteAccess))initBlock andWithBlock : (void (^ __nonnull)(id<RakImportIO> __nonnull controller, NSString * __nonnull filename, uint index, BOOL * __nonnull stop))workingBlock
{
	const char * startExpectedPath = dirName == nil ? NULL : [dirName UTF8String];
	uint lengthExpected = startExpectedPath == NULL ? 0 : strlen(startExpectedPath), nbFileToEvaluate = 0, indexOfFiles[nbFiles];

	//We gather the indexes of the files we'll evaluate
	for(uint pos = 0; pos < nbFiles; pos++)
	{
		//If dirName == NULL, we send everything
		//The comparaison must be performed in this order because it'll prevent shorter filename to get tested for
		//	being precisely the dir we are trying to probe for (possibly a subdir of the directory that was listed)
		if(startExpectedPath == NULL || (!strncmp(filenames[pos], startExpectedPath, lengthExpected) && filenames[pos][lengthExpected] != '\0'))
		{
			indexOfFiles[nbFileToEvaluate++] = pos;
		}
	}

	//Nothing to evaluate...
	if(nbFileToEvaluate == 0)
		return;

	if(initBlock != nil)
		initBlock(nbFileToEvaluate, NO);

	BOOL abort = NO;
	for (uint pos = 0; pos < nbFileToEvaluate && !abort; pos++)
	{
		if(checkFileExist(filenames[indexOfFiles[pos]]))
			workingBlock(self, [NSString stringWithUTF8String:filenames[indexOfFiles[pos]]], pos, &abort);
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