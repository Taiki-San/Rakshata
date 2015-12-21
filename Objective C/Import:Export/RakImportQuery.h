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

@interface RakImportQuery : RakPopoverView
{
	PROJECT_DATA _project;
}

@property RakImportItem * itemOfQueryForMetadata;
@property RakImportStatusController * controller;

- (instancetype) autoInitWithDuplicate : (RakImportItem *) item;
- (instancetype) autoInitWithDetails : (RakImportItem *) item;
- (instancetype) autoInitWithMetadata : (PROJECT_DATA) project;

- (void) setupUIMetadata;
- (NSString *) stringTitleMetadata;
- (PROJECT_DATA) exfilterProject : (NSArray **) images;
- (void) validateMetadata;

- (BOOL) launchPopover : (RakView *) anchor : (RakImportStatusListRowView*) receiver;

@end
