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

#import "MainViewController.h"	//Allow to create NSViewController without NIB

/*Custom NSView to add a couple of shared variable*/

@interface RakTabView : NSView
{
	int flag;
	bool readerMode;
	NSTrackingArea * trackingArea;
}

#define CREATE_CUSTOM_VIEW_TAB_SERIE	1
#define CREATE_CUSTOM_VIEW_TAB_CT		2
#define CREATE_CUSTOM_VIEW_TAB_READER	3

- (NSView *) setUpView: (NSView *)superView;
- (void) drawContentView: (NSRect) frame;
- (void) refreshLevelViews : (NSView*) superView;
- (void) refreshViewSize;

- (void) readerIsOpening;
- (void) resizeReaderCatchArea;
- (void) releaseReaderCatchArea;

- (NSRect) generateNSTrackingAreaSize : (NSRect) viewFrame;
- (void) applyRefreshSizeReaderChecks;
- (BOOL) isStillCollapsedReaderTab;

-(BOOL) isCursorOnMe;

- (NSRect) createFrame : (NSView*) superView;
- (int) convertTypeToPrefArg : (bool) getX;
- (CGFloat) getRequestedViewPosX: (CGFloat) widthWindow;
- (CGFloat) getRequestedViewPosY: (CGFloat) heightWindow;
- (CGFloat) getRequestedViewWidth:(CGFloat) widthWindow;
- (CGFloat) getRequestedViewHeight:(CGFloat) heightWindow;

@end

@interface NSRakButton: NSButton

- (void) refreshViewSize;

@end