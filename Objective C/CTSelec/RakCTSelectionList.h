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

@interface RakCTSelectionList : RakList
{
	PROJECT_DATA projectData;
}

@property BOOL isTome;
@property (readonly) uint nbElem;

- (id) init : (NSRect) frame : (PROJECT_DATA) project : (bool) isTomeRequest : (long) elemSelected : (long) scrollerPosition;
- (BOOL) reloadData : (PROJECT_DATA) project : (int) nbElem : (void *) newData : (BOOL) resetScroller;

- (void) jumpScrollerToRow : (int) row;

@end

#import "RakCTSelectionListContainer.h"
