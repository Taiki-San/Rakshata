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

- (NSArray <NSNumber *> *) getSuggestionForProject : (PROJECT_DATA) project withNumber : (uint) nbSuggestions
{
	nbSuggestions = MIN(nbElem, nbSuggestions);
	
	NSMutableArray < NSNumber * > * array = [[NSMutableArray alloc] initWithCapacity:nbSuggestions];
	
	for (uint i = 0, value; i < nbSuggestions; i++)
	{
		//Prevent reusing IDs
		value = getRandom() % nbElem;
		while([array indexOfObject:@(value)] != NSNotFound)
		{
			++value;
			value %= nbElem;
		}
		
		[array addObject:@(value)];
	}
	
	return [NSArray arrayWithArray:array];
}

- (PROJECT_DATA) dataForIndex : (uint) index
{
	if(index >= nbElem)
		return getEmptyProject();
	
	return cache[index];
}

@end
