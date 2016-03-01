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

@class RakImportStatusController;
@class RakImportItem;

#import "RakImportIO.h"
#import "RakImportIOController.h"

@interface RakImportController : NSObject

+ (void) importFile : (NSArray <RakImportBaseController <RakImportIO> * > *) IOController;
+ (void) postProcessingUI: (RakImportStatusController *) UI;

@end

@interface RakImportItem : NSObject
{
	BOOL didRegister;
}

@property NSString * path;
@property id<RakImportIO> IOController;
@property PROJECT_DATA_EXTRA projectData;
@property uint contentID;
@property BOOL isTome, guessedProject;

@property byte issue;

- (BOOL) isReadable;
- (BOOL) needMoreData;
- (BOOL) checkDetailsMetadata;

- (BOOL) installWithUI : (RakImportStatusController *) UI;
- (void) deleteData;
- (BOOL) overrideDuplicate;
- (BOOL) updateProject : (PROJECT_DATA) project;
- (BOOL) updateCTIDWith : (NSNumber *) rawContentID tomeName : (NSString *) tomeName isTome : (BOOL) isTome;
- (void) refreshState;

- (void) processThumbs;
- (NSData *) queryThumbInWithIndex : (uint) index;

- (void) registerProject;

- (NSString *) inferMetadataFromPathWithHint : (BOOL) haveHintedCT;

@end

@class RakImportQuery;

#import "RakImportStatusList.h"
#import "RakImportStatusController.h"

#if !TARGET_OS_IPHONE
#import "RakImportQuery.h"
#endif

enum
{
	IMPORT_PROBLEM_NONE,
	IMPORT_PROBLEM_DUPLICATE,
	IMPORT_PROBLEM_METADATA,
	IMPORT_PROBLEM_METADATA_DETAILS,
	IMPORT_PROBLEM_INSTALL_ERROR
};
