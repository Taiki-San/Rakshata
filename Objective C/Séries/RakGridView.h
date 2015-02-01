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
 ********************************************************************************************/

enum
{
	RCVC_OFFSET = 10,
	RCVC_MINIMUM_WIDTH = 150,
	RCVC_MINIMUM_WIDTH_OFFSET = (RCVC_MINIMUM_WIDTH + RCVC_OFFSET),
	RCVC_MINIMUM_HEIGHT = 200,
	RCVC_MINIMUM_HEIGHT_OFFSET = (RCVC_MINIMUM_HEIGHT + RCVC_OFFSET),
};

#define SR_NOTIFICATION_TAG 	@"RakSRTagSelectionNotification"
#define SR_NOTIFICATION_AUTHOR 	@"RakSRAuthorSelectionNotification"
#define SR_NOTIFICATION_PROJECT @"RakSRProjectSelectionNotification"

#define SR_NOTIF_CACHEID 		@"cacheID"

#import "RakCollectionView.h"
#import "RakCollectionViewItem.h"

@interface RakGridView : NSObject
{
	RakListScrollView * scrollview;
	
	RakCollectionView * collection;
}

@property (readonly) RakListScrollView * contentView;
@property (getter=isHidden) BOOL hidden;

- (instancetype) initWithFrame:(NSRect)frameRect;

- (void) setFrame : (NSRect) frameRect;
- (void) resizeAnimation : (NSRect) frameRect;

@end
