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

@interface RakSRHeaderText : RakMenuText

@end

@interface RakSRSubMenu : RakMenuText

@end

@interface RakTreeView : NSOutlineView
{
	NSRect _defaultFrame;
}

- (void) setDefaultFrame : (NSRect) frame;

@end

@interface RakTableRowView : NSTableRowView

@end

#import "RakButtonMorphic.h"
#import "RakSRSearchBar.h"
#import "RakSerieMainList.h"
#import "RakSerieList.h"
#import "RakSerieView.h"
#import "RakSRTagRail.h"
#import "RakSRHeader.h"
#import "RakSRSearchTab.h"
