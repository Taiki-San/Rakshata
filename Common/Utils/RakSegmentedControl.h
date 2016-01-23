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

@interface RakMinimalSegmentedControl : NSSegmentedControl

- (NSRect) getButtonFrame : (NSRect) superviewFrame;
- (void) resizeAnimation : (NSRect) frameRect;

@end

@interface RakSegmentedControl : RakMinimalSegmentedControl
{
	id animationController;
}

@property (unsafe_unretained) id postAnimationTarget;
@property SEL postAnimationAction;

- (instancetype) initWithFrame : (NSRect) frame : (NSArray *) buttonMessage;

- (void) updateSelectionWithoutAnimation : (NSInteger) newState;
- (BOOL) setupTransitionAnimation : (NSNumber*) oldValue : (NSNumber *) newValue;

@end
