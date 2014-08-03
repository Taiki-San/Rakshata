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
	NSTrackingArea * trackingArea;
	
	NSRect _lastFrame;

	//Login request
	bool _waitingLogin;
	bool _needPassword;
	RakTabForegroundView * foregroundView;
	
@public
	bool readerMode;
	uint resizeAnimationCount;
}

@property bool waitingLogin;

#define CREATE_CUSTOM_VIEW_TAB_SERIE	1
#define CREATE_CUSTOM_VIEW_TAB_CT		2
#define CREATE_CUSTOM_VIEW_TAB_READER	3

- (id) initView: (NSView *)superView : (NSString *) state;
- (void) endOfInitialization;
- (NSString *) byebye;

+ (BOOL) broadcastUpdateContext : (id) sender : (PROJECT_DATA) project : (BOOL) isTome : (int) element;
- (void) contextChanged : (NSNotification*) notification;
- (void) updateContextNotification : (PROJECT_DATA) project : (BOOL) isTome : (int) element;

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
- (NSRect) createFrameWithSuperView : (NSView*) superView;

- (int) getCodePref : (int) request;
- (CGFloat) getRequestedViewWidth:(CGFloat) widthWindow;

- (NSString *) waitingLoginMessage;

- (id) getMDL : (BOOL) requireAvailable;
- (BOOL) wouldFrameChange : (NSRect) newFrame;

- (void) dragAndDropStarted:(BOOL)started : (BOOL) canDL;
- (BOOL) receiveDrop : (PROJECT_DATA) data : (bool) isTome : (int) element : (uint) sender;
- (BOOL) shouldDeployWhenDragComeIn;
- (NSDragOperation) dropOperationForSender : (uint) sender : (BOOL) canDL;
- (BOOL) acceptDrop : (uint) initialTab : (id<NSDraggingInfo>)sender;

@end
