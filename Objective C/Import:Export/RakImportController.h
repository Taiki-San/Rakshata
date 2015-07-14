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

#import "RakImportStatusController.h"

@interface RakImportController : NSObject

+ (void) importFile : (NSString *) file : (BOOL) generatedArchive;

@end

@interface RakImportItem : NSObject

@property NSString * path;
@property PROJECT_DATA_EXTRA projectData;
@property int contentID;
@property BOOL isTome;

- (BOOL) isReadable;
- (BOOL) needMoreData;

- (BOOL) install : (unzFile *) archive withUI : (RakImportStatusController *) UI;
- (void) deleteData;
- (void) processThumbs : (unzFile *) archive;
- (void) registerProject;

@end
