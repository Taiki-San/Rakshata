/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at https://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

@class Series;

#define SR_HEADER_ROW_HEIGHT 25
#define SR_SEARCH_FIELD_HEIGHT 22

#define SR_HEADER_HEIGHT_SINGLE_ROW (RBB_TOP_BORDURE + SR_HEADER_ROW_HEIGHT + RBB_TOP_BORDURE)

@interface RakSRHeader : RakView
{
	RakButton *preferenceButton;
#ifdef SEVERAL_VIEWS
	RakButtonMorphic * displayType;
#endif
	RakButton * storeSwitch;

	RakSRTagRail * tagRail;
	
	RakSRSearchBar * search;
	
	RakBackButton * backButton;
	
	PrefsUI * winController;
	
	BOOL _haveFocus;
	BOOL _noAnimation;
	
	CGFloat _separatorX;
}

@property CGFloat height;
@property BOOL prefUIOpen;

- (instancetype) initWithFrame : (NSRect) frameRect : (BOOL) haveFocus;

- (void) resizeAnimation : (NSRect) frameRect;
- (NSRect) frameFromParent : (NSRect) parentFrame;

- (void) updateFocus : (uint) mainThread;
- (void) cleanupAfterFocusChange;

- (void) nbRowRailsChanged;

- (void) focusSearchField;

@end


enum
{
	SR_CELLTYPE_GRID = 0,
	SR_CELLTYPE_REPO = 1,
	SR_CELLTYPE_LIST = 2,
	SR_CELLTYPE_MAX = SR_CELLTYPE_LIST
};