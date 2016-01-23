/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
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

#include "JSONParser.h"
#include "tag.h"

bool loadRemoteTagState(char * remoteDump, TAG_VERBOSE ** _tags, uint * _nbTags, CATEGORY_VERBOSE ** _categories, uint * _nbCategories, uint * newDBVersion)
{
	if(remoteDump == NULL || _tags == NULL || _nbTags == NULL || _categories == NULL || _nbCategories == NULL)
		return false;
	
	//We parse the JSON
	NSError * parseError = nil;
	NSDictionary * parseData = [NSJSONSerialization JSONObjectWithData:[NSData dataWithBytes:remoteDump length:strlen(remoteDump)] options:0 error:&parseError];
	
	if(parseError != nil)
	{
#ifdef EXTENSIVE_LOGGING
		NSLog(@"%@", parseError);
#endif
		return false;
	}
	else if(![parseData isKindOfClass:[NSDictionary class]])
		return false;
	
	//We first extract the DB version
	NSNumber * version = objectForKey(parseData, JSON_TAG_VERSION, @"version", [NSNumber class]);
	if(version == nil)
		return false;
	else
		*newDBVersion = [version unsignedIntValue];

	//The JSON is composed of two main sections
	NSArray * mainTags = objectForKey(parseData, JSON_TAG_TAGS, @"tags", [NSArray class]), * mainCats = objectForKey(parseData, JSON_TAG_CATEGORY, @"type", [NSArray class]);

	if(mainTags == nil || mainCats == nil)
		return false;
	
	uint nbTags = [mainTags count], nbCategories = [mainCats count];
	if(nbTags == 0 || nbCategories == 0)
		return false;
	
	//We start by allocating memory (last critical failure point at this point)
	TAG_VERBOSE * tags = malloc(nbTags * sizeof(TAG_VERBOSE));
	if(tags == NULL)
		return false;
	
	CATEGORY_VERBOSE * categories = malloc(nbCategories * sizeof(CATEGORY_VERBOSE));
	if(categories == NULL)
	{
		free(tags);
		return false;
	}
	
	uint currentPosTag = 0;
	for(NSDictionary * currentTag in mainTags)
	{
		if(ARE_CLASSES_DIFFERENT(currentTag, [NSDictionary class]))
			continue;
		
		NSNumber * ID = objectForKey(currentTag, JSON_TAG_ID, @"id", [NSNumber class]);
		if(ID == nil || [ID unsignedIntValue] == CAT_NO_VALUE)
			continue;
		
		NSString * name = objectForKey(currentTag, JSON_TAG_NAME, @"name", [NSString class]);
		if(name == nil || [name length] == 0)
			continue;
		
		//Data sanitized, copy them
		tags[currentPosTag].name = wstrdup((charType *) [name cStringUsingEncoding:NSUTF32StringEncoding]);
		if(tags[currentPosTag].name == NULL)
			continue;
		
		tags[currentPosTag++].ID = [ID unsignedIntValue];
	}
	
	//Reducing the buffer size if we didn't used all the space
	if(currentPosTag < nbTags)
	{
		if(currentPosTag == 0)	//No data, WTF
		{
			free(tags);
			free(categories);
			return false;
		}
		
		void * tmp = realloc(tags, currentPosTag * sizeof(TAG_VERBOSE));
		if(tmp != NULL)
			tags = tmp;
		
		nbTags = currentPosTag;
	}
	
	//Now, we can analyse categories
	uint currentPosCat = 0;
	for(NSDictionary * currentCat in mainCats)
	{
		NSNumber * ID = objectForKey(currentCat, JSON_TAG_ID, @"id", [NSNumber class]);
		if(ID == nil || [ID unsignedIntValue] == CAT_NO_VALUE)
			continue;
		
		NSNumber * master = objectForKey(currentCat, JSON_TAG_MASTER, @"master", [NSNumber class]);
		if(master == nil)
			continue;
		
		NSString * name = objectForKey(currentCat, JSON_TAG_NAME, @"name", [NSString class]);
		if(name == nil || [name length] == 0)
			continue;
		
		//Data sanitized, copy them
		categories[currentPosCat].name = wstrdup((charType *) [name cStringUsingEncoding:NSUTF32StringEncoding]);
		if(categories[currentPosCat].name == NULL)
			continue;
		
		categories[currentPosCat].ID = [ID unsignedIntValue];
		categories[currentPosCat].rootID = [master unsignedIntValue];
	}
	
	//We reduce the size of the allocated memory if needed
	if(currentPosCat < nbCategories)
	{
		if(currentPosCat == 0)
		{
			//We release everythig
			for(uint i = 0; i < nbTags; free(tags[i++].name));
			free(tags);
			free(categories);
			return false;
		}
		
		void * tmp = realloc(categories, currentPosCat * sizeof(CATEGORY_VERBOSE));
		if(tmp != NULL)
			categories = tmp;
		
		nbCategories = currentPosCat;
	}
	
	*_tags = tags;
	*_nbTags = nbTags;
	*_categories = categories;
	*_nbCategories = nbCategories;
	
	return true;
}

void resetTagsToLocal()
{
	NSString * tags = [[NSBundle mainBundle] pathForResource:@"backupTags" ofType:@"db"];

	if(tags == nil)
	{
		//SHIIIIIT, can't access the local DB, no choice but download it :|
		checkIfRefreshTag();
	}
	else
		[[NSData dataWithContentsOfFile:tags] writeToFile:@""TAG_DB"" options:NSDataWritingAtomic error:nil];
}