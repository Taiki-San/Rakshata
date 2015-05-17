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

void * parseChapterStructure(NSArray * chapterBloc, uint * nbElem, BOOL isChapter, BOOL paidContent, uint ** chaptersPrice)
{
	if(nbElem != NULL)
		*nbElem = 0;
	
	if (chapterBloc == NULL)
		return NULL;

	if(paidContent && chaptersPrice == NULL)
		return NULL;
	
	void * output = NULL;
	uint counterVar = 0, *counter, pos = 0, pricePos = 0;
	size_t nbSubBloc = [chapterBloc count];

	if (nbSubBloc == 0)
	{
		if(isChapter)
		{
			output = malloc(sizeof(int));
			if (output != nil)
				((int *)output)[0] = VALEUR_FIN_STRUCT;
		}
	}
	else
	{
		id entry1, entry2 = nil;
		int jump, first, last, sum;
		void* tmp;
		
		BOOL isNativeIfVolume = NO;
		uint typeSize = isChapter ? sizeof(int) : sizeof(CONTENT_TOME);
		
		if (nbElem != NULL)		counter = nbElem;
		else					counter = &counterVar;
		
		for(NSDictionary * dictionary in chapterBloc)
		{
			if([dictionary superclass] != [NSMutableDictionary class])	continue;
			
			entry1 = objectForKey(dictionary, JSON_PROJ_CHAP_DETAILS, @"details");
			if(isChapter && paidContent)
				entry2 = objectForKey(dictionary, JSON_PROJ_PRICE, @"price");
			else if(!isChapter)
				entry2 = objectForKey(dictionary, JSON_PROJ_VOL_ISRESERVEDTOVOL, @"privateTome");
				
			if(entry1 != nil && [(NSObject*) entry1 superclass] == [NSArray class])	//This is a special chunck
			{
				*counter += [(NSArray*) entry1 count];
				
				if(!*counter)
					continue;
				
				else if((tmp = realloc(output, (*counter + 1) * typeSize)) != NULL)
				{
					output = tmp;
					
					if(isChapter)
					{
						if(entry2 != nil && (tmp = realloc(*chaptersPrice, *counter * sizeof(int))) != NULL)
						{
							*chaptersPrice = tmp;
							
							if(entry2 == nil)
								memset(&((*chaptersPrice)[pricePos]), 0, (*counter - pricePos) * sizeof(int));
							else
							{
								uint count = *counter;
								for(NSNumber * entry3 in entry2)
								{
									if(pricePos > count)
										break;
									
									if([entry3 superclass] != [NSNumber class])
										continue;
									
									(*chaptersPrice)[pricePos++] = [entry3 unsignedIntValue];
								}
							}
						}
					}
					else	//Volume detail option telling if native or not
					{
						if(entry2 != nil && [entry2 isKindOfClass:[NSNumber class]])
							isNativeIfVolume = [entry2 boolValue];
						else
							isNativeIfVolume = NO;
					}

					//Actual parsing of details
					for(NSNumber * entry3 in entry1)
					{
						if([entry3 superclass] != [NSNumber class])
							continue;
						
						int value = [entry3 integerValue];
						
#ifdef DEV_VERSION
						if(value == 0xdeadbead)
						{
							//This value is used to signal a deallocated memory area, it'd crash if ran in debugger
							logR("Error: this value (-559038803) is forbiden, moved by one");
							value--;
						}
#endif
						if(isChapter)
							((int *)output)[pos++] = value;
						else
						{
							((CONTENT_TOME *) output)[pos].ID = value;
							((CONTENT_TOME *) output)[pos++].isNative = isNativeIfVolume;
						}

					}
				}
			}
			else
			{
				entry1 = objectForKey(dictionary, JSON_PROJ_CHAP_JUMP, @"jump");
				if(entry1 != nil && [entry1 superclass] == [NSNumber class])	jump = [(NSNumber*) entry1 integerValue];	else	{	continue;	}
				
				entry1 = objectForKey(dictionary, JSON_PROJ_CHAP_FIRST, @"first");
				if(entry1 != nil && [entry1 superclass] == [NSNumber class])	first = [(NSNumber*) entry1 integerValue];	else	{	continue;	}
				
				entry1 = objectForKey(dictionary, JSON_PROJ_CHAP_LAST, @"last");
				if(entry1 != nil && [entry1 superclass] == [NSNumber class])	last = [(NSNumber*) entry1 integerValue];	else	{	continue;	}
				
				sum = (last - first) / jump + 1;
				if(sum > 0)	*counter += sum;
				else		continue;
				

				
				if((tmp = realloc(output, (*counter + 1) * typeSize)) != NULL)
				{
					output = tmp;

					if(!isChapter)
					{
						//Check if native of not if volume
						if(entry2 != nil && [entry2 isKindOfClass:[NSNumber class]])
							isNativeIfVolume = [entry2 boolValue];
						else
							isNativeIfVolume = NO;

						//The first element have to be initialized early
						((CONTENT_TOME *) output)[pos].isNative = isNativeIfVolume;
						for (((CONTENT_TOME *) output)[pos++].ID = first; pos < *counter; pos++)
						{
							((CONTENT_TOME *) output)[pos].ID = ((CONTENT_TOME *) output)[pos - 1].ID + jump;
							((CONTENT_TOME *) output)[pos].isNative = isNativeIfVolume;
						}
					}
					else
					{
						for(((int *)output)[pos++] = first; pos < *counter; pos++)
							((int *)output)[pos] = ((int *)output)[pos-1] + jump;
					}
				}
				
				if(isChapter && entry2 != nil && (tmp = realloc(*chaptersPrice, *counter * sizeof(int))) != NULL)
				{
					*chaptersPrice = tmp;
					
					if([entry2 superclass] == [NSNumber class])
					{
						*chaptersPrice = tmp;
						uint price = [entry2 unsignedIntValue];
						
						while(pricePos < *counter)
							(*chaptersPrice)[pricePos++] = price;
					}
					else
						memset(&((*chaptersPrice)[pricePos]), 0, (*counter - pricePos) * sizeof(int));
				}
			}
		}
	}
	
	return output;
}

NSArray * recoverChapterStructure(void * structure, BOOL isChapter, uint * chapterPrices, uint length)
{
	if(structure == NULL || length == 0)		return nil;
	
	NSMutableArray * output = [NSMutableArray new], *currentDetail = nil, *currentBurst = nil, *pricesInBurst, *priceDetail = nil;
	BOOL currentNativeIfNotChap = NO;
	
	if(output == nil)		return nil;
	
	if(length < 6)	//No need for diffs
	{
		currentDetail = [NSMutableArray array];
		
		if(isChapter)
		{
			for(uint i = 0; i < length; [currentDetail addObject:@(((int *) structure)[i++])]);
			
			if(chapterPrices != NULL)
			{
				NSMutableArray * prices = [NSMutableArray array];
				
				for(uint i = 0; i < length; [prices addObject:@(chapterPrices[i++])]);
				
				[output addObject:[NSDictionary dictionaryWithObjects:@[currentDetail, prices] forKeys : @[JSON_PROJ_CHAP_DETAILS, JSON_PROJ_PRICE]]];
			}
			else
				[output addObject:[NSDictionary dictionaryWithObject:currentDetail forKey : JSON_PROJ_CHAP_DETAILS]];
		}
		else
		{
			currentNativeIfNotChap = ((CONTENT_TOME *) structure)[0].isNative;
			
			for(uint i = 0; i < length; [currentDetail addObject:@(((CONTENT_TOME *) structure)[i++].ID)])
			{
				if(((CONTENT_TOME *) structure)[i].isNative != currentNativeIfNotChap)
				{
					[output addObject:[NSDictionary dictionaryWithObjects:@[currentDetail, @(currentNativeIfNotChap)] forKeys : @[JSON_PROJ_CHAP_DETAILS, JSON_PROJ_VOL_ISRESERVEDTOVOL]]];
					
					currentDetail = [NSMutableArray array];
					currentNativeIfNotChap = ((CONTENT_TOME *) structure)[i].isNative;
				}
			}
			
			[output addObject:[NSDictionary dictionaryWithObjects:@[currentDetail, @(currentNativeIfNotChap)] forKeys : @[JSON_PROJ_CHAP_DETAILS, JSON_PROJ_VOL_ISRESERVEDTOVOL]]];
		}
			
		currentDetail = nil;
	}
	else
	{
		//We create a diff table
		int first;
		uint diff[length-1];
		
		if(isChapter)
		{
			first = ((int *) structure)[0];

			for(uint i = 0; i < length-1; i++)
				diff[i] = ((int *) structure)[i+1] - ((int *) structure)[i];
		}
		else
		{
			first = ((CONTENT_TOME *) structure)[0].ID;
			currentNativeIfNotChap = ((CONTENT_TOME *) structure)[0].isNative;
			
			for(uint i = 0; i < length-1; i++)
				diff[i] = ((CONTENT_TOME *) structure)[i+1].ID - ((CONTENT_TOME *) structure)[i].ID;
		}
		
		//We look for burst
		int repeatingDiff = diff[0];
		currentBurst = [NSMutableArray arrayWithObject:@(first)];
		
		bool pricesValid = isChapter && chapterPrices != NULL;
		
		if(pricesValid)
		{
			pricesInBurst = [NSMutableArray arrayWithObject:@(chapterPrices[0])];

			if(chapterPrices[0] != chapterPrices[1])
			{
				currentDetail = [NSMutableArray arrayWithObject:@(((int *) structure)[0])];
				currentBurst = [NSMutableArray new];
				priceDetail = [NSMutableArray arrayWithObject:@(chapterPrices[0])];
				pricesInBurst = [NSMutableArray new];
			}
		}
		
		for (uint pos = 1, counter = 1; pos < length; pos++)
		{
			if(pos == length-1 || diff[pos] != repeatingDiff ||
			   (pricesValid && chapterPrices[pos] != chapterPrices[pos + 1]) ||
			   (!isChapter && ((CONTENT_TOME *) structure)[pos].isNative != currentNativeIfNotChap))
			{
				if(counter > 5)
				{
					if(currentDetail != nil && [currentDetail count])
					{
						if(pricesValid)
							[output addObject:[NSDictionary dictionaryWithObjects:@[currentDetail, priceDetail] forKeys : @[JSON_PROJ_CHAP_DETAILS, JSON_PROJ_PRICE]]];
						else
							[output addObject:[NSDictionary dictionaryWithObject:currentDetail forKey : JSON_PROJ_CHAP_DETAILS]];

						currentDetail = nil;
					}
					
					//Because diff tell us how far is the next element, a != diff mean the next element break the chain, so the last one of the burst is the current one
					//However, it gets a bit tricky thanks to currentNativeState (!isChapter only, thankfully)
					if(!isChapter)
					{
						//If the native state changed, we are offseted by one, but this branch is > 5 so we don't really care beside applying the offset
						if(((CONTENT_TOME *) structure)[pos].isNative != currentNativeIfNotChap)
						{
							[output addObject:[NSDictionary dictionaryWithObjects:@[@(((CONTENT_TOME *) structure)[pos - counter].ID), @(((CONTENT_TOME *) structure)[pos - 1].ID), @(repeatingDiff)] forKeys:@[JSON_PROJ_CHAP_FIRST, JSON_PROJ_CHAP_LAST, JSON_PROJ_CHAP_JUMP]]];
							currentNativeIfNotChap = ((CONTENT_TOME *) structure)[pos--].isNative;
						}
						else
						{
							[output addObject:[NSDictionary dictionaryWithObjects:@[@(((CONTENT_TOME *) structure)[pos - counter].ID), @(((CONTENT_TOME *) structure)[pos].ID), @(repeatingDiff)] forKeys:@[JSON_PROJ_CHAP_FIRST, JSON_PROJ_CHAP_LAST, JSON_PROJ_CHAP_JUMP]]];
							currentNativeIfNotChap = ((CONTENT_TOME *) structure)[pos + 1].isNative;
						}
						
					}
					else
					{
						if(pricesValid)
							[output addObject:[NSDictionary dictionaryWithObjects:@[@(((int *) structure)[pos - counter]), @(((int *) structure)[pos]), @(repeatingDiff), @(chapterPrices[pos])] forKeys:@[JSON_PROJ_CHAP_FIRST, JSON_PROJ_CHAP_LAST, JSON_PROJ_CHAP_JUMP, JSON_PROJ_PRICE]]];
						else
							[output addObject:[NSDictionary dictionaryWithObjects:@[@(((int *) structure)[pos - counter]), @(((int *) structure)[pos]), @(repeatingDiff)] forKeys:@[JSON_PROJ_CHAP_FIRST, JSON_PROJ_CHAP_LAST, JSON_PROJ_CHAP_JUMP]]];
					}
				}
				else
				{
					if(!isChapter && ((CONTENT_TOME *) structure)[pos].isNative != currentNativeIfNotChap)
					{
						[output addObject:[NSDictionary dictionaryWithObjects:@[currentBurst, @(currentNativeIfNotChap)] forKeys : @[JSON_PROJ_CHAP_DETAILS, JSON_PROJ_VOL_ISRESERVEDTOVOL]]];
						
						currentBurst = nil;
						currentNativeIfNotChap = ((CONTENT_TOME *) structure)[pos].isNative;
					}
					else
					{
						if(pos == length - 1)
						{
							//We're comparing to the next element, so once we reach the end, and there is no element after us, we know we're good
							[currentBurst addObject:@(isChapter ? ((int *) structure)[pos] : ((CONTENT_TOME *) structure)[pos].ID)];
							if(pricesValid)
								[pricesInBurst addObject:@(chapterPrices[pos])];
						}
						
						if(currentDetail == nil)
						{
							currentDetail = [NSMutableArray new];
							
							if(pricesValid)
								priceDetail = [NSMutableArray new];
						}
						
						[currentDetail addObjectsFromArray:currentBurst];
						
						if(pricesValid)
							[priceDetail addObjectsFromArray:pricesInBurst];
					}
				}
				
				currentBurst = counter > 5 ? [NSMutableArray new] : [NSMutableArray arrayWithObject:@(isChapter ? ((int *) structure)[pos] : ((CONTENT_TOME *) structure)[pos].ID)];
				if(pricesValid)
					pricesInBurst = counter > 5 ? [NSMutableArray new] : [NSMutableArray arrayWithObject:@(chapterPrices[pos])];

				repeatingDiff = diff[pos];	counter = 1;
			}
			else
			{
				[currentBurst addObject:@(isChapter ? ((int *) structure)[pos] : ((CONTENT_TOME *) structure)[pos].ID)];
				
				if(pricesValid)
					[pricesInBurst addObject:@(chapterPrices[pos+1])];
				
				counter++;
			}
		}
		
		if(currentDetail != nil && [currentDetail count])
		{
			if(pricesValid)
				[output addObject:[NSDictionary dictionaryWithObjects:@[currentDetail, pricesInBurst] forKeys : @[JSON_PROJ_CHAP_DETAILS, JSON_PROJ_PRICE]]];
			else if(isChapter)
				[output addObject:[NSDictionary dictionaryWithObject:currentDetail forKey : JSON_PROJ_CHAP_DETAILS]];
			else
				[output addObject:[NSDictionary dictionaryWithObjects:@[currentBurst, @(currentNativeIfNotChap)] forKeys : @[JSON_PROJ_CHAP_DETAILS, JSON_PROJ_VOL_ISRESERVEDTOVOL]]];
		}
	}
	
	return [NSArray arrayWithArray:output];
}

META_TOME * getVolumes(NSArray* volumeBloc, uint * nbElem, BOOL paidContent)
{
	if(nbElem == NULL)
		return NULL;
	else
		*nbElem = 0;
	
	if(volumeBloc == nil || ![volumeBloc isKindOfClass:[NSArray class]])
		return NULL;
	
	size_t nbElemMax = [volumeBloc count];
	META_TOME * output = malloc(nbElemMax * sizeof(META_TOME));

	if(output != NULL)
	{
		uint cache = 0;
		NSArray * content;
		NSDictionary * dict;
		NSString *description, *readingName;
		NSNumber *readingID, *internalID, *priceObj;

		for(dict in volumeBloc)
		{
			if([dict superclass] != [NSMutableDictionary class])					continue;
			
			readingName = objectForKey(dict, JSON_PROJ_VOL_READING_NAME, @"Reading name");
			if (readingName == nil || [readingName superclass] != [NSMutableString class] || [readingName length] == 0)
			{
				readingName = nil;
				readingID = objectForKey(dict, JSON_PROJ_VOL_READING_ID, @"Reading ID");
				if(readingID == nil || [readingID superclass] != [NSNumber class])	continue;
			}
			else
				readingID = nil;
			
			internalID = objectForKey(dict, JSON_PROJ_VOL_INTERNAL_ID, @"Internal ID");
			if(internalID == nil || [internalID superclass] != [NSNumber class])	continue;

			description = objectForKey(dict, JSON_PROJ_VOL_DESCRIPTION, @"Description");
			
			content = objectForKey(dict, JSON_PROJ_CHAPTERS, @"chapters");
			if(content == nil || ![content isKindOfClass:[NSArray class]])	continue;
			
			if(paidContent)
				priceObj = objectForKey(dict, JSON_PROJ_PRICE, @"price");
			
			output[cache].details = parseChapterStructure(content, &(output[cache].lengthDetails), NO, NO, NULL);
			
			if(output[cache].details == NULL)
				continue;
			
			output[cache].ID = [internalID intValue];
			output[cache].readingID = readingID == nil ? VALEUR_FIN_STRUCT : [readingID intValue];
			
			if(readingName == nil)			output[cache].readingName[0] = 0;
			else							wcsncpy(output[cache].readingName, (charType*) [readingName cStringUsingEncoding:NSUTF32StringEncoding], MAX_TOME_NAME_LENGTH);
			
			if(description == nil)			output[cache].description[0] = 0;
			else							wcsncpy(output[cache].description, (charType*) [description cStringUsingEncoding:NSUTF32StringEncoding], TOME_DESCRIPTION_LENGTH);
			
			if(priceObj == nil || ![priceObj isKindOfClass:[NSNumber class]])	output[cache].price = UINT_MAX;
			else							output[cache].price = [priceObj unsignedIntValue];
			
			cache++;
		}
		
		*nbElem = cache;
	}
	
	return output;
}

NSArray * recoverVolumeBloc(META_TOME * volume, uint length, BOOL paidContent)
{
	if (volume == NULL)
		return nil;
	
	NSMutableArray *output = [NSMutableArray array];
	NSMutableDictionary * dict;
	
	for(uint pos = 0; pos < length; pos++)
	{
		if(volume[pos].ID == VALEUR_FIN_STRUCT)
			break;
		else if(volume[pos].readingID == VALEUR_FIN_STRUCT && volume[pos].readingName[0] == 0)
			continue;
		
		dict = [NSMutableDictionary dictionaryWithObject:@(volume[pos].ID) forKey:JSON_PROJ_VOL_INTERNAL_ID];
		
		if(volume[pos].description[0])
			[dict setObject:getStringForWchar(volume[pos].description) forKey:JSON_PROJ_VOL_DESCRIPTION];
		
		if(volume[pos].readingName[0])
			[dict setObject:getStringForWchar(volume[pos].readingName) forKey:JSON_PROJ_VOL_READING_NAME];
		
		if(volume[pos].readingID != VALEUR_FIN_STRUCT)
			[dict setObject:@(volume[pos].readingID) forKey:JSON_PROJ_VOL_READING_ID];
		
		if(paidContent && volume[pos].price != UINT_MAX)
			[dict setObject:@(volume[pos].price) forKey:JSON_PROJ_PRICE];
		
		if(volume->details != NULL)
		{
			NSArray * data = recoverChapterStructure(volume->details, NO, NULL, volume->lengthDetails);
			if(data != nil)
				[dict setObject:data forKey:JSON_PROJ_CHAPTERS];
		}
		
		[output addObject:dict];
	}
	
	return [NSArray arrayWithArray:output];
}

PROJECT_DATA parseBloc(NSDictionary * bloc)
{
	PROJECT_DATA data = getEmptyProject();
	
	int * chapters = NULL;
	uint * chaptersPrices = NULL;
	META_TOME * volumes = NULL;
	
	//We create all variable first, otherwise ARC complain
	NSNumber *ID, *status = nil, *type = nil, *asianOrder = nil, *tag = nil, *paidContent = nil, *DRM = nil;
	NSString * projectName = nil, *description = nil, *authors = nil;
	
	ID = objectForKey(bloc, JSON_PROJ_ID, @"ID");
	if (ID == nil || [ID superclass] != [NSNumber class])					goto end;
	
	projectName = objectForKey(bloc, JSON_PROJ_PROJECT_NAME, @"projectName");
	if([projectName superclass] != [NSMutableString class])					goto end;
	
	DRM = objectForKey(bloc, JSON_PROJ_DRM, nil);
	if(DRM != nil && [DRM superclass] != [NSNumber class])					goto end;
	
	paidContent = objectForKey(bloc, JSON_PROJ_PRICE, @"price");
	if(paidContent != nil && [paidContent superclass] != [NSNumber class])	goto end;
		 
	BOOL isPaidContent = paidContent == nil ? NO : [paidContent boolValue];
	
	uint nbChapters = 0, nbVolumes = 0;
	chapters = parseChapterStructure(objectForKey(bloc, JSON_PROJ_CHAPTERS, @"chapters"), &nbChapters, YES, isPaidContent, &chaptersPrices);
	volumes = getVolumes(objectForKey(bloc, JSON_PROJ_VOLUMES, @"volumes"), &nbVolumes, isPaidContent);

	if(nbChapters == 0 && nbVolumes == 0)									goto end;
	
	description = objectForKey(bloc, JSON_PROJ_DESCRIPTION, @"description");
	if(description == nil || [description superclass] != [NSMutableString class])	description = nil;
	
	authors = objectForKey(bloc, JSON_PROJ_AUTHOR , @"author");
	if(authors == nil || [authors superclass] != [NSMutableString class])			goto end;
	
	status = objectForKey(bloc, JSON_PROJ_STATUS , @"status");
	if(status == nil || [status superclass] != [NSNumber class])			goto end;
	
	type = objectForKey(bloc, JSON_PROJ_TYPE , @"type");
	if(type == nil || [type superclass] != [NSNumber class])				goto end;
	
	asianOrder = objectForKey(bloc, JSON_PROJ_ASIAN_ORDER , @"asian_order_of_reading");
	if(asianOrder == nil || [asianOrder superclass] != [NSNumber class])	goto end;
	
	tag = objectForKey(bloc, JSON_PROJ_TAG , @"category");
	if(tag == nil || [tag superclass] != [NSNumber class])		goto end;
	
	data.projectID = [ID unsignedIntValue];
	data.isPaid = isPaidContent;
	data.chapitresPrix = chaptersPrices;
	data.chapitresFull = chapters;		data.chapitresInstalled = NULL;		data.nombreChapitre = nbChapters;	data.nombreChapitreInstalled = 0;
	data.tomesFull = volumes;			data.tomesInstalled = NULL;			data.nombreTomes = nbVolumes;		data.nombreTomesInstalled = 0;
	data.status = [status unsignedCharValue];
	if(data.status > STATUS_MAX)	data.status = STATUS_MAX;
	data.type = [type unsignedIntValue];
	if(data.type > TYPE_MAX)		data.type = TYPE_MAX;
	data.tag = [tag unsignedIntValue];
	data.haveDRM = (DRM != nil && [DRM boolValue]) | (DRM == nil && isPaidContent);
	data.isInitialized = true;
	
	wcsncpy(data.projectName, (charType*) [projectName cStringUsingEncoding:NSUTF32StringEncoding], LENGTH_PROJECT_NAME);
	wcsncpy(data.authorName, (charType*) [authors cStringUsingEncoding:NSUTF32StringEncoding], LENGTH_AUTHORS);
	
	if(description != nil)
	{
		wcsncpy(data.description, (charType*) [description cStringUsingEncoding:NSUTF32StringEncoding], [description length]);
		data.description[LENGTH_DESCRIPTION-1] = 0;
	}
	else
		memset(&data.description, 0, sizeof(data.description));
	
	chapters = NULL;
	chaptersPrices = NULL;
	volumes = NULL;
end:
	
	free(chapters);
	free(chaptersPrices);
	freeTomeList(volumes, true);
	return data;
}

NSDictionary * reverseParseBloc(PROJECT_DATA project)
{
	if(project.repo == NULL)
		return nil;
	
	id buf;
	NSMutableDictionary * output = [NSMutableDictionary dictionary];
	
	[output setObject:@(project.projectID) forKey:JSON_PROJ_ID];
	[output setObject:getStringForWchar(project.projectName) forKey:JSON_PROJ_PROJECT_NAME];
	
	buf = recoverChapterStructure(project.chapitresFull, YES, project.chapitresPrix, project.nombreChapitre);
	if(buf != nil)		[output setObject:buf forKey:JSON_PROJ_CHAPTERS];
	
	buf = recoverVolumeBloc(project.tomesFull, project.nombreTomes, project.isPaid);
	if(buf != nil)		[output setObject:buf forKey:JSON_PROJ_VOLUMES];
	
	if(project.description[0])
		[output setObject:getStringForWchar(project.description) forKey:JSON_PROJ_DESCRIPTION];
	
	if(project.authorName[0])
		[output setObject:getStringForWchar(project.authorName) forKey:JSON_PROJ_AUTHOR];
	
	[output setObject:@(project.status) forKey:JSON_PROJ_STATUS];
	[output setObject:@(project.type) forKey:JSON_PROJ_TYPE];
	[output setObject:@(project.japaneseOrder) forKey:JSON_PROJ_ASIAN_ORDER];
	[output setObject:@(project.tag) forKey:JSON_PROJ_TAG];
	[output setObject:@(project.haveDRM) forKey:JSON_PROJ_DRM];
	
	if(project.isPaid)
		[output setObject:@(YES) forKey:JSON_PROJ_PRICE];
	
	return [NSDictionary dictionaryWithDictionary:output];
}

PROJECT_DATA_EXTRA parseBlocExtra(NSDictionary * bloc)
{
	PROJECT_DATA_EXTRA output;
	PROJECT_DATA shortData = parseBloc(bloc);
	
	if(shortData.isInitialized)
	{
		memcpy(&output, &shortData, sizeof(shortData));
		
		NSString * URL, * CRC;
		NSArray * IDURL = @[JSON_PROJ_URL_SRGRID, JSON_PROJ_URL_SRGRID_2X, JSON_PROJ_URL_HEAD, JSON_PROJ_URL_HEAD_2X, JSON_PROJ_URL_CT, JSON_PROJ_URL_CT_2X, JSON_PROJ_URL_DD, JSON_PROJ_URL_DD_2X], * IDHash = @[JSON_PROJ_HASH_SRGRID, JSON_PROJ_HASH_SRGRID_2X, JSON_PROJ_HASH_HEAD, JSON_PROJ_HASH_HEAD_2X, JSON_PROJ_HASH_CT, JSON_PROJ_HASH_CT_2X, JSON_PROJ_HASH_DD, JSON_PROJ_HASH_DD_2X];

		for(byte i = 0; i < NB_IMAGES; i++)
		{
			URL = objectForKey(bloc, IDURL[i], nil);
			CRC = objectForKey(bloc, IDHash[i], nil);
			
			if(URL == nil || CRC == nil)
				output.haveImages[i] = false;
			else
			{
				char * URLCopy = strdup([URL cStringUsingEncoding:NSASCIIStringEncoding]);
				
				if(URLCopy != NULL)
				{
					strncpy((void*) &(output.hashesImages[i]), [CRC cStringUsingEncoding:NSASCIIStringEncoding], LENGTH_HASH);
					output.URLImages[i] = URLCopy;
					output.haveImages[i] = true;
				}
				else
				{
					output.haveImages[i] = false;
				}
			}
		}
	}
	else
		output.isInitialized = false;
	
	return output;
}

void* parseProjectJSON(REPO_DATA* repo, NSDictionary * remoteData, uint * nbElem, bool parseExtra)
{
	void * outputData = NULL;
	bool isInit;
	NSArray * projects = objectForKey(remoteData, JSON_PROJ_PROJECTS, @"projects");
	
	if(projects == nil || [projects superclass] != [NSArray class])
		return NULL;
	
	size_t size = [projects count];
	outputData = malloc(size * (parseExtra ? sizeof(PROJECT_DATA_EXTRA) : sizeof(PROJECT_DATA)));
	
	if(outputData != NULL)
	{
		size_t validElements = 0;
		
		for (remoteData in projects)
		{
			if(validElements >= size)
				break;
			else if([remoteData superclass] != [NSMutableDictionary class])
				continue;
			
			if(parseExtra)
			{
				((PROJECT_DATA_EXTRA*)outputData)[validElements] = parseBlocExtra(remoteData);
				isInit = ((PROJECT_DATA_EXTRA*)outputData)[validElements].isInitialized;
			}
			else
			{
				((PROJECT_DATA*)outputData)[validElements] = parseBloc(remoteData);
				isInit = ((PROJECT_DATA*)outputData)[validElements].isInitialized;
			}
		
			if(isInit)
			{
				PROJECT_DATA * project;
				
				if(parseExtra)
					project = (PROJECT_DATA*) &((PROJECT_DATA_EXTRA*)outputData)[validElements++];
				else
					project = &((PROJECT_DATA*)outputData)[validElements++];
				
				project->repo = repo;
			}
		}
		
		if(nbElem != NULL)
			*nbElem = validElements;
	}
	
	return outputData;
}

PROJECT_DATA_EXTRA * parseRemoteData(REPO_DATA* repo, char * remoteDataRaw, uint * nbElem)
{
	NSError * error = nil;
	NSMutableDictionary * remoteData = [NSJSONSerialization JSONObjectWithData:[NSData dataWithBytes:remoteDataRaw length:ustrlen(remoteDataRaw)] options:0 error:&error];
	
	if(error != nil || remoteData == nil || [remoteData superclass] != [NSMutableDictionary class])
	{
		if(error != nil)
			NSLog(@"%@", error.description);
		else
			NSLog(@"Parse error when analysing remote project file for %@", getStringForWchar(repo->name));
		return NULL;
	}
	
	id repoID = objectForKey(remoteData, JSON_PROJ_AUTHOR_ID, @"authorID");
	if(repoID == nil || ![repoID isKindOfClass:[NSNumber class]] || [(NSNumber*) repoID unsignedLongLongValue] != repo->repoID)
	{
		NSLog(@"Invalid remote project file for %@: incorrect repo ID", getStringForWchar(repo->name));
		return NULL;
	}
	
	return parseProjectJSON(repo, remoteData, nbElem, true);
}

PROJECT_DATA * parseLocalData(REPO_DATA ** repo, uint nbRepo, unsigned char * remoteDataRaw, uint *nbElem)
{
	if(repo == NULL || nbElem == NULL)
		return NULL;
	
	NSError * error = nil;
	NSMutableDictionary * remoteData = [NSJSONSerialization JSONObjectWithData:[NSData dataWithBytes:remoteDataRaw length:ustrlen(remoteDataRaw)] options:0 error:&error];
	
	if(error != nil || remoteData == nil || [remoteData superclass] != [NSArray class])
		return NULL;
	
	uint nbElemPart, posRepo;
	PROJECT_DATA *output = NULL, *currentPart;
	id repoID;
	
	for(NSDictionary * remoteDataPart in remoteData)
	{
		if([remoteDataPart superclass] != [NSMutableDictionary class])
			continue;
		
		repoID = objectForKey(remoteDataPart, JSON_PROJ_AUTHOR_ID, @"authorID");
		if (repoID == nil || ![repoID isKindOfClass : [NSNumber class]])
			continue;
		
		for(posRepo = 0; posRepo < nbRepo; posRepo++)
		{
			if(repo[posRepo] == NULL)
				continue;
			
			if([(NSNumber*) repoID unsignedLongLongValue] == getRepoID(repo[posRepo]))
			{
				nbElemPart = 0;
				currentPart = (PROJECT_DATA*) parseProjectJSON(repo[posRepo], remoteDataPart, &nbElemPart, false);
				
				if(nbElemPart)
				{
					void * buf = realloc(output, (*nbElem + nbElemPart) * sizeof(PROJECT_DATA));
					if(buf != NULL)
					{
						output = buf;
						memcpy(&output[*nbElem], currentPart, nbElemPart * sizeof(PROJECT_DATA));
						*nbElem += nbElemPart;
					}
				}
				free(currentPart);
				break;
			}
		}
	}
	
	return output;
}

char * reversedParseData(PROJECT_DATA * data, uint nbElem, REPO_DATA ** repo, uint nbRepo, size_t * sizeOutput)
{
	if(data == NULL || repo == NULL || !nbElem || !nbRepo)
		return NULL;
	
	uint counters[nbRepo], jumpTable[nbRepo][nbElem];
	bool projectLinkedToRepo = false;
	
	memset(counters, 0, sizeof(counters));
	
	//Create a table linking projects to a repo
	for(uint pos = 0, posRepo; pos < nbElem; pos++)
	{
		uint64_t repoID = getRepoID(data[pos].repo);
		
		for(posRepo = 0; posRepo < nbRepo; posRepo++)
		{
			if(repoID == getRepoID(repo[posRepo]))
			{
				jumpTable[posRepo][counters[posRepo]++] = pos;
				projectLinkedToRepo = true;
				break;
			}
		}
	}
	
	if(!projectLinkedToRepo)
		return NULL;
	
	NSMutableArray *root = [NSMutableArray array], *projects;
	NSDictionary * currentNode;
	id currentProject;
	
	for(uint pos = 0; pos < nbRepo; pos++)
	{
		if(!counters[pos])	continue;
		
		projects = [NSMutableArray array];
		
		for(uint index = 0; index < counters[pos]; index++)
		{
			currentProject = reverseParseBloc(data[jumpTable[pos][index]]);
			if(currentProject != nil)
				[projects addObject:currentProject];
		}
		
		if([projects count])
		{
			currentNode = [NSDictionary dictionaryWithObjects:@[@(getRepoID(repo[pos])), projects] forKeys:@[JSON_PROJ_AUTHOR_ID, JSON_PROJ_PROJECTS]];
			if(currentNode != nil)
				[root addObject:currentNode];
		}
	}
	
	if(![root count])
		return NULL;
	
	NSError * error = nil;
	NSData * dataOutput = [NSJSONSerialization dataWithJSONObject:root options:0 error:&error];
	
	size_t length = [dataOutput length];
	void * outputDataC = malloc(length);

	if (dataOutput == NULL)
		return NULL;
	
	[dataOutput getBytes:outputDataC length:length];
	
	char * output = base64_encode(outputDataC, length, sizeOutput);
	
	free(outputDataC);
	
	return output;
}

#pragma mark - Toolbox

id objectForKey(NSDictionary * dict, NSString * ID, NSString * fullName)
{
	id value = [dict objectForKey : ID];
	
	if(value == nil && fullName != nil)
	{
		value = [dict objectForKey:fullName];
	}
	
	return value;
}

void moveProjectExtraToStandard(const PROJECT_DATA_EXTRA input, PROJECT_DATA * output)
{
	if(!input.isInitialized || output == NULL)
		return;
	
	memcpy(output, &input, sizeof(PROJECT_DATA));
}
