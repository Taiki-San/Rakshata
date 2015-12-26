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

#if TARGET_OS_IPHONE
@interface RakTabBase : UIViewController
#else
@interface RakTabBase : RakView
#endif
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

- (NSString *) byebye;

+ (BOOL) broadcastUpdateContext : (id) sender : (PROJECT_DATA) project : (BOOL) isTome : (uint) element;
+ (void) broadcastUpdateFocus : (uint) newFocus;
- (void) contextChanged : (NSNotification*) notification;
- (void) ownFocus;
- (void) updateContextNotification : (PROJECT_DATA) project : (BOOL) isTome : (uint) element;

- (void) seriesIsOpening : (byte) context;
- (void) CTIsOpening : (byte) context;
- (void) readerIsOpening : (byte) context;
- (void) MDLIsOpening : (byte) context;

- (id) getMDL : (BOOL) requireAvailable;

@end

#if !TARGET_OS_IPHONE
@interface RakTabView : RakTabBase <NSDraggingDestination>
{
	NSTrackingRectTag trackingArea;
	RakTabForegroundView * foregroundView;
}

- (void) initView: (RakView *)superview : (NSString *) state;
- (NSString *) byebye;

- (RakColor*) getMainColor;
- (void) refreshLevelViews : (RakView*) superview : (byte) context;
- (void) refreshLevelViewsAnimation : (RakView*) superview;
- (void) fastAnimatedRefreshLevel : (RakView*) superview;
- (void) resetFrameSize : (BOOL) withAnimation;
- (void) refreshViewSize;
- (void) resize : (NSRect) bounds : (BOOL) animated;
- (void) animationIsOver : (uint) mainThread : (byte) context;

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
- (NSRect) createFrameWithSuperView : (RakView*) superview;

- (uint) getFrameCode;

- (NSString *) waitingLoginMessage;
- (void) setWaitingLoginWrapper : (NSNumber*) objWaitingLogin;
- (RakTabForegroundView *) getForgroundView;

- (BOOL) wouldFrameChange : (NSRect) newFrame;

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
