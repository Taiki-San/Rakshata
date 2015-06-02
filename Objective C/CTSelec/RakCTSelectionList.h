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
	
	BOOL _rowNumberUpdateQueued;
	BOOL _resizingQueued;
	BOOL _UIOnlySelection;
	BOOL _selectionWithoutUI;
	
	NSArray * _mainColumns;
	NSArray * _detailColumns;
	uint _detailWidth;
	
	uint _indexSelectedBeforeUpdate;
	
	//Various data
	//Chapter only
	uint _nbChapterPrice;
	uint * chapterPrice;
	
	uint _nbElem;
	uint _numberOfRows;
	uint _nbInstalled;
	uint * _installedJumpTable;		//Give the position in the main array of installed elements
	BOOL * _installedTable;			//Tell if an element of the main array is installed
}

@property BOOL isTome;
@property (readonly) uint cacheID;
@property (readonly) BOOL isEmpty;

@property (readonly) uint nbElem;

@property (nonatomic) BOOL compactMode;

- (instancetype) initWithFrame : (NSRect) frame  isCompact : (BOOL) isCompact projectData : (PROJECT_DATA) project isTome : (BOOL) isTomeRequest selection : (long) elemSelected  scrollerPos : (long) scrollerPosition;
- (BOOL) reloadData : (PROJECT_DATA) project : (BOOL) resetScroller;
- (void) flushContext : (BOOL) animated;

- (void) postProcessColumnUpdate;

- (void) jumpScrollerToIndex : (uint) index;

@end

#import "RakCTSelectionListContainer.h"
