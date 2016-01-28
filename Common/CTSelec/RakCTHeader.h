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

#import "RakCTHeaderImage.h"
#import "RakCTProjectSynopsis.h"

@interface RakCTHeader : RakView
{
	RakCTHeaderImage * header;
	RakCTProjectSynopsis * synopsis;
	
	//Background thingy
	RakColor * _backgroundColor;
	RakColor * _synopsisTitleBackground;
	CGFloat _synopsisTitleHeight;
	
	//Optimization
	NSRect _cachedFrame;
	BOOL forceUpdate;
}

- (instancetype) initWithData : (NSRect) frameRect : (PROJECT_DATA) project;
- (void) updateProject : (PROJECT_DATA) project;
- (void) updateProjectDiff : (PROJECT_DATA) oldData : (PROJECT_DATA) newData;

- (NSRect) frameByParent : (NSRect) parentFrame : (RakCTHeaderImage *) _header;

- (void) resizeAnimation : (NSRect) frame;

@end
