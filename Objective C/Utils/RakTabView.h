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

#import "RakTabViewTools.h"

#define STATE_EMPTY @"Luna is bored"

#define NOTIFICATION_UPDATE_TAB_FOCUS	@"RakTabUpdateFocusNotification"
#define NOTIFICATION_UPDATE_TAB_CONTENT	@"RakTabUpdateContentNotification"

enum {
    REFRESHVIEWS_CHANGE_MT,
    REFRESHVIEWS_CHANGE_READER_TAB,
	REFRESHVIEWS_NO_CHANGE
} REFRESHVIEWS_CODE;

@interface RakTabView : NSView <NSDraggingDestination>
{
	bool noDrag;
	bool canDeploy;
	int flag;
	NSTrackingRectTag trackingArea;
	
	NSRect _lastFrame;

	//Login request
	bool _waitingLogin;
	bool _needPassword;
	RakTabForegroundView * foregroundView;
}

@property uint mainThread;
@property bool waitingLogin;
@property bool initWithNoContent;

@property BOOL forceNextFrameUpdate;

#define CREATE_CUSTOM_VIEW_TAB_SERIE	1
#define CREATE_CUSTOM_VIEW_TAB_CT		2
#define CREATE_CUSTOM_VIEW_TAB_READER	3

- (id) initView: (NSView *)superview : (NSString *) state;
- (NSString *) byebye;

+ (BOOL) broadcastUpdateContext : (id) sender : (PROJECT_DATA) project : (BOOL) isTome : (int) element;
+ (void) broadcastUpdateFocus : (uint) newFocus;
- (void) contextChanged : (NSNotification*) notification;
- (void) ownFocus;
- (void) updateContextNotification : (PROJECT_DATA) project : (BOOL) isTome : (int) element;

- (NSColor*) getMainColor;
- (void) drawContentView: (NSRect) frame;
- (void) refreshLevelViews : (NSView*) superview : (byte) context;
- (void) refreshLevelViewsAnimation : (NSView*) superview;
- (void) fastAnimatedRefreshLevel : (NSView*) superview;
- (void) resetFrameSize : (BOOL) withAnimation;
- (void) refreshViewSize;
- (void) animationIsOver : (uint) mainThread : (byte) context;

- (void) seriesIsOpening : (byte) context;
- (void) CTIsOpening : (byte) context;
- (void) readerIsOpening : (byte) context;
- (void) MDLIsOpening : (byte) context;

- (void) resizeReaderCatchArea;
- (void) releaseReaderCatchArea;
- (void) setUpViewForAnimation : (uint) mainThread;

- (NSRect) generateNSTrackingAreaSize;
- (void) refreshDataAfterAnimation;
- (BOOL) isStillCollapsedReaderTab;
- (BOOL) abortCollapseReaderTab;

- (BOOL) isCursorOnMe;
- (BOOL) isCursorOnRect : (NSRect) frame;
- (NSPoint) getCursorPosInWindow;
- (NSRect) getFrameOfNextTab;
- (BOOL) mouseOutOfWindow;
- (void) objectWillLooseFocus : (id) object;

- (void) resizeAnimation;
- (NSRect) createFrame;
- (BOOL) needToConsiderMDL;
- (void) setLastFrame : (NSRect) frame;
- (NSRect) lastFrame;
- (NSRect) createFrameWithSuperView : (NSView*) superview;

- (int) getCodePref : (int) request;
- (CGFloat) getRequestedViewWidth:(CGFloat) widthWindow;

- (NSString *) waitingLoginMessage;
- (void) setWaitingLoginWrapper : (NSNumber*) objWaitingLogin;
- (RakTabForegroundView *) getForgroundView;

- (id) getMDL : (BOOL) requireAvailable;
- (BOOL) wouldFrameChange : (NSRect) newFrame;

- (void) dragAndDropStarted:(BOOL)started : (BOOL) canDL;
- (BOOL) receiveDrop : (PROJECT_DATA) data : (bool) isTome : (int) element : (uint) sender;
- (BOOL) shouldDeployWhenDragComeIn;
- (NSDragOperation) dropOperationForSender : (uint) sender : (BOOL) canDL;
- (BOOL) acceptDrop : (uint) initialTab : (id<NSDraggingInfo>)sender;

@end
