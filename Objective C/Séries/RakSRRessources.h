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

@interface RakTableRowView : NSTableRowView
{
	BOOL haveForcedWidth;
}

@property BOOL drawBackground;
@property (nonatomic) CGFloat forcedWidth;

@end

#import "RakButtonMorphic.h"

#import "RakSRSearchBar.h"
#import "RakSRTagRail.h"
#import "RakSRHeader.h"
#import "RakSRSearchTab.h"

@class RakSRContentManager;

#import "RakGridView.h"

#import "RakSRContentManager.h"

#import "RakSerieMainList.h"
#import "RakSerieList.h"

#import "RakSerieView.h"
