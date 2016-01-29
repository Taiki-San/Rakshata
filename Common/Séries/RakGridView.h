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
 ********************************************************************************************/

enum
{
	RCVC_OFFSET = 10,
	RCVC_MINIMUM_WIDTH = 150,
	RCVC_MINIMUM_WIDTH_OFFSET = (RCVC_MINIMUM_WIDTH + RCVC_OFFSET),
	RCVC_MINIMUM_HEIGHT = 200,
	RCVC_MINIMUM_HEIGHT_OFFSET = (RCVC_MINIMUM_HEIGHT + RCVC_OFFSET),
};

#define RCVC_FOCUS_DELAY 		0.2f
#define SR_NOTIFICATION_FOCUS	@"RakSRItemFocusChanged"
#define SR_FOCUS_IN_OR_OUT		@"inOrOut"

#define SR_NOTIF_CACHEID 		@"cacheID"
#define SR_NOTIF_OPTYPE 		@"opType"
#define SR_NOTIF_UNSELECT 		@"evolution"

#import "RakCollectionView.h"
#import "RakCollectionViewItem.h"

@interface RakGridView : RakDragResponder <NSCollectionViewDelegate>
{
	RakListScrollView * scrollview;
	RakCollectionView * collection;
	
	PROJECT_DATA _dragProject;
	uint _currentDragItem;
	
	RakText * backgroundText;
	
	NSTrackingArea * trackingArea;
	NSDraggingSession * draggingSession;
}

@property (nonatomic) BOOL wasEmpty;
@property (readonly) RakListScrollView * contentView;
@property (getter=isHidden) BOOL hidden;
@property CGFloat alphaValue;

- (instancetype) initWithFrame : (NSRect) frameRect : (RakSRContentManager *) manager;

- (id) animator;
- (void) setAlphaAnimated : (CGFloat) alpha;
- (void) setFrame : (NSRect) frameRect;
- (void) resizeAnimation : (NSRect) frameRect;

- (void) updateTrackingArea;
- (void) removeTracking;

@end
