/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

#import "RakOutlineListItem.h"

@interface RakTreeView : NSOutlineView
{
	NSRect _defaultFrame;
	BOOL tryingToProxy;
}

- (void) setDefaultFrame : (NSRect) frame;

@property BOOL wantUpdateScrollview;
@property BOOL manualEventDispatching;

@end

@interface RakOutlineList : RakDragResponder <NSOutlineViewDataSource, NSOutlineViewDelegate, NSDraggingDestination, NSMenuDelegate>
{
	uint _nbRoot;

	RakTreeView * content;
	NSTableColumn * firstColumn;

	RakOutlineListItem * currentDraggedItem;
	RakProjectMenu * menuHandler;
	
	uint nbColumn;
}

@property (getter=isHidden, setter=setHidden:)				BOOL hidden;

- (void) initializeMain : (NSRect) frame;
- (RakTreeView *) getContent;

- (void) moreFlushing;

- (void) activateMenu;
- (void) configureMenu : (NSMenu *) menu forItem : (id) item atColumn : (NSInteger) column;

- (void) setColumnWidth : (NSTableColumn *) _column : (uint) index : (CGFloat) fullWidth;

- (void) setFrame: (NSRect) frame;
- (void) setFrameOrigin : (NSPoint) newOrigin;
- (void) resizeAnimation : (NSRect) frame;
- (void) additionalResizing : (NSRect) frame : (BOOL) animated;

- (RakColor *) getFontTopColor;
- (RakColor *) getFontClickableColor;

- (uint) getNbRoot;

@end
