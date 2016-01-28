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

@interface RakTabContentTemplate : RakView
{
	uint mainThread;
	
	BOOL isFocusDrop;
	
	BOOL _animatedContextChange;
	BOOL _ignoredPreviousContextChange;
	
	BOOL _CTViewHidden;
	BOOL _readerViewHidden;
}

@property BOOL serieViewHidden;
@property BOOL CTViewHidden;
@property BOOL readerViewHidden;

- (void) setupInternal;
- (NSString *) getContextToGTFO;

- (void) setFocusDrop : (BOOL) isFocus;
- (BOOL) isFocusDrop;
- (void) updateViewForFocusDrop;

- (void) resizeAnimation : (NSRect)frameRect;

- (void) setFrameInternalViews : (NSRect) newBound;
- (void) resizeAnimationInternalViews : (NSRect) newBound;

- (void) drawFocusRing;

- (RakColor*) getBackgroundColor;
- (RakColor*) getBorderColor;

- (void) focusViewChanged : (uint) newMainThread;
- (void) cleanupFocusViewChange;

@end
