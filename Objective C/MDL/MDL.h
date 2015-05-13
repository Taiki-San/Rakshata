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

@interface MDL : RakTabView
{
	RakMDLView * coreView;
	RakMDLController * controller;
	
	RakReaderControllerUIQuery * _popover;
}

@property BOOL forcedToShowUp;
@property BOOL needUpdateMainViews;

- (instancetype) init : (NSView*)contentView : (NSString *) state;
- (void) initContent : (NSString *) state;
- (BOOL) available;
- (void) wakeUp;

- (void) proxyAddElement : (PROJECT_DATA) data  isTome : (bool) isTome element : (int) newElem  partOfBatch : (bool) partOfBatch;
- (BOOL) proxyCheckForCollision : (PROJECT_DATA) data : (BOOL) isTome : (int) element;

- (NSRect) getCoreviewFrame : (NSRect) frame;

- (BOOL) isDisplayed;

- (void) updateDependingViews : (BOOL) animated;

- (void) propagateContextUpdate : (PROJECT_DATA) data : (bool) isTome : (int) element;
- (void) registerPopoverExistance : (RakReaderControllerUIQuery*) popover;

@end
