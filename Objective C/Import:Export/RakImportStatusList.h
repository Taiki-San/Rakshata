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

#define NOTIFICATION_IMPORT_STATUS_UI	@"RakImportStatusUpdateUI"
#define NOTIFICATION_CHILD	@"RakImportStatusUpdateChild"
#define NOTIFICATION_ROOT	@"RakImportStatusUpdateRoot"

@interface RakImportStatusListItem : RakOutlineListItem

@property PROJECT_DATA projectData;
@property RakImportItem * itemForChild;
@property byte status;
@property BOOL metadataProblem;

- (instancetype) initWithProject : (PROJECT_DATA) project;
- (void) addItemAsChild : (RakImportItem *) item;
- (void) commitFinalList;
- (byte) checkStatusFromChildren;

@end

@interface RakImportStatusList : RakOutlineList
{
	NSArray * _dataSet;
	NSArray * rootItems;
}

@property BOOL haveDuplicate;
@property (nonatomic) RakImportQuery * query;
@property __weak RakImportStatusController * controller;

- (instancetype) initWithImportList : (NSArray *) dataset;
+ (void) refreshAfterPass;

@end

@interface RakImportStatusListRowView : NSView
{
	RakImportStatusListItem * listItem;

	RakImportItem * _item;
	BOOL isRoot, metadataProblem;

	RakText * projectName;
	RakStatusButton * button;

	__weak RakImportQuery * alert;
}

@property RakImportStatusList * list;

- (void) updateWithItem : (RakImportStatusListItem *) item;
- (byte) status;

@end
