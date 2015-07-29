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

@interface RakImportRarController : NSObject <RakImportIO>
{
	ARCHIVE * archive;
}

@end

@implementation RakImportRarController

- (instancetype) initWithFilename : (NSString *) filename
{
	self = [super init];

	if(self != nil)
	{
		archive = openArchiveFromFile([filename UTF8String]);
		if(archive == NULL)
			return nil;
	}

	return self;
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

}

- (BOOL) copyItemOfName : (NSString * __nonnull) name toDir : (NSString * __nonnull) dirName
{
	return YES;
}

- (BOOL) copyItemOfName : (NSString * __nonnull) name toPath : (NSString * __nonnull) path
{
	return YES;
}

- (BOOL) copyItemOfName : (NSString * __nullable) name toData : (NSData * __nullable * __nonnull) data;
{
	return YES;
}

- (void) willStartEvaluateFromScratch
{

}


@end