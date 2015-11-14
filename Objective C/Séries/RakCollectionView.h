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
 ********************************************************************************************/

@class RakCollectionViewItem;

@interface RakCollectionView : NSCollectionView
{
	uint activeProject;

	uint nbColumn;
	BOOL _resized;
	
	NSClipView * _clipView;
	RakCollectionViewItem * selectedItem;
}

@property RakSRContentManager * manager;
@property (strong) RakCollectionViewItem * clickedView;
@property BOOL draggedSomething;

- (instancetype) initWithFrame : (NSRect) frameRect : (RakSRContentManager *) manager;
- (void) bindManager;
- (void) resizeAnimation : (NSRect) frameRect;

- (BOOL) isValidIndex : (uint) index;
- (uint) cacheIDForIndex : (uint) index;

- (BOOL) needUpdateNumberColumn;
- (uint) updateNumberColumn;

@end
