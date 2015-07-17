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

@interface RakImportStatusListItem : RakOutlineListItem

@property PROJECT_DATA projectForRoot;
@property RakImportItem * itemForChild;
@property byte status;

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

- (instancetype) initWithImportList : (NSArray *) dataset;
- (void) refreshAfterPass;

@end
