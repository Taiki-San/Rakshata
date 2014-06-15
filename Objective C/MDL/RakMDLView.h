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

@interface RakMDLView : RakTabContentTemplate
{
	RakMDLHeaderText * headerText;
	RakMDLList * MDLList;
}

- (id)initContent: (NSRect) frame : (NSString *) state : (RakMDLController*) controller;
- (CGFloat) getContentHeight;
- (NSRect) getMainListFrame : (NSRect) output;

- (void) updateScroller : (BOOL) hidden;
- (void) wakeUp;

- (void) resizeAnimation : (NSRect) frame;

- (void) needToQuit;

- (BOOL) proxyReceiveDrop : (MANGAS_DATA) data : (bool) isTome : (int) element : (uint) sender;

@end
