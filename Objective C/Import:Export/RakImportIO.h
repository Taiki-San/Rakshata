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

@protocol RakImportIO <NSObject>

@required

//Craft a list of the content of the dataset to import
- (NSArray * __nullable) getManifest;

//Check the file exist
- (BOOL) canLocateFile : (NSString * __nonnull) file;

//Iterate a directory with a block, after first calling a block with the number of item to be evaluate (UI feedback)
- (void) evaluateItemFromDir : (NSString * __nonnull) dirName withInitBlock : (void (^__nonnull)(uint nbItems))initBlock andWithBlock : (void (^ __nonnull)(id<RakImportIO> __nonnull controller, NSString * __nonnull filename, uint index, BOOL * __nonnull stop))workingBlock;

//Copy a file from the entity to a directory
- (BOOL) copyItemOfName : (NSString * __nonnull) name toDir : (NSString * __nonnull) dirName;

//Copy a file from the entity to a file pointed by path
- (BOOL) copyItemOfName : (NSString * __nonnull) name toPath : (NSString * __nonnull) path;

//Copy a file from the entity to a NSData
- (BOOL) copyItemOfName : (NSString * __nullable) name toData : (NSData * __nullable * __nonnull) data;

@optional
//Used by the zip layer to move to the begining
- (void) willStartEvaluateFromScratch;

@end

@interface RakImportDotRakController : NSObject <RakImportIO>
{
	unzFile * archive;

	char ** filenames;
	uint nbFiles;
}

- (instancetype __nullable) initWithArchive : (unzFile __nullable * __nullable) file;

@end

