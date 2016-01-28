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
 *********************************************************************************************/

@interface RakBackButton : NSButton <NSAnimationDelegate>
{
	byte ID;
	BOOL cursorOnMe;
	NSTrackingRectTag tag;

	NSAnimation * _animation;
}

- (instancetype) initWithFrame : (NSRect) frame : (BOOL) isOneLevelBack;

- (void) resizeAnimation : (NSRect) frameRect;

- (RakColor *) getColorBackground;
- (RakColor *) getColorBackgroundSlider;

@end

@interface RakBackButtonAnimation : NSAnimation

@end

@interface RakBackButtonCell : RakButtonCell
{
	BOOL animationInProgress;
	CGFloat animationStatus;
}

- (void) switchToNewContext : (NSString*) imageName : (short) state;

- (void) setAnimationInProgress : (BOOL) start;
- (void) setAnimationStatus:(CGFloat) status;

@end

#define RBB_TOP_BORDURE 10
#define RBB_BUTTON_HEIGHT	25
#define RBB_BUTTON_WIDTH	75		//	%
#define RBB_BUTTON_POSX		12.5f	//	%