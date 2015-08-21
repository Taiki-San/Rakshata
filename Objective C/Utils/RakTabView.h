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

#define ANIMATION_DURATION_SHORT 0.1f
#define ANIMATION_DURATION_LONG 0.2f

enum {
    REFRESHVIEWS_CHANGE_MT,
    REFRESHVIEWS_CHANGE_READER_TAB,
	REFRESHVIEWS_NO_CHANGE
} REFRESHVIEWS_CODE;

@interface RakTabView : NSView <NSDraggingDestination>
{
	BOOL noDrag;
	BOOL canDeploy;
	uint flag;
	NSTrackingRectTag trackingArea;
	
	NSRect _lastFrame;

	//Login request
	BOOL _waitingLogin;
	BOOL _needPassword;
	RakTabForegroundView * foregroundView;
}

@property uint mainThread;
@property BOOL waitingLogin;
@property BOOL initWithNoContent;

@property BOOL forceNextFrameUpdate;

- (void) initView: (NSView *)superview : (NSString *) state;
- (NSString *) byebye;

+ (BOOL) broadcastUpdateContext : (id) sender : (PROJECT_DATA) project : (BOOL) isTome : (uint) element;
+ (void) broadcastUpdateFocus : (uint) newFocus;
- (void) contextChanged : (NSNotification*) notification;
- (void) ownFocus;
- (void) updateContextNotification : (PROJECT_DATA) project : (BOOL) isTome : (uint) element;

- (NSColor*) getMainColor;
- (void) drawContentView: (NSRect) frame;
- (void) refreshLevelViews : (NSView*) superview : (byte) context;
- (void) refreshLevelViewsAnimation : (NSView*) superview;
- (void) fastAnimatedRefreshLevel : (NSView*) superview;
- (void) resetFrameSize : (BOOL) withAnimation;
- (void) refreshViewSize;
- (void) resize : (NSRect) bounds : (BOOL) animated;
- (void) animationIsOver : (uint) mainThread : (byte) context;

- (void) seriesIsOpening : (byte) context;
- (void) CTIsOpening : (byte) context;
- (void) readerIsOpening : (byte) context;
- (void) MDLIsOpening : (byte) context;

- (void) resizeTrackingArea;
- (void) releaseTrackingArea;
- (void) setUpViewForAnimation : (uint) mainThread;

- (NSRect) generatedReaderTrackingFrame;
- (void) refreshDataAfterAnimation;
- (BOOL) isStillCollapsedReaderTab;
- (BOOL) abortCollapseReaderTab;

- (BOOL) isCursorOnMe;
- (BOOL) isCursorOnRect : (NSRect) frame;
- (NSPoint) getCursorPosInWindow;
- (NSRect) getFrameOfNextTab;
- (BOOL) mouseOutOfWindow;
- (void) objectWillLooseFocus : (id) object;
- (void) rejectedMouseEntered;
- (void) rejectedMouseExited;

- (void) resizeAnimation;
- (void) resizingCanceled;
- (NSRect) createFrame;
- (BOOL) needToConsiderMDL;
- (void) setLastFrame : (NSRect) frame;
- (NSRect) lastFrame;
- (NSRect) createFrameWithSuperView : (NSView*) superview;

- (uint) getFrameCode;

- (NSString *) waitingLoginMessage;
- (void) setWaitingLoginWrapper : (NSNumber*) objWaitingLogin;
- (RakTabForegroundView *) getForgroundView;

- (id) getMDL : (BOOL) requireAvailable;
- (BOOL) wouldFrameChange : (NSRect) newFrame;

- (void) dragAndDropStarted:(BOOL)started : (BOOL) canDL;
- (BOOL) receiveDrop : (PROJECT_DATA) data : (BOOL) isTome : (uint) element : (uint) sender;
- (BOOL) shouldDeployWhenDragComeIn;
- (NSDragOperation) dropOperationForSender : (uint) sender : (BOOL) canDL;
- (BOOL) acceptDrop : (uint) initialTab : (id<NSDraggingInfo>)sender;

@end
