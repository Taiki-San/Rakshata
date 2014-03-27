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

#define STATE_EMPTY @"Luna is bored"

@interface RakTabView : NSView
{
	int flag;
	NSTrackingArea * trackingArea;
	
@public
	bool readerMode;
	uint resizeAnimationCount;
}

#define CREATE_CUSTOM_VIEW_TAB_SERIE	1
#define CREATE_CUSTOM_VIEW_TAB_CT		2
#define CREATE_CUSTOM_VIEW_TAB_READER	3

- (id) initView: (NSView *)superView : (NSString *) state;
- (NSString *) byebye;

- (void) drawContentView: (NSRect) frame;
- (void) refreshLevelViews : (NSView*) superView;
- (void) refreshViewSize;

- (void) readerIsOpening;
- (void) resizeReaderCatchArea;
- (void) releaseReaderCatchArea;
- (void) setUpViewForAnimation : (BOOL) newReaderMode;

- (NSRect) generateNSTrackingAreaSize : (NSRect) viewFrame;
- (void) applyRefreshSizeReaderChecks;
- (BOOL) isStillCollapsedReaderTab;
- (BOOL) abortCollapseReaderTab;

- (BOOL) isCursorOnMe;
- (NSPoint) getCursorPosInWindow;
- (BOOL) mouseOutOfWindow;


- (NSRect) createFrame;
- (NSRect) createFrameWithSuperView : (NSView*) superView;
- (NSRect) getCurrentFrame;


- (int) getCodePref : (int) request;
- (CGFloat) getRequestedViewPosX: (CGFloat) widthWindow;
- (CGFloat) getRequestedViewPosY: (CGFloat) heightWindow;
- (CGFloat) getRequestedViewWidth:(CGFloat) widthWindow;
- (CGFloat) getRequestedViewHeight:(CGFloat) heightWindow;

@end

@interface RakTabAnimationResize : NSObject
{
	BOOL readerMode;
	NSArray* _views;
}
- (id) init : (NSArray*)views;
- (void) setUpViews;
- (void) performTo;
- (void) performFromTo : (NSArray*) basePosition;

@end
