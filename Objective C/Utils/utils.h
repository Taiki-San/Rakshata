/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                          **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite   **
 **                                                                                          **
 *********************************************************************************************/

/*Custom NSView to add a couple of shared variable*/
@interface RakTabView : NSView
{
	int flag;
	bool readerMode;
	NSView *blurView;
	NSTrackingArea * trackingArea;
	
@public
	uint resizeAnimationCount;
}

#define CREATE_CUSTOM_VIEW_TAB_SERIE	1
#define CREATE_CUSTOM_VIEW_TAB_CT		2
#define CREATE_CUSTOM_VIEW_TAB_READER	3

- (NSView *) setUpView: (NSView *)superView;
- (void) setUpBlur;

- (void) drawContentView: (NSRect) frame;
- (void) refreshLevelViews : (NSView*) superView;
- (void) refreshViewSize;

- (void) readerIsOpening;
- (void) resizeReaderCatchArea;
- (void) releaseReaderCatchArea;
- (void) setUpViewForAnimation : (BOOL) readerMode;

- (NSRect) generateNSTrackingAreaSize : (NSRect) viewFrame;
- (void) applyRefreshSizeReaderChecks;
- (BOOL) isStillCollapsedReaderTab;
- (BOOL) abortCollapseReaderTab;

- (BOOL) isCursorOnMe;
- (NSPoint) getCursorPosInWindow;
- (BOOL) mouseOutOfWindow;


- (NSRect) createFrame;
- (NSRect) createFrameWithSuperView : (NSView*) superView;

- (int) convertTypeToPrefArg : (bool) getX;
- (CGFloat) getRequestedViewPosX: (CGFloat) widthWindow;
- (CGFloat) getRequestedViewPosY: (CGFloat) heightWindow;
- (CGFloat) getRequestedViewWidth:(CGFloat) widthWindow;
- (CGFloat) getRequestedViewHeight:(CGFloat) heightWindow;

@end

@interface RakTabAnimationResize : NSObject
{
	BOOL readerMode;
	RakTabAnimationResize * _instance;
	NSArray* _views;
}
- (id) init : (RakTabAnimationResize*) instance : (NSArray*)views;
- (void) setUpViews;
- (void) perform;

@end

@interface RakButton: NSButton

- (void) refreshViewSize;

@end