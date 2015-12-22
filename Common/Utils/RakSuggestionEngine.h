/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **		Source code and assets are property of Taiki, distribution is stricly forbidden		**
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