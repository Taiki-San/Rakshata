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

@interface RakSerieListItem : RakOutlineListItem
{
	BOOL _isRecentList;
	BOOL _isDLList;
	BOOL _isMainList;
	
	CGFloat _mainListHeight;
	
	PROJECT_DATA dataChild;
}

- (instancetype) init : (void*) data : (BOOL) isRootItem : (int) initStage : (uint) nbChildren;

- (BOOL) isRecentList;
- (BOOL) isDLList;
- (BOOL) isMainList;

- (void) setMainListHeight : (CGFloat) height;
- (void) resetMainListHeight;

- (void) setNbChildren : (uint) nbChildren : (BOOL) flush;

- (PROJECT_DATA) getRawDataChild;

@end
