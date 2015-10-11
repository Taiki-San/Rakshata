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

@implementation RakImportDirController

- (instancetype) initWithFilename : (NSString *) filename
{
	if([filename length] == 0)
		return nil;

	self = [super init];

	if(self != nil)
	{
		filenames = listDir([filename UTF8String], &nbFiles);
		if(filenames == NULL || nbFiles == 0 || nbFiles + 1 < nbFiles)
			return nil;

		//We add the directory to the filename list
		void * tmp = realloc(filenames, (nbFiles + 1) * sizeof(char *));
		if(tmp == NULL)		//Dealloc will clean the memory for us
			return nil;

		//Offset everything
		filenames = tmp;
		for(uint i = nbFiles; i-- > 0;)
			filenames[i + 1] = filenames[i];
        
		//Add / at the end if needed
		if([filename UTF8String][[filename length] - 1] != '/')
			filename = [filename stringByAppendingString:@"/"];

		filenames[0] = strdup([filename UTF8String]);
		if(filenames[0] == NULL)
			return nil;

		qsort(filenames, nbFiles, sizeof(char *), strnatcmp);
		archiveFileName = filename;
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
	return importDataForFiles(strdup([archiveFileName UTF8String]), filenames, nbFiles, self);
}

- (void) generateConfigDatInPath : (NSString *) path
{
    createIOConfigDatForData(path, filenames, nbFiles);
}

@end