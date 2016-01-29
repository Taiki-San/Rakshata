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

@interface RakCTSelection : RakView
{
	PROJECT_DATA data;
	RakCTCoreViewButtons * _buttons;
	RakCTSelectionListContainer * _chapterView;
	RakCTSelectionListContainer * _volView;
	
	uint _cachedHeaderHeight;
}

@property (nonatomic) uint currentContext;

@property BOOL dontNotify;
@property (readonly) PROJECT_DATA currentProject;


- (instancetype) initWithProject : (PROJECT_DATA) project : (BOOL) isTome : (NSRect) parentBounds : (CGFloat) headerHeight : (long [4]) context : (uint) mainThread;

- (void) setFrame : (NSRect) parentFrame : (CGFloat) headerHeight;
- (void) resizeAnimation : (NSRect) parentFrame : (CGFloat) headerHeight;

- (NSString *) getContextToGTFO;

- (void) gotClickedTransmitData : (NSNotification*) notification;

- (void) feedAnimationController : (RakCTAnimationController *) animationController;
- (void) switchIsTome;

- (void) selectElem : (uint) projectID : (BOOL) isTome : (uint) element;
- (BOOL) updateContext : (PROJECT_DATA) newData;

- (void) cleanChangeCurrentContext;

@end

