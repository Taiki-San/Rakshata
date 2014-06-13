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

#define STATE_EMPTY @"Luna is bored"

enum {
    REFRESHVIEWS_CHANGE_MT,
    REFRESHVIEWS_CHANGE_READER_TAB,
	REFRESHVIEWS_NO_CHANGE
} REFRESHVIEWS_CODE;

@interface RakTabView : NSView <NSDraggingDestination>
{
	bool noDrag;
	int flag;
	NSTrackingArea * trackingArea;
	
	NSRect _lastFrame;
	
@public
	bool readerMode;
	uint resizeAnimationCount;
}

#define CREATE_CUSTOM_VIEW_TAB_SERIE	1
#define CREATE_CUSTOM_VIEW_TAB_CT		2
#define CREATE_CUSTOM_VIEW_TAB_READER	3

- (id) initView: (NSView *)superView : (NSString *) state;
- (void) endOfInitialization;
- (NSString *) byebye;
- (void) noContent;

+ (BOOL) broadcastUpdateContext : (id) sender : (MANGAS_DATA) project : (BOOL) isTome : (int) element;
- (void) contextChanged : (NSNotification*) notification;
- (void) updateContextNotification : (MANGAS_DATA) project : (BOOL) isTome : (int) element;

- (NSColor*) getMainColor;
- (void) drawContentView: (NSRect) frame;
- (void) refreshLevelViews : (NSView*) superView : (byte) context;
- (void) refreshLevelViewsAnimation : (NSView*) superView;
- (void) refreshViewSize;
- (void) animationIsOver : (uint) mainThread : (byte) context;

- (void) seriesIsOpening : (byte) context;
- (void) CTIsOpening : (byte) context;
- (void) readerIsOpening : (byte) context;
- (void) MDLIsOpening : (byte) context;

- (void) resizeReaderCatchArea : (bool) inReaderMode;
- (void) releaseReaderCatchArea;
- (void) setUpViewForAnimation : (BOOL) newReaderMode;

- (NSRect) generateNSTrackingAreaSize : (NSRect) viewFrame;
- (void) refreshDataAfterAnimation;
- (BOOL) isStillCollapsedReaderTab;
- (BOOL) abortCollapseReaderTab;

- (BOOL) isCursorOnMe;
- (BOOL) isCursorOnRect : (NSRect) frame;
- (NSPoint) getCursorPosInWindow;
- (NSRect) getFrameOfNextTab;
- (BOOL) mouseOutOfWindow;

- (void) resizeAnimation;
- (NSRect) createFrame;
- (BOOL) needToConsiderMDL;
- (void) setLastFrame : (NSRect) frame;
- (NSRect) lastFrame;
- (void) updateLastFrame : (NSRect) newFrame;
- (NSRect) createFrameWithSuperView : (NSView*) superView;


- (int) getCodePref : (int) request;
- (CGFloat) getRequestedViewWidth:(CGFloat) widthWindow;

- (id) getMDL : (BOOL) requireAvailable;
- (BOOL) wouldFrameChange : (NSRect) newFrame;

- (void) receiveDrop : (MANGAS_DATA) data : (bool) isTome : (int) element;
- (BOOL) shouldDeployWhenDragComeIn;

@end

@interface RakTabAnimationResize : NSObject
{
	BOOL readerMode;
	BOOL haveBasePos;
	NSArray* _views;
}
- (id) init : (NSArray*)views;
- (void) setUpViews;
- (void) performTo;
- (void) performFromTo : (NSArray*) basePosition;
- (void) resizeView : (RakTabView *) view : (id) basePos;

@end
