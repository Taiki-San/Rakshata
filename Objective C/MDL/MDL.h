/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite  **
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

- (void) proxyAddElement : (MANGAS_DATA) data : (bool) isTome : (int) newElem;

- (NSRect) getCoreviewFrame : (NSRect) frame;

- (void) updateDependingViews;

- (void) propagateContextUpdate : (MANGAS_DATA) data : (bool) isTome : (int) element;

@end
