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

@class RakImportStatusController;
@class RakImportItem;

#import "RakImportIO.h"
#import "RakImportIOController.h"

@interface RakImportController : NSObject

+ (void) importFile : (NSArray <id <RakImportIO>> *) IOController;
+ (void) postProcessingUI: (RakImportStatusController *) UI;

@end

@interface RakImportItem : NSObject

@property NSString * path;
@property id<RakImportIO> IOController;
@property PROJECT_DATA_EXTRA projectData;
@property uint contentID;
@property BOOL isTome;

@property byte issue;

- (BOOL) isReadable;
- (BOOL) needMoreData;

- (BOOL) installWithUI : (RakImportStatusController *) UI;
- (void) deleteData;
- (BOOL) overrideDuplicate;
- (BOOL) updateProject : (PROJECT_DATA) project;
- (void) refreshState;

- (void) processThumbs;
- (NSData *) queryThumbInWithIndex : (uint) index;

- (void) registerProject;

- (NSString *) inferMetadataFromPathWithHint : (BOOL) haveHintedCT;

@end

@class RakImportQuery;

#import "RakImportStatusList.h"
#import "RakImportQuery.h"
#import "RakImportStatusController.h"

enum
{
	IMPORT_PROBLEM_NONE,
	IMPORT_PROBLEM_DUPLICATE,
	IMPORT_PROBLEM_METADATA,
	IMPORT_PROBLEM_METADATA_DETAILS,
	IMPORT_PROBLEM_INSTALL_ERROR
};
