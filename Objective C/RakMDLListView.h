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
	RakButton * _pause;
	RakButton * _read;
	RakButton * _remove;
	
	CGFloat iconWidth;
	
	RakText * requestName;
	RakText * statusText;
}

- (id) init : (CGFloat) width : (CGFloat) height : (RakButton *) pause : (RakButton *) read : (RakButton *) remove : (id) controller;
- (void) setFont : (NSFont*) font;
- (void) updateData : (id) data;

- (void) sendRemove;
- (void) sendPause;
- (void) sendRead;

@end
