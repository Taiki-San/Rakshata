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

@class RakImportNode;

@protocol RakImportIO <NSObject>

//Check the file exist
- (BOOL) canLocateFile : (NSString * __nonnull) file;

//Iterate a directory with a block, after first calling a block with the number of item to be evaluate (UI feedback)
- (void) evaluateItemFromDir : (NSString * __nonnull) dirName withInitBlock : (void (^__nonnull)(uint nbItems, BOOL wantBroadWriteAccess))initBlock andWithBlock : (void (^ __nonnull)(id<RakImportIO> __nonnull controller, NSString * __nonnull filename, uint index, BOOL * __nonnull stop))workingBlock;

//Copy a file from the entity to a directory
- (BOOL) copyItemOfName : (NSString * __nonnull) name toDir : (NSString * __nonnull) dirName;

//Copy a file from the entity to a file pointed by path
- (BOOL) copyItemOfName : (NSString * __nonnull) name toPath : (NSString * __nonnull) path;

//Copy a file from the entity to a NSData
- (BOOL) copyItemOfName : (NSString * __nullable) name toData : (NSData * __nullable * __nonnull) data;

//Get the state of the node
- (RakImportNode * __nonnull) getNode;

//Generate the config.dat file if needed
- (void) generateConfigDatInPath : (NSString * __nonnull) path;

@optional
//Used by the zip layer to move to the begining
- (void) willStartEvaluateFromScratch;

@end

//This object/structure (required to have a strong relation with IOController) is used to map the directory structure and infer metadata
//TL;DR: Shaddy shit
@interface RakImportNode : NSObject

@property NSArray<RakImportNode *> * __nullable children;
@property id <RakImportIO> __nonnull IOController;

@property NSString * __nonnull nodeName;

@property uint nbImages;

@property bool isFlatCT;
@property bool couldBeComplexT;
@property bool isValid;

@end

@interface RakImportZipController : NSObject <RakImportIO>
{
	unzFile * archive;
	NSString * archiveFileName;

	char * __nonnull * __nonnull filenames;
	uint nbFiles;
}

- (instancetype __nullable) initWithFilename : (NSString * __nonnull) filename;

@end

@interface RakImportDotRakController : RakImportZipController

//Craft a list of the content of the dataset to import
- (NSArray <RakImportItem *> * __nullable) getManifest;

@end

@interface RakImportRarController : NSObject <RakImportIO>
{
	ARCHIVE * archive;
	NSString * archiveFileName;
}

- (instancetype __nullable) initWithFilename : (NSString * __nonnull) filename;

@end

@interface RakImportDirController : NSObject <RakImportIO>
{
	NSString * archiveFileName;

	char * __nonnull * __nonnull filenames;
	uint nbFiles;
}

- (instancetype __nullable) initWithFilename : (NSString * __nonnull) filename;

@end

id <RakImportIO> __nullable createIOForFilename(NSString * __nonnull filename);
NSArray <RakImportItem *> * __nullable getManifestForIOs(NSArray <id <RakImportIO>> * __nonnull IOControllers);
void createIOConfigDatForData(NSString * __nonnull path, char * __nonnull * __nonnull filenames, uint nbFiles);