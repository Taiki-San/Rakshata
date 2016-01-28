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
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 ********************************************************************************************/

@class RakPopoverView;

@interface RakPopoverWrapper : NSObject
{
	INPopoverController * _popover;
}

@property (weak) RakView * anchor;
@property NSRect anchorFrame;
@property INPopoverArrowDirection direction;

- (instancetype) init : (RakView *) contentView;
- (void) togglePopover : (NSRect) frame;

- (void) additionalConfiguration : (id) target;
- (void) justCallSuperConfigure : (RakPopoverView *) view;

- (void) updatePosition : (NSPoint) origin : (BOOL) animated;
- (void) setDelegate:(id <INPopoverControllerDelegate>) delegate;
- (void) closePopover;

- (void) clearMemory;

@end

@interface RakPopoverView : RakView <INPopoverControllerDelegate>
{
	BOOL prefsRegistered;
	BOOL additionalConfigRequired;
	
	RakView * __weak _anchor;

	RakPopoverWrapper * popover;
}

- (void) internalInit : (id) anchor : (NSRect) baseFrame : (BOOL) wantAdditionalConfig;

- (void) setupView;
- (void) configurePopover : (INPopoverController*) internalPopover;
- (void) directConfiguration : (INPopoverController*) internalPopover;

- (RakColor *) popoverBorderColor;
- (RakColor *) popoverArrowColor;
- (RakColor *) borderColor;
- (RakColor *) backgroundColor;

- (void) updateOrigin : (NSPoint) origin : (BOOL) animated;
- (void) additionalUpdateOnThemeChange;
- (void) closePopover;

@end
