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

@interface RakMDLListView : NSView
{
	BOOL isSecondTextHidden;
	
	RakButton * _pause;
	RakButton * _read;
	RakButton * _remove;
	
	RakProgressBar * DLprogress;
	
	RakText * requestName;
	RakText * statusText;
	
	RakMDLController * _controller;
	uint _row;
	DATA_LOADED ** todoList;
	
	CGFloat iconWidth;
	int8_t previousStatus;
}

- (id) init : (CGFloat) width : (CGFloat) height : (id) controller : (uint) rowID;
- (void) initIcons;
- (NSString *) getName;
- (void) setFont : (NSFont*) font;
- (void) updateData : (uint) data : (uint) newCellWidth;
- (void) updateContext;
- (void) requestReloadData : (NSTableView *) tableView;

- (void) updatePercentage : (CGFloat) percentage : (size_t) speed;
- (void) sendRemove;
- (void) sendPause;
- (void) sendRead;
- (void) installOver;

@end
