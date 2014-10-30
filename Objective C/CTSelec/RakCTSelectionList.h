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
	
	BOOL _compactMode;
	BOOL _resizingQueued;
	
	NSTableColumn * _mainColumn;
	NSTableColumn * _detailColumn;
	uint _detailWidth;
	
	
	//Various data
	//Chapter only
	uint _nbChapterPrice;
	uint * chapterPrice;
	
	uint _nbElem;
	uint _nbInstalled;
	uint * _installedJumpTable;		//Give the position in the main array of installed elements
	BOOL * _installedTable;			//Tell if an element of the main array is installed
}

@property BOOL isTome;
@property (readonly) uint nbElem;

@property BOOL compactMode;

- (instancetype) initWithFrame : (NSRect) frame  isCompact : (BOOL) isCompact projectData : (PROJECT_DATA) project isTome : (bool) isTomeRequest selection : (long) elemSelected  scrollerPos : (long) scrollerPosition;
- (BOOL) reloadData : (PROJECT_DATA) project : (BOOL) resetScroller;
- (void) flushContext : (BOOL) animated;

- (void) jumpScrollerToRow : (int) row;

@end

#import "RakCTSelectionListContainer.h"
