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

@interface RakCTCoreContentView : NSObject <NSTableViewDelegate, NSTableViewDataSource>
{
	bool isTome;
	void* data;
	uint amountData;
	NSScrollView * scrollView;
	NSTableView * _tableView;
}

- (id) init : (NSRect) frame : (MANGAS_DATA) project : (bool) isTomeRequest;
- (void) setSuperView : (NSView *) superview;
- (NSRect) getTableViewFrame : (NSRect) superViewFrame;

@end

@interface RakCTScrollView : NSScrollView

@end