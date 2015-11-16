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

#define CHAPTER_FOR_IMPORTED_VOLUMES 42

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

- (BOOL) launchPopover : (NSView *) anchor : (RakImportStatusListRowView*) receiver;

@end
