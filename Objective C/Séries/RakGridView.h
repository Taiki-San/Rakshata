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
#define SR_NOTIFICATION_TYPE 	@"RakSRTypeSelectionNotification"
#define SR_NOTIFICATION_AUTHOR 	@"RakSRAuthorSelectionNotification"
#define SR_NOTIFICATION_SOURCE 	@"RakSRSourceSelectionNotification"
#define SR_NOTIFICATION_FLUSH 	@"RakSRFlushSelectionNotification"
#define SR_NOTIFICATION_SELECTION_TRIGGERED	@"RakSRSelectionTriggeredNotification"

#define RCVC_FOCUS_DELAY 		0.2f
#define SR_NOTIFICATION_FOCUS	@"RakSRItemFocusChanged"
#define SR_FOCUS_IN_OR_OUT		@"inOrOut"

#define SR_NOTIF_CACHEID 		@"cacheID"
#define SR_NOTIF_OPTYPE 		@"opType"

#import "RakCollectionView.h"
#import "RakCollectionViewItem.h"

@interface RakGridView : RakDragResponder <NSCollectionViewDelegate>
{
	RakListScrollView * scrollview;
	RakCollectionView * collection;
	
	PROJECT_DATA _dragProject;
	uint _currentDragItem;
	
	NSTrackingArea * trackingArea;
	NSDraggingSession * draggingSession;
}

@property (readonly) RakListScrollView * contentView;
@property (getter=isHidden) BOOL hidden;
@property CGFloat alphaValue;

- (instancetype) initWithFrame : (NSRect) frameRect : (RakSRContentManager *) manager;

- (id) animator;
- (void) setFrame : (NSRect) frameRect;
- (void) resizeAnimation : (NSRect) frameRect;

- (void) updateTrackingArea;
- (void) removeTracking;

@end
