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

__strong RakSuggestionEngine * sharedObject;

@interface RakSuggestionEngine ()
{
	PROJECT_DATA * cache;
	uint nbElem;
}

@end

@implementation RakSuggestionEngine

#pragma mark - Initialization

- (instancetype) init
{
	self = [super init];
	if(self != nil)
	{
		cache = getCopyCache(SORT_NAME, &nbElem);
		[RakDBUpdate registerForUpdate:self :@selector(DBUpdated:)];
	}
	
	return self;
}

+ (instancetype) getShared
{
	if(sharedObject == nil)
		sharedObject = [[RakSuggestionEngine alloc] init];
	
	return sharedObject;
}

#pragma mark - Context update

- (void) DBUpdated : (NSNotification *) notification
{
	if(![RakDBUpdate isProjectUpdate:notification.userInfo])
		return;
	
	freeProjectData(cache);
	cache = getCopyCache(SORT_NAME, &nbElem);
}

#pragma mark - Query

//Random placeholder
- (NSArray <NSDictionary *> *) getSuggestionForProject : (uint) cacheID withNumber : (uint) nbSuggestions
{
	nbSuggestions = MIN(cacheID != INVALID_VALUE ? nbElem - 1 : nbElem, nbSuggestions);
	
	NSMutableArray < NSDictionary * > * array = [[NSMutableArray alloc] initWithCapacity:nbSuggestions];
	NSMutableArray < NSNumber *> * usedID = [[NSMutableArray alloc] initWithCapacity:nbSuggestions];
	
	uint nbFavs;
	
	SUGGESTIONS_FAVS * favorites = getIDOfInterestingFavorites(cacheID, ceil(nbSuggestions / 2.0), &nbFavs);
	if(favorites != NULL)
	{
		for(uint i = 0; i < nbFavs; ++i)
		{
			[usedID addObject:@(favorites[i].ID)];
			
			byte reason;
			if(favorites[i].priority != SUGG_PRIORITY_FAVS_OLD)
				reason = SUGGESTION_REASON_FAVORITE_NEW_STUFFS;
			else
				reason = SUGGESTION_REASON_FAVORITE_OLD;
			
			NSDictionary * insertionPoint;
			if(favorites[i].indexInsertionID != INVALID_VALUE)
				insertionPoint = @{@"isTome" : @(favorites[i].isTome), @"ID" : @(favorites[i].indexInsertionID)};
			else
				insertionPoint = @{@"isTome" : @(favorites[i].isTome)};
			
			[array addObject:@{@"ID" : @(favorites[i].ID), @"reason" : @(reason), @"insertionPoint" : insertionPoint}];
		}
	}
	
	return [self fillWithRandomData:array containing:usedID project:cacheID withNumber:nbSuggestions];
}

- (NSArray <NSDictionary *> *) fillWithRandomData : (NSMutableArray <NSDictionary *> *) array containing: (NSMutableArray <NSNumber *> *) usedID project : (uint) cacheID withNumber : (uint) nbSuggestions
{
	for (uint i = [array count], value; i < nbSuggestions; i++)
	{
		//Prevent reusing IDs
		value = getRandom() % nbElem;
		while(cache[value].cacheDBID == cacheID || [usedID indexOfObject:@(value)] != NSNotFound)
		{
			++value;
			value %= nbElem;
		}
		
		[usedID addObject:@(value)];
		[array addObject:@{@"ID" : @(value), @"reason" : @(SUGGESTION_REASON_RANDOM)}];
	}
	
	return [NSArray arrayWithArray:array];
}

//Current recipe
//Quand arrive à fin de liste, pop-up sur bouton suivant pour proposer suggestions
//↪ si un projet est favoris, et il y a un chapitre non-lu, suggestion prioritaire
//↪ 1 du même tag (si impossible, catégorie)
//↪ 1 du même auteur (si impossible, même source)

- (PROJECT_DATA) dataForIndex : (uint) index
{
	if(index >= nbElem)
		return getEmptyProject();
	
	return cache[index];
}

#pragma mark - Process a click

//Return YES if could process the suggestion, NO otherwise
+ (BOOL) suggestionWasClicked : (uint) projectID withInsertionPoint : (NSDictionary *) insertionPoint
{
	PROJECT_DATA project = getProjectByID(projectID);
	if(!project.isInitialized)
		return NO;
	
	STATE_DUMP state = recoverStateForProject(project);
	if(projectHaveValidSavedState(project, state))
	{
		//We restore the page if the reading was interrupted
		if(!state.wasLastPage)
		{
			[[NSNotificationCenter defaultCenter] postNotificationName:NOTIFICATION_RESUME_READING object:@(projectID) userInfo:insertionPoint];
			goto end;
		}
		
		//Okay, it stopped reading at the end of a CT, let's check if there is something after that
		uint index = reader_getPosIntoContentIndex(project, state.CTID, state.isTome);
		if(index != INVALID_VALUE && ++index < ACCESS_DATA(state.isTome, project.nbChapterInstalled, project.nbVolumesInstalled))
		{
			[RakTabView broadcastUpdateContext: nil : project : state.isTome : ACCESS_CT(state.isTome, project.chaptersInstalled, project.volumesInstalled, index)];
			goto end;
		}
		
		//Okay, either the CT doesn't exist, or it was the last one
		//If it was the last one, we'll just loop back to the first one, which is the default behavior
		//Otherwise, we try to find the closest match
		if(index == INVALID_VALUE)
		{
			//Okayyy, the CT doesn't exist anymore, or was deleted and we couldn't find anything after it, yay
			//We try to find the first CT following it
			index = reader_findReadableAfter(project, state.CTID, state.isTome);
			if(index != INVALID_VALUE)
			{
				[RakTabView broadcastUpdateContext:nil :project :state.isTome :ACCESS_CT(state.isTome, project.chaptersInstalled, project.volumesInstalled, index)];
				goto end;
			}
		}
		
		//Well, we just give up...
	}
	
	//We haven't read anything yet, but have we downloaded something yet?
	if(project.nbVolumesInstalled > 0 || project.nbChapterInstalled > 0)
	{
		bool isTome = project.nbVolumesInstalled != 0;

		[RakTabView broadcastUpdateContext:nil :project :isTome :ACCESS_CT(isTome, project.chaptersInstalled, project.volumesInstalled, 0)];
	}

	//Nop, let's just open the CT tab
	else
	{
		[RakTabView broadcastUpdateContext:nil :project :NO :INVALID_VALUE];
		[RakApp.CT ownFocus];
	}
	
end:
	releaseCTData(project);
	return YES;
}

@end
