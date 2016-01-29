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

#define PROJECT_PASTEBOARD_TYPE @"com.taiki.rakshataProject"

@interface RakDragItem : NSObject
{
	BOOL canDL;
}

@property PROJECT_DATA project;
@property BOOL isTome, fullProject;
@property uint selection;
@property uint price;

- (void) setDataProject : (PROJECT_DATA) project fullProject : (BOOL) fullProject isTome : (BOOL) isTome element : (uint) element;
- (instancetype) initWithData : (NSData *) data;

- (NSData *) getData;
- (BOOL) isFavorite;
+ (BOOL) isFavorite : (NSPasteboard*) pasteboard;
- (BOOL) canDL;
- (BOOL) canDLRefreshed;
+ (BOOL) canDL : (NSPasteboard*) pasteboard;
+ (BOOL) canDL : (PROJECT_DATA) project isTome : (BOOL) isTome element : (uint) element;
+ (BOOL) defineIsTomePriority : (PROJECT_DATA*) project  alreadyRefreshed : (BOOL) refreshed;

@end
