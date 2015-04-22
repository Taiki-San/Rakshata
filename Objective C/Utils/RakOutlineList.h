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

#import "RakOutlineListItem.h"

@interface RakTreeView : NSOutlineView
{
	NSRect _defaultFrame;
}

- (void) setDefaultFrame : (NSRect) frame;

@property BOOL wantUpdateScrollview;

@end

@interface RakOutlineList : RakDragResponder <NSOutlineViewDataSource, NSOutlineViewDelegate, NSDraggingDestination>
{
	uint _nbRoot;

	RakTreeView * content;
	NSTableColumn * firstColumn;

	RakOutlineListItem * currentDraggedItem;
	
	uint nbColumn;
}

@property (getter=isHidden, setter=setHidden:)				BOOL hidden;

- (BOOL) initializeMain : (NSRect) frame;
- (RakTreeView *) getContent;

- (void) moreFlushing;

- (void) setColumnWidth : (NSTableColumn *) _column : (uint) index : (CGFloat) fullWidth;

- (void) setFrame: (NSRect) frame;
- (void) setFrameOrigin : (NSPoint) newOrigin;
- (void) resizeAnimation : (NSRect) frame;
- (void) additionalResizing : (NSRect) frame : (BOOL) animated;

- (NSColor *) getFontTopColor;
- (NSColor *) getFontClickableColor;

- (uint) getNbRoot;

@end
