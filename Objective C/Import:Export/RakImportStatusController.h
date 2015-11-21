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

@interface RakImportStatusController : RakEIStatusController
{
	BOOL windowDidExpand;
}

- (void) switchToIssueUI : (NSArray *) dataSet;

- (NSData *) queryThumbOf : (RakImportItem *) item withIndex : (uint) index;
- (BOOL) reflectMetadataUpdate : (PROJECT_DATA) project withImages : (NSArray *) overridenImages forItem : (RakImportItem *) item;

- (void) postProcessUpdate;

- (void) close;

@end

#define NOTIFICATION_IMPORT_REPLACE_ONE	@"RakImportReplaceOne"
#define NOTIFICATION_IMPORT_REPLACE_ALL	@"RakImportReplaceAll"