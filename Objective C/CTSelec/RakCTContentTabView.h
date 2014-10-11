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

@interface RakCTContentTabView : NSView
{
	PROJECT_DATA data;
	RakCTCoreViewButtons * buttons;
	RakCTCoreContentView * tableViewControllerChapter;
	RakCTCoreContentView * tableViewControllerVolume;
	
	uint _currentContext;
}

@property uint currentContext;

@property BOOL dontNotify;
@property (readonly) PROJECT_DATA currentProject;

- (id) initWithProject : (PROJECT_DATA) project : (bool) isTome : (NSRect) frame : (long [4]) context;

- (void) resizeAnimation : (NSRect) frameRect;
- (NSString *) getContextToGTFO;

- (void) gotClickedTransmitData : (bool) isTome : (uint) index;

- (void) feedAnimationController : (RakCTAnimationController *) animationController;
- (void) switchIsTome : (RakCTCoreViewButtons*) sender;

- (void) refreshCTData : (BOOL) checkIfRequired : (uint) ID;
- (void) selectElem : (uint) projectID : (BOOL) isTome : (int) element;
- (BOOL) updateContext : (PROJECT_DATA) newData;

@end

