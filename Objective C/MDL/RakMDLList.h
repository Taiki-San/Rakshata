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

#define MDLLIST_CELL_HEIGHT ((11 + 1) * 2)

@interface RakMDLList : RakList
{
	RakMDLController * controller;
	
	CGFloat cellHeight;
	
	RakButton * pause;
	RakButton * read;
	RakButton * remove;
	
	bool dragInProgress;
	uint draggedElement;
}

- (id) init : (NSRect) frame : (RakMDLController *) _controller;
- (void) wakeUp;
- (void) setScrollerHidden : (BOOL) hidden;

- (CGFloat) contentHeight;
- (void) resizeAnimation: (NSRect) frameRect;

@end
