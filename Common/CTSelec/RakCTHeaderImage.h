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

#define CT_HEADER_MIN_HEIGHT 200

@class RakCTHeaderImage;

#import "RakCTHImage.h"
#import "RakCTHTableController.h"
#import "RakCTHContainer.h"

@interface RakCTHeaderImage : RakView
{
	PROJECT_DATA _data;
	
	//Elements
	RakCTHImage * _background;
	RakCTHContainer * _container;
}

- (instancetype) initWithData : (NSRect) frame : (PROJECT_DATA) project;
- (void) updateHeaderProject : (PROJECT_DATA) project;
- (BOOL) updateHeaderProjectInternal : (PROJECT_DATA) project;

- (void) resizeAnimation : (NSRect) frameRect;
- (NSRect) frameByParent : (NSRect) parentFrame;
- (NSSize) sizeByParent : (NSRect) parentFrame;

@end
