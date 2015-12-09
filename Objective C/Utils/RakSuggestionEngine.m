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
	nbSuggestions = MIN(nbElem, nbSuggestions);
	
	NSMutableArray < NSDictionary * > * array = [[NSMutableArray alloc] initWithCapacity:nbSuggestions];
	NSMutableArray < NSNumber *> * usedID = [[NSMutableArray alloc] initWithCapacity:nbSuggestions];
	
	for (uint i = 0, value; i < nbSuggestions; i++)
	{
		//Prevent reusing IDs
		value = getRandom() % nbElem;
		while([usedID indexOfObject:@(value)] != NSNotFound)
		{
			++value;
			value %= nbElem;
		}
		
		[usedID addObject:@(value)];
		[array addObject:@{@"ID" : @(value), @"reason" : getRandom() & 0x1 ? @(SUGGESTION_REASON_TAG) : @(SUGGESTION_REASON_AUTHOR)}];
	}
	
	return [NSArray arrayWithArray:array];
}

//Actual receipe
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

@end
