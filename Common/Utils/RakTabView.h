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

#define ANIMATION_DURATION_SHORT 0.1f
#define ANIMATION_DURATION_LONG 0.2f

enum
{
    REFRESHVIEWS_CHANGE_MT,
    REFRESHVIEWS_CHANGE_READER_TAB,
	REFRESHVIEWS_NO_CHANGE
} REFRESHVIEWS_CODE;

@interface RakTabBase : RakView
{
	BOOL noDrag;
	BOOL canDeploy;
	uint flag;
	
	NSRect _lastFrame;

	//Login request
	BOOL _waitingLogin;
	BOOL _needPassword;
}

@property uint mainThread;
@property (readonly) BOOL waitingLogin;
@property BOOL initWithNoContent;

@property BOOL forceNextFrameUpdate;

- (void) initView: (RakView *)superview : (NSString *) state;
- (NSString *) byebye;

+ (BOOL) broadcastUpdateContext : (id) sender : (PROJECT_DATA) project : (BOOL) isTome : (uint) element;
+ (void) broadcastUpdateFocus : (uint) newFocus;
- (void) contextChanged : (NSNotification*) notification;
- (void) ownFocus;
- (void) updateContextNotification : (PROJECT_DATA) project : (BOOL) isTome : (uint) element;

- (RakColor*) getMainColor;
- (void) refreshLevelViews : (RakView*) superview : (byte) context;
- (void) refreshLevelViewsAnimation : (RakView*) superview;
- (void) fastAnimatedRefreshLevel : (RakView*) superview;
- (void) resetFrameSize : (BOOL) withAnimation;
- (void) refreshViewSize;
- (void) _refreshViewSize;
- (void) resize : (NSRect) bounds : (BOOL) animated;
- (void) animationIsOver : (uint) mainThread : (byte) context;

- (void) seriesIsOpening : (byte) context;
- (void) CTIsOpening : (byte) context;
- (void) readerIsOpening : (byte) context;
- (void) MDLIsOpening : (byte) context;

- (void) setUpViewForAnimation : (uint) mainThread;

- (void) refreshDataAfterAnimation;
- (BOOL) isStillCollapsedReaderTab;
- (BOOL) abortCollapseReaderTab;

- (NSRect) getFrameOfNextTab;
- (void) rejectedMouseEntered;
- (void) rejectedMouseExited;

- (void) resizeAnimation;
- (void) resizingCanceled;
- (NSRect) createFrame;
- (BOOL) needToConsiderMDL;
- (void) setLastFrame : (NSRect) frame;
- (NSRect) lastFrame;
- (NSRect) createFrameWithSuperView : (RakView*) superview;

- (uint) getFrameCode;

- (NSString *) waitingLoginMessage;
- (void) setWaitingLoginWrapper : (NSNumber*) objWaitingLogin;

- (id) getMDL : (BOOL) requireAvailable;
- (BOOL) wouldFrameChange : (NSRect) newFrame;

@end

#if !TARGET_OS_IPHONE
@interface RakTabView : RakTabBase <NSDraggingDestination>
{
	NSTrackingRectTag trackingArea;
	RakTabForegroundView * foregroundView;
}

- (BOOL) isCursorOnMe;
- (BOOL) isCursorOnRect : (NSRect) frame;
- (void) objectWillLooseFocus : (id) object;

- (NSPoint) getCursorPosInWindow;
- (BOOL) mouseOutOfWindow;

- (void) resizeTrackingArea;
- (void) releaseTrackingArea;
- (NSRect) generatedReaderTrackingFrame;

- (RakTabForegroundView *) getForgroundView;

- (void) dragAndDropStarted:(BOOL)started : (BOOL) canDL;
- (BOOL) receiveDrop : (PROJECT_DATA) data : (BOOL) isTome : (uint) element : (uint) sender;
- (BOOL) shouldDeployWhenDragComeIn;
- (NSDragOperation) dropOperationForSender : (uint) sender : (BOOL) canDL;
- (BOOL) acceptDrop : (uint) initialTab : (id<NSDraggingInfo>)sender;

@end

#else

@interface RakTabView : RakTabBase

@end

#endif
