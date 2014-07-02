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

typedef struct dataProject
{
	//Pointeurs, un bloc chacun (64b)
	TEAMS_DATA *team;
	int *chapitresFull;
	int *chapitresInstalled;
	META_TOME *tomesFull;
	META_TOME *tomesInstalled;
	
	//Un bloc de 64b complet chacun
	size_t nombreChapitre;
	size_t nombreChapitreInstalled;
	size_t nombreTomes;
	size_t nombreTomesInstalled;
	
	wchar_t description[LENGTH_DESCRIPTION];
	
	//2 x ((51 + 1) x 32b) = 52 x 64b
	wchar_t projectName[LENGTH_PROJECT_NAME];
	bool contentDownloadable;
	wchar_t authorName[LENGTH_AUTHORS];
	bool favoris;
	
	//Un bloc de 64b complet
	uint8_t status;
	uint8_t type;
	uint16_t category;
	bool japaneseOrder;
	
	//Un bloc de 64b
	uint32_t projectID;
	uint32_t cacheDBID;
	
} PROJECT_DATA;

typedef struct dataProjectWithExtra
{
	//Pointeurs, un bloc chacun (64b)
	TEAMS_DATA *team;
	int *chapitresFull;
	int *chapitresInstalled;
	META_TOME *tomesFull;
	META_TOME *tomesInstalled;
	
	//Un bloc de 64b complet chacun
	size_t nombreChapitre;
	size_t nombreChapitreInstalled;
	size_t nombreTomes;
	size_t nombreTomesInstalled;
	
	wchar_t description[LENGTH_DESCRIPTION];
	
	//2 x ((51 + 1) x 32b) = 52 x 64b
	wchar_t projectName[LENGTH_PROJECT_NAME];
	bool contentDownloadable;
	wchar_t authorName[LENGTH_AUTHORS];
	bool favoris;
	
	//Un bloc de 64b complet
	uint8_t status;
	uint8_t type;
	uint16_t category;
	bool japaneseOrder;
	
	//Un bloc de 64b
	uint32_t projectID;
	uint32_t cacheDBID;
	
	//2 x 64b
	char hashLarge[LENGTH_HASH];
	char hashSmall[LENGTH_HASH];
	
	//2 x 256o
	char URLLarge[LENGTH_URL];
	char URLSmall[LENGTH_URL];
				
} PROJECT_DATA_EXTRA;

int * getChapters(NSArray * chapterBloc, uint * nbElem);
META_TOME * getVolumes(NSArray* volumeBloc, uint * nbElem);
PROJECT_DATA parseBloc(NSDictionary * bloc);
PROJECT_DATA_EXTRA parseBlocExtra(NSDictionary * bloc);
PROJECT_DATA_EXTRA * parseRemoteData(TEAMS_DATA* team, char * remoteDataRaw);

int * getChapters(NSArray * chapterBloc, uint * nbElem)
{
	if(nbElem != NULL)
		*nbElem = 0;
	
	if (chapterBloc == NULL)
		return NULL;
	
	int * output = NULL;
	uint counterVar = 0, *counter, pos = 0;
	size_t nbSubBloc = [chapterBloc count];
	
	if (nbSubBloc == 0)
	{
		output = malloc(sizeof(int));
		if (output != nil)
			output[0] = VALEUR_FIN_STRUCT;
	}
	else
	{
		id entry1;
		int jump, first, last, sum;
		void* tmp;
		
		if (nbElem != NULL)		counter = nbElem;
		else					counter = &counterVar;
		
		for(NSDictionary * dictionary in chapterBloc)
		{
			if([dictionary superclass] != [NSMutableDictionary class])	continue;
			
			entry1 = [dictionary objectForKey:@"details"];
			if(entry1 != nil && [(NSObject*)entry1 superclass] == [NSArray class])	//This is a special chunck
			{
				*counter += [(NSArray*) entry1 count];
				
				if(!*counter)
					continue;
				
				else if((tmp = realloc(output, (*counter + 1) * sizeof(int))) != NULL)
				{
					output = tmp;
					
					for(NSNumber * entry2 in entry1)
					{
						if([entry2 superclass] != [NSNumber class])
							continue;
						
						output[pos++] = [entry2 integerValue];
					}
					
					output[pos] = VALEUR_FIN_STRUCT;
				}
			}
			else
			{
				entry1 = [dictionary objectForKey:@"jump"];
				if(entry1 != nil && [entry1 superclass] == [NSNumber class])	jump = [(NSNumber*) entry1 integerValue];	else	{	continue;	}
				
				entry1 = [dictionary objectForKey:@"first"];
				if(entry1 != nil && [entry1 superclass] == [NSNumber class])	first = [(NSNumber*) entry1 integerValue];	else	{	continue;	}
				
				entry1 = [dictionary objectForKey:@"last"];
				if(entry1 != nil && [entry1 superclass] == [NSNumber class])	last = [(NSNumber*) entry1 integerValue];	else	{	continue;	}
				
				sum = (last - first) / jump + 1;
				if(sum > 0)	*counter += sum;
				else		continue;
				
				if((tmp = realloc(output, (*counter + 1) * sizeof(int))) != NULL)
				{
					output = tmp;
					for (output[pos++] = first; pos < *counter; pos++) {	output[pos] = output[pos-1] + jump;		}
					output[pos] = VALEUR_FIN_STRUCT;
				}
			}
		}
	}
	
	return output;
}

META_TOME * getVolumes(NSArray* volumeBloc, uint * nbElem)
{
	if(nbElem == NULL)
		return NULL;
	else
		*nbElem = 0;
	
	if(volumeBloc == nil || [volumeBloc superclass] != [NSArray class])
		return NULL;
	
	size_t nbElemMax = [volumeBloc count];
	META_TOME * output = malloc((nbElemMax + 1) * sizeof(META_TOME));

	if(output != NULL)
	{
		uint cache = 0;
		NSDictionary * dict;
		NSString *description, *readingName;
		NSNumber *readingID, *internalID;

		for(dict in volumeBloc)
		{
			if([dict superclass] != [NSMutableDictionary class])					continue;
			
			readingName = [dict objectForKey:@"Reading name"];
			if (readingName == nil || [readingName superclass] != [NSMutableString class] || [readingName length] == 0)
			{
				readingName = nil;
				readingID = [dict objectForKey:@"Reading ID"];
				if(readingID == nil || [readingID superclass] != [NSNumber class])	continue;
			}
			else
				readingID = nil;
			
			internalID = [dict objectForKey:@"Internal ID"];
			if(internalID == nil || [internalID superclass] != [NSNumber class])	continue;

			(*nbElem)++;
			
			description = [dict objectForKey:@"Description"];
			
			output[cache].ID = [internalID intValue];
			output[cache].readingID = readingID == nil ? VALEUR_FIN_STRUCT : [readingID intValue];
			
			if(readingName == nil)			output[cache].readingName[0] = 0;
			else							wcsncpy(output[cache].readingName, (wchar_t*) [readingName cStringUsingEncoding:NSUTF32StringEncoding], MAX_TOME_NAME_LENGTH);
			
			if(description == nil)			output[cache].description[0] = 0;
			else							wcsncpy(output[cache].description, (wchar_t*) [description cStringUsingEncoding:NSUTF32StringEncoding], TOME_DESCRIPTION_LENGTH);
			
#warning "Need to link this code"
			output[cache].details = NULL;
		}
		
		output[*nbElem].ID = VALEUR_FIN_STRUCT;
	}
	
	return output;
}

PROJECT_DATA parseBloc(NSDictionary * bloc)
{
	PROJECT_DATA data;
	memset(&data, 0, sizeof(data));
	
	int * chapters = NULL;
	META_TOME * volumes = NULL;
	
	NSNumber * ID = [bloc objectForKey:@"ID"];
	if (ID == nil || [ID superclass] != [NSNumber class])					goto end;
	
	NSString * projectName = [bloc objectForKey:@"projectName"];
	if([projectName superclass] != [NSMutableString class])					goto end;
	
	uint nbChapters, nbVolumes;
	chapters = getChapters([bloc objectForKey:@"chapters"], &nbChapters);
	volumes = getVolumes([bloc objectForKey:@"volumes"], &nbVolumes);

	if(nbChapters == 0 && nbVolumes == 0)									goto end;
	
	NSString * description = [bloc objectForKey:@"description"];
	if(description == nil || [description superclass] != [NSMutableString class])	goto end;
	
	NSString * authors = [bloc objectForKey:@"author"];
	if(authors == nil || [authors superclass] != [NSMutableString class])			goto end;
	
	NSNumber * status = [bloc objectForKey:@"status"];
	if(status == nil || [status superclass] != [NSNumber class])			goto end;
	
	NSNumber * type = [bloc objectForKey:@"type"];
	if(type == nil || [type superclass] != [NSNumber class])				goto end;
	
	NSNumber * asianOrder = [bloc objectForKey:@"asian_order_of_lecture"];
	if(asianOrder == nil || [asianOrder superclass] != [NSNumber class])	goto end;
	
	NSNumber * category = [bloc objectForKey:@"category"];
	if(category == nil || [category superclass] != [NSNumber class])		goto end;
	
	data.projectID = [ID unsignedIntValue];
	data.chapitresFull = chapters;		data.chapitresInstalled = NULL;		data.nombreChapitre = nbChapters;	data.nombreChapitreInstalled = 0;
	data.tomesFull = volumes;			data.tomesInstalled = NULL;			data.nombreTomes = nbVolumes;		data.nombreTomesInstalled = 0;
	data.status = [status unsignedCharValue];
	if(data.status > STATUS_MAX)	data.status = STATUS_MAX;
	data.type = [type unsignedCharValue];
	if(data.type > TYPE_MAX)		data.type = TYPE_MAX;
	data.category = [category unsignedIntValue];
	
	wcsncpy(data.projectName, (wchar_t*) [projectName cStringUsingEncoding:NSUTF32StringEncoding], LENGTH_PROJECT_NAME);
	wcsncpy(data.description, (wchar_t*) [description cStringUsingEncoding:NSUTF32StringEncoding], LENGTH_DESCRIPTION);
	wcsncpy(data.authorName, (wchar_t*) [authors cStringUsingEncoding:NSUTF32StringEncoding], LENGTH_AUTHORS);
	
	chapters = NULL;
	volumes = NULL;
end:
	
	free(chapters);
	freeTomeList(volumes, true);
	return data;
}

PROJECT_DATA_EXTRA parseBlocExtra(NSDictionary * bloc)
{
	PROJECT_DATA_EXTRA output;
	PROJECT_DATA shortData = parseBloc(bloc), empty;
	memset(&empty, 0, sizeof(empty));
	
	if(memcmp(&shortData, &empty, sizeof(PROJECT_DATA)))
	{
		memcpy(&output, &shortData, sizeof(shortData));
		
		NSString * URLLarge = [bloc objectForKey:@"URL_large_pic"];
		if(URLLarge != nil && [URLLarge superclass] == [NSMutableString class])
			strncpy(output.URLLarge, [URLLarge cStringUsingEncoding:NSASCIIStringEncoding], LENGTH_URL);
		else
			memset(output.URLLarge, 0, LENGTH_URL);
		
		NSString * crcLarge = [bloc objectForKey:@"hash_URL_large_pic"];
		if(crcLarge != nil && [crcLarge superclass] == [NSMutableString class])
			strncpy(output.hashLarge, [crcLarge cStringUsingEncoding:NSASCIIStringEncoding], LENGTH_HASH);
		else
			memset(output.hashLarge, 0, LENGTH_HASH);
		
		NSString * URLSmall = [bloc objectForKey:@"URL_small_pic"];
		if(URLSmall != nil && [URLSmall superclass] == [NSMutableString class])
			strncpy(output.URLSmall, [URLSmall cStringUsingEncoding:NSASCIIStringEncoding], LENGTH_URL);
		else
			memset(output.URLSmall, 0, LENGTH_URL);
		
		NSString * crcSmall = [bloc objectForKey:@"hash_URL_small_pic"];
		if(crcSmall != nil && [crcSmall superclass] == [NSMutableString class])
			strncpy(output.hashSmall, [crcSmall cStringUsingEncoding:NSASCIIStringEncoding], LENGTH_HASH);
		else
			memset(output.hashSmall, 0, LENGTH_HASH);
	}
	else
		memset(&output, 0, sizeof(output));
	
	return output;
}

PROJECT_DATA_EXTRA * parseRemoteData(TEAMS_DATA* team, char * remoteDataRaw)
{
	PROJECT_DATA_EXTRA * outputData = NULL;
	NSError * error = nil;
	NSMutableDictionary * remoteData = [[NSJSONSerialization JSONObjectWithData:[NSData dataWithBytes:remoteDataRaw length:ustrlen(remoteDataRaw)] options:0 error:&error] autorelease];
	
	if(error != nil || remoteData == nil || [remoteData superclass] != [NSMutableDictionary class])
		return NULL;
	
	id teamName = [remoteData objectForKey:@"teamName"], teamURL = [remoteData objectForKey:@"teamURL"];
	NSArray * projects = [remoteData objectForKey:@"projects"];
	
	if (teamName == nil || [teamName superclass] != [NSMutableString class] || ![(NSString*) teamName isEqualToString:[NSString stringWithUTF8String:team->teamLong]] ||
		teamURL == nil || [teamURL superclass] != [NSMutableString class] || ![(NSString*) teamURL isEqualToString:[NSString stringWithUTF8String:team->URLRepo]] ||
		projects == nil || [projects superclass] != [NSArray class])
	{	return NULL;	}
	
	size_t size = [projects count];
	outputData = malloc(size * sizeof(PROJECT_DATA_EXTRA));
	
	if (outputData != NULL)
	{
		size_t validElements = 0;
		PROJECT_DATA emptySample;
		memset(&emptySample, 0, sizeof(emptySample));
		
		for (remoteData in projects)
		{
			if(validElements >= size)
				break;
			else if([remoteData superclass] != [NSMutableDictionary class])
				continue;
			
			outputData[validElements] = parseBlocExtra(remoteData);
		
			if(memcmp(&(outputData[validElements]), &emptySample, sizeof(emptySample)))
				outputData[validElements++].team = team;
		}
	}
	
	return outputData;
}

NSArray * recoverChapterBloc(int * chapter, uint length)
{
	if(chapter == NULL)		return nil;
	
	NSMutableArray * output = [[NSMutableArray new] autorelease], *currentDetail = nil, *currentBurst = nil;
	
	if(output == nil)		return nil;
	
	//We create a diff table
	uint diff[length-1];
	for(uint i = 0; i < length-1; i++)
		diff[i] = chapter[i+1] - chapter[i];
	
	//We look for burst
	int repeatingDiff = diff[0];
	currentBurst = [NSMutableArray arrayWithObject:@(chapter[0])];
	uint counter = 1;
	
	//1 because the n-1 list in started after the first element
	for (uint i = 1; i < length; i++)
	{
		if(i == length-1 || diff[i] != repeatingDiff)
		{
			if(counter > 5)
			{
				if(currentDetail != nil && [currentDetail count])
				{
					[output addObject:currentDetail];
					[currentDetail release];		currentDetail = nil;
				}
				
				[output addObject:[NSDictionary dictionaryWithObjects:@[@(chapter[i - counter]), @(chapter[i]), @(repeatingDiff)] forKeys:@[@"first", @"last", @"jump"]]];
			}
			else
			{
				if(i == length - 1)		[currentBurst addObject:@(chapter[i])];
				
				if(currentDetail == nil)		currentDetail = [NSMutableArray new];
				
				[currentDetail addObjectsFromArray:currentBurst];
			}

			[currentBurst release];		currentBurst = counter > 5 ? [NSMutableArray new] : [NSMutableArray arrayWithObject:@(chapter[i])];
			repeatingDiff = diff[i];	counter = 1;
		}
		else
		{
			[currentBurst addObject:@(chapter[i])];
			counter++;
		}
	}
	
	if(currentDetail != nil && [currentDetail count])
	{
		[output addObject:currentDetail];
		[currentDetail release];
	}
	if(currentBurst != nil)
		[currentBurst release];
	
	return [NSArray arrayWithArray:output];
}
