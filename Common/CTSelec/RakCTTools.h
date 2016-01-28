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

@interface RakCTAnimationController : RakAnimationController
{
	RakSegmentedButtonCell * _cell;
	RakCTSelectionListContainer * _chapter;
	RakCTSelectionListContainer * _volume;
	
	CGFloat distanceToCoverPerMark;
	NSPoint chapOrigin;
	NSPoint volOrigin;
}

- (instancetype) init : (NSInteger) initialPos : (CGFloat) diff : (RakSegmentedButtonCell*) cell;

- (void) addCTContent : (RakCTSelectionListContainer*) chapter : (RakCTSelectionListContainer*) volume;

@end

@interface RakCTProjectImageView : NSImageView
{
	PROJECT_DATA _cachedProject;
}

- (instancetype) initWithImageName : (PROJECT_DATA) project : (NSRect) superviewFrame;
- (void) updateProject : (PROJECT_DATA) project;

- (NSRect) getProjectImageSize : (NSRect) superviewFrame : (NSSize) imageSize;

- (void) resizeAnimation : (NSRect) frameRect;

@end

@interface RakCTCoreViewButtons : RakSegmentedControl

@end