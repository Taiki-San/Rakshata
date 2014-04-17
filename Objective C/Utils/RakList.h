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

@interface RakListScrollView : NSScrollView

@end

@interface RakList : NSObject <NSTableViewDelegate, NSTableViewDataSource>
{
	void* data;
	uint amountData;
	RakListScrollView * scrollView;
	NSTableView * _tableView;
	
	//Color cache
	NSColor * normal;
	NSColor * highlight;
}

- (void) applyContext : (NSRect) frame : (int) activeRow : (long) scrollerPosition;
- (bool) didInitWentWell;
- (void) failure;

- (void) setSuperView : (NSView *) superview;
- (void) setHidden : (bool) state;
- (void) setFrame : (NSRect) frameRect;

- (NSRect) getTableViewFrame : (NSRect) superViewFrame;

- (NSInteger) getSelectedElement;
- (float) getSliderPos;

- (NSColor *) getTextColor;
- (NSColor *) getTextHighlightColor;
- (NSColor *) getBackgroundHighlightColor;

@end