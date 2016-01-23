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

@interface RakCTSelectionListContainer : RakView
{
	RakCTDragableTitle * _title;
	RakCTSelectionList * _content;
	RakText * _placeholder;
	
	BOOL _placeholderActive;
	BOOL _wasHidden;
}

@property (nonatomic) BOOL compactMode;
@property (readonly) uint nbElem;

- (instancetype) initWithFrame : (NSRect) parentFrame : (BOOL) isCompact : (RakCTSelectionList*) content;

- (void) resizeAnimation : (NSRect) parentFrame;

//Proxy
- (NSScrollView*) getContent;

- (uint) getSelectedElement;
- (float) getSliderPos;

- (BOOL) reloadData : (PROJECT_DATA) project : (BOOL) resetScroller;

- (NSInteger) getIndexOfElement : (uint) element;
- (void) selectIndex : (uint) index;
- (void) jumpScrollerToIndex : (uint) index;

- (void) resetSelection;

- (void) setWantIsTome : (BOOL) isTome;

@end
