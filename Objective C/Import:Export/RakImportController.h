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

#import "RakImportIO.h"
#import "RakImportIOController.h"

@class RakImportStatusController;

@interface RakImportController : NSObject

+ (void) importFile : (id <RakImportIO>) IOController;
+ (void) postProcessingUI: (RakImportStatusController *) UI;

@end

@interface RakImportItem : NSObject

@property NSString * path;
@property PROJECT_DATA_EXTRA projectData;
@property int contentID;
@property BOOL isTome;

@property byte issue;

- (BOOL) isReadable;
- (BOOL) needMoreData;

- (BOOL) install : (id<RakImportIO>) IOControler withUI : (RakImportStatusController *) UI;
- (void) deleteData;
- (BOOL) overrideDuplicate : (id<RakImportIO>) IOControler;
- (BOOL) updateProject : (PROJECT_DATA) project withArchive : (id<RakImportIO>) IOControler;

- (void) processThumbs : (id<RakImportIO>) IOControler;
- (NSData *) queryThumbIn : (id<RakImportIO>) IOControler withIndex : (uint) index;

- (void) registerProject;

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
	IMPORT_PROBLEM_INSTALL_ERROR
};
