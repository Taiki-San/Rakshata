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

@class MDL;
@class RakReaderControllerUIQuery;
@class RakMDLList;

#import "RakMDLController.h"
#import "RakMDLListView.h"
#import "RakMDLList.h"
#import "RakMDLView.h"
#import "RakMDLFooter.h"

@interface MDL : RakTabView
{
	BOOL seriesCollapsedBySetting;
	
	RakMDLView * coreView;
	RakMDLFooter * footer;
	RakMDLController * controller;
	
	RakReaderControllerUIQuery * _popover;
}

@property BOOL forcedToShowUp;
@property BOOL needUpdateMainViews;
@property (nonatomic) BOOL wantCollapse;

- (instancetype) init : (NSView*)contentView : (NSString *) state;
- (BOOL) available;
- (void) wakeUp;

- (void) proxyAddElement : (PROJECT_DATA) data  isTome : (BOOL) isTome element : (uint) newElem  partOfBatch : (BOOL) partOfBatch;
- (BOOL) proxyCheckForCollision : (PROJECT_DATA) data : (BOOL) isTome : (uint) element;
- (void) removingEmailAddress;

- (BOOL) isDisplayed;

- (void) updateDependingViews : (BOOL) animated;

- (void) propagateContextUpdate : (PROJECT_DATA) data : (BOOL) isTome : (uint) element;
- (void) registerPopoverExistance : (RakReaderControllerUIQuery*) popover;

@end
