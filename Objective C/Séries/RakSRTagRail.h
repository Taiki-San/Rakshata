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

#import "RakSRTagItem.h"

@interface RakSRTagRail : NSView
{
	NSMutableArray * tagList;
	NSMutableArray * tagNames;
	
	uint _currentX, _currentRow;
	CGFloat _reducedWidth, _baseSearchBar;
	BOOL _noReorder;
}

@property uint nbRow;

- (instancetype) initWithFrame : (NSRect) frameRect : (CGFloat) baseSearchBar;

- (void) performOperation : (NSString *) object : (uint64_t) ID : (byte) dataType : (BOOL) insertion;

- (void) resizeAnimation : (NSRect) frameRect;
- (void) setBaseSearchBar : (CGFloat) baseSearchBar;

@end

enum
{
	TAG_RAIL_INTER_RAIL_BORDER = 5,
	TAG_RAIL_INTER_ITEM_BORDER = 7
};

