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
 ********************************************************************************************/

#define SYNOPSIS_TOP_BORDER_WIDTH 	5
#define SYNOPSIS_SPACING			5
#define SYNOPSIS_BORDER 			BORDER_CT_FOCUS
#define SYNOPSIS_MAIN_TEXT_BORDER 	10

@interface RakSynopsis : RakView
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

- (BOOL) setStringToSynopsis : (NSString *) synopsis;

- (BOOL) postProcessScrollView;
- (BOOL) generatedScrollView : (NSRect) frame;
- (void) updateScrollViewState;

- (NSRect) frameFromParent : (NSRect) parentFrame;
- (NSPoint) placeholderOrigin : (NSRect) scrollviewBounds;
- (NSRect) frameForContent : (NSRect) mainBounds;

- (void) resizeAnimation : (NSRect) frameRect;
- (void) _updateFrame : (NSRect) mainFrame : (BOOL) animated;

@end
