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
	RakMenuText * headerText;
	RakMDLList * MDLList;
	
	RakText * dropPlaceHolder;
	
	RakMDLController* _controller;
}

- (instancetype) initContent: (NSRect) frame : (NSString *) state : (RakMDLController*) controller;
- (CGFloat) getContentHeight;
- (NSRect) getMainListFrame : (NSRect) output;
- (NSPoint) getPosDropPlaceHolder : (NSSize) frameSize;

- (void) wakeUp;
- (void) hideList : (BOOL) hide;

- (BOOL) proxyReceiveDrop : (PROJECT_DATA) data : (BOOL) isTome : (uint) element : (uint) sender;

@end
