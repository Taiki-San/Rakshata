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

@interface RakSuggestionEngine : NSObject

+ (instancetype) getShared;

- (NSArray <NSDictionary *> *) getSuggestionForProject : (uint) cacheID withNumber : (uint) nbSuggestions;
- (PROJECT_DATA) dataForIndex : (uint) index;

+ (BOOL) suggestionWasClicked : (uint) projectID withInsertionPoint : (NSDictionary *) insertionPoint;

@end

enum
{
	SUGGESTION_REASON_TAG = 1,
	SUGGESTION_REASON_AUTHOR,
	SUGGESTION_REASON_FAVORITE_NEW_STUFFS,
	SUGGESTION_REASON_FAVORITE_OLD,
	SUGGESTION_REASON_RANDOM,
};