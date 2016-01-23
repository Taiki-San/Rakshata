/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

@class MDL;
@class RakReaderControllerUIQuery;
@class RakMDLList;

#import "RakMDLController.h"

#if !TARGET_OS_IPHONE
#import "RakMDLListView.h"
#import "RakMDLList.h"
#import "RakMDLView.h"
#import "RakMDLFooter.h"
#endif

@interface MDL : RakTabView
{
	BOOL seriesCollapsedBySetting;

	RakMDLController * controller;

#if !TARGET_OS_IPHONE
	RakMDLView * coreView;
	RakMDLFooter * footer;

	RakReaderControllerUIQuery * _popover;
#endif
}

@property BOOL forcedToShowUp;
@property BOOL needUpdateMainViews;
@property (nonatomic) BOOL wantCollapse;
@property (readonly) BOOL isDisplayed;

- (instancetype) init : (RakView*)contentView : (NSString *) state;
- (BOOL) initContent : (NSString *) state;
- (BOOL) available;
- (void) wakeUp;

- (void) proxyAddElement : (uint) cacheDBID  isTome : (BOOL) isTome element : (uint) newElem  partOfBatch : (BOOL) partOfBatch;
- (BOOL) proxyCheckForCollision : (PROJECT_DATA) data : (BOOL) isTome : (uint) element;
- (void) removingEmailAddress;

#if !TARGET_OS_IPHONE
- (void) updateDependingViews : (BOOL) animated;

- (void) propagateContextUpdate : (PROJECT_DATA) data : (BOOL) isTome : (uint) element;
- (void) registerPopoverExistance : (RakReaderControllerUIQuery*) popover;
#endif

@end
