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

#import "RakMDLRessources.h"
#import "RakMDLController.h"
#import "RakMDLListView.h"
#import "RakMDLList.h"
#import "RakMDLView.h"

@interface MDL : RakTabView
{
	RakMDLView * coreView;
	RakMDLController * controller;
	
	BOOL needUpdateMainViews;
}

- (id)init : (NSView*)contentView : (NSString *) state;
- (void) initContent : (NSString *) state;
- (BOOL) available;
- (void) wakeUp;

- (void) proxyAddElement : (MANGAS_DATA) data : (bool) isTome : (int) newElem : (bool) partOfBatch;
- (BOOL) proxyCheckForCollision : (MANGAS_DATA) data : (BOOL) isTome : (int) element;

- (NSRect) getCoreviewFrame : (NSRect) frame;

- (void) updateDependingViews;

- (void) propagateContextUpdate : (MANGAS_DATA) data : (bool) isTome : (int) element;

@end
