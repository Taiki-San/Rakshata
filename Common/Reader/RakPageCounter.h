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
 ********************************************************************************************/

@interface RakPageCounterPopover : RakPopoverView
{
	IBOutlet NSTextField *mainLabel;
	IBOutlet NSTextField *textField;
	IBOutlet RakView * gotoButtonContainer;

	uint _maxPage;
}

- (void) launchPopover : (RakView *) anchor : (uint) curPage : (uint) maxPage;

- (void) locationUpdated : (NSRect) frame : (BOOL) animated;

@end

@interface RakPageCounter : RakText
{
	Reader * __weak _target;
	
	uint currentPage;
	uint pageMax;
	
	BOOL popoverStillAround;
	
	IBOutlet RakPageCounterPopover * popover;
}

- (instancetype) init: (RakView*) superview : (CGFloat) posX : (uint) currentPageArg : (uint) pageMaxArg : (Reader *) target;
- (void) updateContext;
- (void) updatePopoverFrame : (NSRect) newFrame : (BOOL) animated;
- (void) stopUsePopover;
- (void) removePopover;

- (void) updateSize : (CGFloat) heightSuperView : (CGFloat) posX;

- (RakColor *) getColorBackground;
- (RakColor *) getFontColor;

- (void) updatePage : (uint) newCurrentPage : (uint) newPageMax;
- (void) transmitPageJump : (uint) newPage;

- (BOOL) openPopover;
- (void) closePopover;

@end
