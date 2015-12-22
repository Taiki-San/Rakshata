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

#define RCTH_TITLE_ID @"CTHProperty"
#define RCTH_DETAILS_ID @"CTHData"

/*Data are organized as the following:
	- Release date
	- Number of chapters
	- Number of volumes
	- Paid content
	- DRM
 */


@interface RakCTHTableController : NSObject <NSTableViewDataSource, NSTableViewDelegate>
{
	NSTableView * _tableView;
	
	uint cacheID;
	
	//Context data
	uint numberOfRows;
	
	uint numberOfChapters;
	uint numberOfChaptersInstalled;
	uint numberOfVolumes;
	uint numberOfVolumesInstalled;
	
	uint status;
	uint32_t category;
	uint32_t tag;
	
	BOOL paidContent;
	BOOL DRM;
}

@property (strong) RakListScrollView * scrollView;
@property (readonly) CGFloat baseX;

- (instancetype) initWithProject : (PROJECT_DATA) project frame : (NSRect) frame;
- (void) updateProject : (PROJECT_DATA) projectID;

- (void) setFrame : (NSRect) frameRect;
- (void) resizeAnimation : (NSRect) frameRect;
- (CGFloat) rawBaseX : (NSRect) frameRect;	//When animating, heavier to get, reflect expectation

- (void) craftTableView : (NSRect) frame;

@end
