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

@interface RakMDLListView : NSView
{
	BOOL isSecondTextHidden;
	
	RakButton * _pause;
	RakButton * _read;
	RakButton * _remove;
	
	RakProgressCircle * DLprogress;
	
	
	RakText * requestName;
	RakText * statusText;
	
	RakMDLController * _controller;
	uint _row;
	DATA_LOADED ** todoList;
	
	CGFloat iconWidth;
	int8_t previousStatus;
}

- (id) init : (CGFloat) width : (CGFloat) height : (RakButton *) pause : (RakButton *) read : (RakButton *) remove : (id) controller : (uint) rowID;
- (NSString *) getName;
- (void) setFont : (NSFont*) font;
- (void) updateData : (uint) data;
- (void) updateContext;

- (void) updatePercentage : (CGFloat) percentage;
- (void) sendRemove;
- (void) sendPause;
- (void) sendRead;

@end
