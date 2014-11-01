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

@interface RakCTSelection : NSView
{
	PROJECT_DATA data;
	RakCTCoreViewButtons * _buttons;
	RakCTSelectionListContainer * _chapterView;
	RakCTSelectionListContainer * _volView;
	
	uint _currentContext;
	uint _cachedHeaderHeight;
	
	BOOL _preventContextUpdate;
}

@property uint currentContext;

@property BOOL dontNotify;
@property (readonly) PROJECT_DATA currentProject;


- (instancetype) initWithProject : (PROJECT_DATA) project : (BOOL) isTome : (NSRect) parentBounds : (CGFloat) headerHeight : (long [4]) context;

- (void) setFrame : (NSRect) parentFrame : (CGFloat) headerHeight;
- (void) resizeAnimation : (NSRect) parentFrame : (CGFloat) headerHeight;

- (NSString *) getContextToGTFO;

- (void) gotClickedTransmitData : (NSNotification*) notification;

- (void) feedAnimationController : (RakCTAnimationController *) animationController;
- (void) switchIsTome : (RakCTCoreViewButtons*) sender;

- (void) selectElem : (uint) projectID : (BOOL) isTome : (int) element;
- (BOOL) updateContext : (PROJECT_DATA) newData;

@end

