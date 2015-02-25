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
 *********************************************************************************************/

#include "JSONParser.h"

#define MINIMUM_VERSION_FOR_DUMPER 200

#pragma mark - Utilities

charType ** parseDescriptions(NSArray * array, char *** languages, uint *length)
{
	//Initial sanity checks
	if(array == nil || ![array isKindOfClass:[NSArray class]] || languages == NULL || length == NULL)
		return NULL;
	
	//We check that there is work to do, and if so allocate the memory we're going to need
	uint localLength = [array count];
	
	if(localLength == 0)
		return NULL;
	
	charType ** output = calloc(localLength, sizeof(charType *));
	*languages = calloc(localLength, sizeof(char*));
	
	if(output == NULL || *languages == NULL)
	{
		free(output);
		free(*languages);
		*languages = NULL;
		return NULL;
	}
	
	//Great, now, let's parse the whole thing
	uint failure = 0, pos = 0, lengthDescription, lengthLanguage;
	NSString * description, *language;
	
	for(NSDictionary * line in array)
	{
		//Sanitization and extraction
		if(![line isKindOfClass:[NSDictionary class]])
		{
			failure++;
			continue;
		}
		
		description = objectForKey(line, JSON_REPO_DESCRIPTION, @"description");
		if(description == nil || ![description isKindOfClass:[NSString class]])
		{
			failure++;
			continue;
		}
		
		language = objectForKey(line, JSON_REPO_LANGUAGE, @"language");
		if(language == nil || ![language isKindOfClass:[NSString class]] || [language length] > REPO_LANGUAGE_LENGTH - 1)
		{
			failure++;
			continue;
		}
		
		//Memory allocation
		lengthDescription = [description length];
		
		output[pos] = malloc((lengthDescription + 1) * sizeof(charType));
		if(output[pos] == NULL)
		{
			failure++;
			continue;
		}
		
		lengthLanguage = [language length];
		(*languages)[pos] = malloc((lengthLanguage + 1) * sizeof(char));
		if((*languages)[pos] == NULL)
		{
			free(output[pos]);
			output[pos] = NULL;
			failure++;
			continue;
		}
		
		//Copy
		memcpy(output[pos], [description cStringUsingEncoding:NSUTF32LittleEndianStringEncoding], (lengthDescription + 1) * sizeof(charType));
		memcpy((*languages)[pos++], [language cStringUsingEncoding:NSASCIIStringEncoding], lengthLanguage + 1);
	}

	//If nothing was copied
	if (pos == 0)
	{
		free(output);
		free(*languages);
		*languages = NULL;
		return NULL;
	}
	else if(failure != 0)	//If there was any error, we drop the unecessary memory
	{
		void * tmp = realloc(output, pos * sizeof(charType*));
		if(tmp != NULL)
			output = tmp;
		
		tmp = realloc(*languages, pos * sizeof(char*));
		if(tmp != NULL)
			*languages = tmp;
	}
	
	*length = pos;
	
	return output;
}

bool validateTrust(NSString * input, bool localSource)
{
	if(!localSource)
		return false;
	
	if([input isKindOfClass:[NSNumber class]])
	{
		return [(NSNumber*) input boolValue];
	}
	
	return false;
}

REPO_DATA parseSingleSubRepo(NSDictionary * dict, uint parentID, bool * error)
{
	REPO_DATA output;
	output.active = false;
	
	if(error == NULL)
		return output;

	else if(dict == nil)
	{
fail:	//We'll jump back here when it's starting to go wrong
		*error = true;
		return output;
	}
	else
		*error = false;
	
	//Allocate everything we're going to need
	NSString *name, *URL, *language, *website;
	NSNumber *repoID, *type, *isMature;
	
	//Now, let's parse
	
	repoID = objectForKey(dict, JSON_REPO_SUB_ID, @"repoID");
	if(repoID == nil || ![repoID isKindOfClass:[NSNumber class]])
		goto fail;
	
	name = objectForKey(dict, JSON_REPO_NAME, @"repo_name");
	if(name == nil || ![name isKindOfClass:[NSString class]] || [name length] > REPO_NAME_LENGTH - 1)
		goto fail;
	
	language = objectForKey(dict, JSON_REPO_LANGUAGE, @"language");
	if(language == nil || ![language isKindOfClass:[NSString class]] || [language length] > REPO_LANGUAGE_LENGTH - 1)
		goto fail;
	
	type = objectForKey(dict, JSON_REPO_TYPE, @"type");
	if(type == nil || ![type isKindOfClass:[NSNumber class]])
		goto fail;
	
	URL = objectForKey(dict, JSON_REPO_URL, @"URL");
	if(URL == nil || ![URL isKindOfClass:[NSString class]] || [URL length] > REPO_URL_LENGTH - 1)
		goto fail;
	
	website = objectForKey(dict, JSON_REPO_SUB_WEBSITE, @"website");
	if(website == nil || ![website isKindOfClass:[NSString class]] || [website length] > REPO_WEBSITE_LENGTH - 1)
		goto fail;
	
	isMature = objectForKey(dict, JSON_REPO_SUB_MATURE, @"mature_content");
	if(isMature == nil || ![isMature isKindOfClass:[NSNumber class]])
		goto fail;
	
	//Great, parsing is over, copy time
	
	output.repoID = [repoID unsignedIntValue];
	wstrncpy(output.name, REPO_NAME_LENGTH, (void *) [name cStringUsingEncoding:NSUTF32LittleEndianStringEncoding]);
	strncpy(output.language, [language cStringUsingEncoding:NSASCIIStringEncoding], REPO_LANGUAGE_LENGTH);
	output.type = [type unsignedCharValue];
	strncpy(output.URL, [URL cStringUsingEncoding:NSASCIIStringEncoding], REPO_URL_LENGTH);
	strncpy(output.website, [website cStringUsingEncoding:NSASCIIStringEncoding], REPO_WEBSITE_LENGTH);
	output.isMature = [isMature boolValue];
	
	output.parentRepoID = parentID;
	output.active = true;
	
	return output;
}

#pragma mark - Main parsers

void * parseSubRepo(NSArray * array, bool wantExtra, uint * nbSubRepo, uint parentID)
{
	if(nbSubRepo == NULL)
		return NULL;
	
	size_t count = [array count];
	
	if(count == 0 || count > UINT_MAX)
		return NULL;
	
	bool error = false;
	uint pos = 0, failure = 0, sizeOfStruct = wantExtra ? sizeof(REPO_DATA_EXTRA) : sizeof(REPO_DATA);
	REPO_DATA * output = calloc(count, sizeOfStruct);
	REPO_DATA_EXTRA * outputExtra = (void*) output;			//Too lazy to cast too often
	
	if(output == NULL)
		return NULL;
	
	NSString * URLImage, * URLImageRetina, * hash, * hashRetina;
	
	for(NSDictionary * repo in array)
	{
		if(repo == nil || ![repo isKindOfClass:[NSDictionary class]])
		{
			failure++;
			continue;
		}

		if(!wantExtra)
		{
			output[pos] = parseSingleSubRepo(repo, parentID, &error);
			
			if(error)
				failure++;
			else
				pos++;
		}
		else
		{
			outputExtra[pos].data = calloc(1, sizeof(REPO_DATA));
			if(outputExtra[pos].data == NULL)
			{
				failure++;
				continue;
			}
			
			//We parse the different component
			URLImage = objectForKey(repo, JSON_REPO_SUB_IMAGE, @"image");
			if(URLImage != nil)
			{
				if(![URLImage isKindOfClass:[NSString class]] || [URLImage length] >= REPO_URL_LENGTH - 1)
				{
					failure++;
					continue;
				}
				
				hash = objectForKey(repo, JSON_REPO_SUB_IMAGE_HASH, @"hash_image");
				if(hash == nil || ![hash isKindOfClass:[NSString class]] || [hash length] != LENGTH_HASH - 1)
				{
					failure++;
					continue;
				}

				URLImageRetina = objectForKey(repo, JSON_REPO_SUB_RETINA_IMAGE, @"imageRetina");
				if(URLImageRetina != nil && (![URLImageRetina isKindOfClass:[NSString class]] || [URLImageRetina length] >= REPO_URL_LENGTH - 1))
				{
					URLImageRetina = nil;
				}
				else
				{
					hashRetina = objectForKey(repo, JSON_REPO_SUB_RETINA_HASH, @"hash_image_retina");
					if(hashRetina == nil || ![hashRetina isKindOfClass:[NSString class]] || [hashRetina length] != LENGTH_HASH - 1)
					{
						failure++;
						continue;
					}
				}
			}
			
			*(outputExtra[pos].data) = parseSingleSubRepo(repo, parentID, &error);
			
			if(error)
			{
				free(outputExtra[pos].data);
				failure++;
				continue;
			}
			
			//Now, time to copy the rest of the data
			
			if(URLImage != nil)
			{
				strncpy(outputExtra[pos].URLImage, [URLImage cStringUsingEncoding:NSASCIIStringEncoding], REPO_URL_LENGTH);
				strncpy(outputExtra[pos].hashImage, [hash cStringUsingEncoding:NSASCIIStringEncoding], LENGTH_HASH);
				
				if(URLImageRetina != nil)
				{
					strncpy(outputExtra[pos].URLImageRetina, [URLImageRetina cStringUsingEncoding:NSASCIIStringEncoding], REPO_URL_LENGTH);
					strncpy(outputExtra[pos].hashImageRetina, [hashRetina cStringUsingEncoding:NSASCIIStringEncoding], LENGTH_HASH);
					outputExtra[pos].haveRetina = true;
				}
				else
					outputExtra[pos].haveRetina = false;
			}
			else
			{
				outputExtra[pos].URLImage[0] = 0;
				outputExtra[pos].hashImage[0] = 0;
				outputExtra[pos].haveRetina = false;
			}
			
			pos++;
		}
	}
	
	if(pos == 0)
	{
		free(output);
		return NULL;
	}
	else if(failure != 0)
	{
		void * tmp = realloc(output, pos * sizeOfStruct);
		if(tmp != NULL)
			output = tmp;
	}
	
	*nbSubRepo = pos;
	
	return output;
}

ROOT_REPO_DATA * parseRootRepo(NSDictionary * parseData, bool wantExtra, bool localSource)
{
	//Great, parseData contain the serialized structure, let's go
	
	NSNumber * version = objectForKey(parseData, JSON_REPO_MIN_VERSION, @"minimum_rakshata_version");
	if(version == nil || ![version isKindOfClass:[NSNumber class]])
		return NULL;
	else if([version unsignedIntegerValue] > CURRENTVERSION)
	{
		logR("Unsupported file, please update Rakshata");
		return NULL;
	}
	
	bool trusted;
	uint nbDescriptions = 0;
	charType ** descriptions = NULL;
	char ** languages;
	
	ROOT_REPO_DATA * root = calloc(1, sizeof(ROOT_REPO_DATA)), *rootWip = root;
	if(root == NULL)
		return NULL;
	else
		root = NULL;
	
	//Create everything we need
	NSString *name, *URL;
	NSNumber * type, *ID;
	NSArray * array;
	
	//Parse the shit out of this file
	name = objectForKey(parseData, JSON_REPO_NAME, @"root_repo_name");
	if(name == nil || ![name isKindOfClass:[NSString class]] || [name length] >= REPO_NAME_LENGTH - 1)
		goto error;
	
	type = objectForKey(parseData, JSON_REPO_TYPE, @"type");
	if(type == nil || ![type isKindOfClass:[NSNumber class]])
		goto error;
	
	URL = objectForKey(parseData, JSON_REPO_URL, @"URL");
	if(URL == nil || ![URL isKindOfClass:[NSString class]] || [URL length] >= REPO_URL_LENGTH - 1)
		goto error;
	
	array = objectForKey(parseData, JSON_REPO_DESCRIPTION, @"description");
	if(array == nil || ![array isKindOfClass:[NSArray class]])
		goto error;
	
	descriptions = parseDescriptions(array, &languages, &nbDescriptions);
	trusted = validateTrust(objectForKey(parseData, JSON_REPO_TRUSTED, @"trusted"), localSource);	//Not required, so if it doesn't work, there is no big deal
	
	array = objectForKey(parseData, JSON_REPO_REPO_TREE, @"repository");
	if(array == nil || ![array isKindOfClass:[NSArray class]])
		goto error;
	
	//Okay, the root parsing is over, the sub-repo parsing is performed by an other routine
	
	if(localSource)
	{
		ID = objectForKey(parseData, JSON_REPO_ID, @"GUID");
		if(ID == nil || ![ID isKindOfClass:[NSNumber class]])
			goto error;
		
		rootWip->repoID = [ID unsignedIntValue];
	}

	rootWip->subRepo = parseSubRepo(array, wantExtra, &(rootWip->nombreSubrepo), rootWip->repoID);
	rootWip->subRepoAreExtra = wantExtra;
	
	wstrncpy(rootWip->name, REPO_NAME_LENGTH, (void*) [name cStringUsingEncoding:NSUTF32LittleEndianStringEncoding]);
	usstrcpy(rootWip->URL, REPO_URL_LENGTH, [URL cStringUsingEncoding:NSASCIIStringEncoding]);
	
	rootWip->descriptions = descriptions;
	rootWip->langueDescriptions = languages;
	rootWip->nombreDescriptions = nbDescriptions;

	rootWip->type = [type unsignedCharValue];
	rootWip->trusted = trusted;
	
	root = rootWip;
	rootWip = NULL;
	descriptions = NULL;
	languages = NULL;
error:

	free(rootWip);
	
	if(descriptions != NULL)
	{
		for(uint i = 0; i < nbDescriptions; free(descriptions[i++]));
		free(descriptions);
	}

	if(languages != NULL)
	{
		for(uint i = 0; i < nbDescriptions; free(languages[i++]));
		free(languages);
	}

	return root;
}

ROOT_REPO_DATA * parseRemoteRepo(char * parseDataRaw)
{
	NSError * parseError = nil;
	NSDictionary * parseData = [NSJSONSerialization JSONObjectWithData:[NSData dataWithBytes:parseDataRaw length:ustrlen(parseDataRaw)] options:0 error:&parseError];
	
	if(parseError != nil)
	{
#ifdef DEV_VERSION
		NSLog(@"%@", parseError);
#endif
		return NULL;
	}
	else if(parseData == nil || ![parseData isKindOfClass: [NSDictionary class]])
		return NULL;

	return parseRootRepo(parseData, true, false);
}

ROOT_REPO_DATA * parseLocalRootRepo(NSDictionary * rootRepo)
{
	return parseRootRepo(rootRepo, false, true);
}

ROOT_REPO_DATA ** parseLocalRepo(char * parseDataRaw, uint * nbElem)
{
	if(parseDataRaw == NULL || nbElem == NULL)
		return NULL;
	
	NSError * parseError = nil;
	NSArray * parseData = [NSJSONSerialization JSONObjectWithData:[NSData dataWithBytes:parseDataRaw length:ustrlen(parseDataRaw)] options:0 error:&parseError];
	
	if(parseError != nil)
	{
#ifdef DEV_VERSION
		NSLog(@"%@", parseError);
#endif
		return NULL;
	}
	else if(parseData == nil || ![parseData isKindOfClass: [NSArray class]])
		return NULL;
	
	uint length = [parseData count];
	
	if(length == 0)
		return NULL;
	
	//Keep one empty slot
	ROOT_REPO_DATA ** root = calloc(length + 1, sizeof(ROOT_REPO_DATA *));

	if(root != NULL)
	{
		uint count = 0;
		
		for(NSDictionary * dict in parseData)
		{
			if(![dict isKindOfClass:[NSDictionary class]])
				continue;
			
			root[count] = parseRootRepo(dict, false, true);

			if(root[count] != NULL)
				count++;
		}
		
		*nbElem = count;
	}
	
	return root;
}

#pragma mark - Linearizer

NSArray * rebuildDescriptions(charType ** descriptions, char ** langueDescriptions, uint nombreDescriptions)
{
	if(descriptions == NULL || langueDescriptions == NULL || nombreDescriptions == 0)
		return nil;
	
	NSMutableArray * output = [NSMutableArray arrayWithCapacity:nombreDescriptions];
	NSMutableDictionary * dictionary;
	
	for(uint i = 0; i < nombreDescriptions; i++)
	{
		dictionary = [NSMutableDictionary dictionaryWithCapacity:2];
		
		[dictionary setObject:getStringForWchar(descriptions[i]) forKey:JSON_REPO_DESCRIPTION];
		[dictionary setObject:[NSString stringWithUTF8String:langueDescriptions[i]] forKey:JSON_REPO_LANGUAGE];
		
		[output addObject:[NSDictionary dictionaryWithDictionary:dictionary]];
	}
	
	return [NSArray arrayWithArray:output];
}

NSArray * rebuildRepoTree(REPO_DATA * subRepo, uint nombreSubrepo, bool isExtra)
{
	if(subRepo == NULL || nombreSubrepo == 0)
		return nil;
	
	NSMutableArray * output = [NSMutableArray arrayWithCapacity:nombreSubrepo];
	NSMutableDictionary * dict;
	REPO_DATA * element;
	
	for(uint i = 0; i < nombreSubrepo; i++)
	{
		dict = [NSMutableDictionary dictionaryWithCapacity:7];
		
		if(isExtra)
			element = ((REPO_DATA_EXTRA *) subRepo)[i].data;
		else
			element = &subRepo[i];
		
		[dict setObject:@(element->repoID) forKey:JSON_REPO_SUB_ID];
		[dict setObject:getStringForWchar(element->name) forKey:JSON_REPO_NAME];
		[dict setObject:[NSString stringWithUTF8String:element->language] forKey:JSON_REPO_LANGUAGE];
		[dict setObject:@(element->type) forKey:JSON_REPO_TYPE];
		[dict setObject:[NSString stringWithUTF8String:element->URL] forKey:JSON_REPO_URL];
		[dict setObject:[NSString stringWithUTF8String:element->website] forKey:JSON_REPO_SUB_WEBSITE];
		[dict setObject:@(element->isMature) forKey:JSON_REPO_SUB_MATURE];
		
		[output addObject:dict];
	}
	
	return [NSArray arrayWithArray:output];
}

NSDictionary * linearizeRootRepo(ROOT_REPO_DATA * root)
{
	if(root == NULL)
		return nil;
	
	//Will leak one empty element if no description but don't survive the end of the function, so we don't care
	NSMutableDictionary * dict = [NSMutableDictionary dictionaryWithCapacity:8];
	NSArray * array;
	
	[dict setObject:@(MINIMUM_VERSION_FOR_DUMPER) forKey:JSON_REPO_MIN_VERSION];
	[dict setObject:getStringForWchar(root->name) forKey:JSON_REPO_NAME];
	[dict setObject:@(root->type) forKey:JSON_REPO_TYPE];
	[dict setObject:[NSString stringWithUTF8String:root->URL] forKey:JSON_REPO_URL];
	[dict setObject:@(root->repoID) forKey:JSON_REPO_ID];

	array = rebuildDescriptions(root->descriptions, root->langueDescriptions, root->nombreDescriptions);
	if(array != nil)
		[dict setObject:array forKey:JSON_REPO_DESCRIPTION];
	
	if(root->trusted)
		[dict setObject:@(YES) forKey:JSON_REPO_TRUSTED];
	
	array = rebuildRepoTree(root->subRepo, root->nombreSubrepo, root->subRepoAreExtra);
	if(array == nil)
		return nil;
	
	[dict setObject:array forKey:JSON_REPO_REPO_TREE];
	
	return [NSDictionary dictionaryWithDictionary:dict];
}

char * linearizeRepoData(ROOT_REPO_DATA ** root, uint rootLength, size_t * sizeOutput)
{
	if(root == NULL)
		return NULL;
	
	NSMutableArray * array = [NSMutableArray arrayWithCapacity:rootLength];
	if(array != nil)
	{
		NSDictionary * dict;
		
		for(uint i = 0; i < rootLength; i++)
		{
			dict = linearizeRootRepo(root[i]);
			
			if(dict != nil)
				[array addObject:dict];
		}
	}
	
	NSError * error = nil;
	NSData * dataOutput = [NSJSONSerialization dataWithJSONObject:array options:0 error:&error];
	
	size_t length = [dataOutput length];
	void * outputDataC = malloc(length);
	
	if (dataOutput == NULL)
		return NULL;
	
	[dataOutput getBytes:outputDataC length:length];
	
	char * output = base64_encode(outputDataC, length, sizeOutput);
	
	free(outputDataC);
	
	return output;
	
}
