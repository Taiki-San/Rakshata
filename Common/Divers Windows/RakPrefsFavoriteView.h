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

@interface RakPrefsFavoriteView : RakView

@end

@interface RakPrefsFavoriteList : RakList
{
	PROJECT_DATA * projectDB;
	uint * IDList, lengthList;
	
	//Internal data
	uint lastTransmittedSelectedRowIndex;
	
	NSMutableArray * repoRefresh;
}

- (instancetype) initWithFrame : (NSRect) frame;

- (BOOL) isRepoRefreshing : (uint64_t) repoID;
- (BOOL) tryInsertRepo : (uint64_t) repoID;
- (void) removeRepo : (uint64_t) repoID;

@end

@interface RakPrefsFavoriteListView : RakListItemView
{
	RakPrefsFavoriteList * _mainList;
	
	RakClickableText * repo;
	RakButton * refresh, * read, * remove, * download;
	
	BOOL refreshing;
}

@property PROJECT_DATA project;

- (instancetype) initWithProject : (PROJECT_DATA) project andList : (RakPrefsFavoriteList *) mainList;
- (void) updateContent : (PROJECT_DATA) project;

+ (void) readProject : (PROJECT_DATA) project;

@end
