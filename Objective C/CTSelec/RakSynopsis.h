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

#define SYNOPSIS_TOP_BORDER_WIDTH 	5
#define SYNOPSIS_SPACING			5
#define SYNOPSIS_BORDER 			20
#define SYNOPSIS_MAIN_TEXT_BORDER 	10

@interface RakSynopsis : NSView
{
	RakListScrollView * _scrollview;
	RakText * _synopsis;
	RakText * _placeholder;
	
	BOOL placeholderString;
	BOOL _haveScroller;
}

@property (readonly) CGFloat titleHeight;
@property (nonatomic) BOOL haveBackground;

- (instancetype) initWithSynopsis : (charType *) synopsis : (NSRect) frame : (BOOL) haveScroller;
- (void) updateSynopsis : (charType *) newSynopsis;

- (BOOL) setStringToSynopsis : (charType *) synopsis;

- (BOOL) postProcessScrollView;
- (BOOL) generatedScrollView : (NSRect) frame;
- (void) updateScrollViewState;

- (NSRect) frameFromParent : (NSRect) parentFrame;
- (NSPoint) placeholderOrigin : (NSRect) scrollviewBounds;
- (NSRect) frameForContent : (NSRect) mainBounds;

- (void) resizeAnimation : (NSRect) frameRect;
- (void) _updateFrame : (NSRect) mainFrame : (BOOL) animated;

@end
