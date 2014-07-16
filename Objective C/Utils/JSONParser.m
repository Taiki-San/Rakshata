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

int * getChapters(NSArray * chapterBloc, uint * nbElem);
NSArray * recoverChapterBloc(int * chapter, uint length);
META_TOME * getVolumes(NSArray* volumeBloc, uint * nbElem);
NSArray * recoverVolumeBloc(META_TOME * volume, uint length);
PROJECT_DATA parseBloc(NSDictionary * bloc);
PROJECT_DATA_EXTRA parseBlocExtra(NSDictionary * bloc);
void* parseJSON(TEAMS_DATA* team, NSDictionary * remoteData, uint * nbElem, bool parseExtra);

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

NSArray * recoverChapterBloc(int * chapter, uint length)
{
	if(chapter == NULL)		return nil;
	
	NSMutableArray * output = [NSMutableArray new], *currentDetail = nil, *currentBurst = nil;
	
	if(output == nil)		return nil;
	
	if(length < 6)	//No need for diffs
	{
		currentDetail = [NSMutableArray array];
		
		for(uint i = 0; i < length; [currentDetail addObject:@(chapter[i++])]);
		
		[output addObject:[NSDictionary dictionaryWithObject:currentDetail forKey:@"details"]];
		currentDetail = nil;
	}
	else
	{
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
						currentDetail = nil;
					}
					
					[output addObject:[NSDictionary dictionaryWithObjects:@[@(chapter[i - counter]), @(chapter[i]), @(repeatingDiff)] forKeys:@[@"first", @"last", @"jump"]]];
				}
				else
				{
					if(i == length - 1)		[currentBurst addObject:@(chapter[i])];
					
					if(currentDetail == nil)		currentDetail = [NSMutableArray new];
					
					[currentDetail addObjectsFromArray:currentBurst];
				}
				
				currentBurst = counter > 5 ? [NSMutableArray new] : [NSMutableArray arrayWithObject:@(chapter[i])];
				repeatingDiff = diff[i];	counter = 1;
			}
			else
			{
				[currentBurst addObject:@(chapter[i])];
				counter++;
			}
		}
		
		if(currentDetail != nil && [currentDetail count])
			[output addObject:currentDetail];
	}
	
	return [NSArray arrayWithArray:output];
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
			
			output[cache++].details = NULL;
		}
		
		output[*nbElem].details = NULL;
		output[*nbElem].ID = VALEUR_FIN_STRUCT;
	}
	
	return output;
}

NSArray * recoverVolumeBloc(META_TOME * volume, uint length)
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
		
		dict = [NSMutableDictionary dictionaryWithObject:@(volume[pos].ID) forKey:@"Internal ID"];
		
		if(volume[pos].description[0])
			[dict setObject:[[NSString alloc] initWithData:[NSData dataWithBytes:volume[pos].description length:sizeof(volume[pos].description)] encoding:NSUTF32LittleEndianStringEncoding] forKey:@"description"];
		
		if(volume[pos].readingName[0])
			[dict setObject:[[NSString alloc] initWithData:[NSData dataWithBytes:volume[pos].readingName length:sizeof(volume[pos].readingName)] encoding:NSUTF32LittleEndianStringEncoding] forKey:@"Reading name"];
		
		if(volume[pos].readingID != VALEUR_FIN_STRUCT)
			[dict setObject:@(volume[pos].readingID) forKey:@"Reading ID"];
		
		[output addObject:dict];
	}
	
	return [NSArray arrayWithArray:output];
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
	
	uint nbChapters = 0, nbVolumes = 0;
	chapters = getChapters([bloc objectForKey:@"chapters"], &nbChapters);
	volumes = getVolumes([bloc objectForKey:@"volumes"], &nbVolumes);

	if(nbChapters == 0 && nbVolumes == 0)									goto end;
	
	NSString * description = [bloc objectForKey:@"description"];
	if(description == nil || [description superclass] != [NSMutableString class])	description = nil;
	
	NSString * authors = [bloc objectForKey:@"author"];
	if(authors == nil || [authors superclass] != [NSMutableString class])			goto end;
	
	NSNumber * status = [bloc objectForKey:@"status"];
	if(status == nil || [status superclass] != [NSNumber class])			goto end;
	
	NSNumber * type = [bloc objectForKey:@"type"];
	if(type == nil || [type superclass] != [NSNumber class])				goto end;
	
	NSNumber * asianOrder = [bloc objectForKey:@"asian_order_of_reading"];
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
	wcsncpy(data.authorName, (wchar_t*) [authors cStringUsingEncoding:NSUTF32StringEncoding], LENGTH_AUTHORS);
	
	if(description != nil)
		wcsncpy(data.description, (wchar_t*) [description cStringUsingEncoding:NSUTF32StringEncoding], LENGTH_DESCRIPTION);
	else
		memset(&data.description, 0, sizeof(data.description));
	
	chapters = NULL;
	volumes = NULL;
end:
	
	free(chapters);
	freeTomeList(volumes, true);
	return data;
}

NSDictionary * reverseParseBloc(PROJECT_DATA project)
{
	if(project.team == NULL)
		return nil;
	
	id buf;
	NSMutableDictionary * output = [NSMutableDictionary dictionary];
	
	[output setObject:@(project.projectID) forKey:@"ID"];
	[output setObject:[[NSString alloc] initWithData:[NSData dataWithBytes:project.projectName length:wstrlen(project.projectName) * sizeof(wchar_t)] encoding:NSUTF32LittleEndianStringEncoding] forKey:@"projectName"];
	
	buf = recoverChapterBloc(project.chapitresFull, project.nombreChapitre);
	if(buf != nil)		[output setObject:buf forKey:@"chapters"];
	
	buf = recoverVolumeBloc(project.tomesFull, project.nombreTomes);
	if(buf != nil)		[output setObject:buf forKey:@"volumes"];
	
	if(project.description[0])
		[output setObject:[[NSString alloc] initWithData:[NSData dataWithBytes:project.description length:wstrlen(project.description) * sizeof(wchar_t)] encoding:NSUTF32LittleEndianStringEncoding] forKey:@"description"];
	
	if(project.authorName[0])
		[output setObject:[[NSString alloc] initWithData:[NSData dataWithBytes:project.authorName length:wstrlen(project.authorName) * sizeof(wchar_t)] encoding:NSUTF32LittleEndianStringEncoding] forKey:@"author"];
	
	[output setObject:@(project.status) forKey:@"status"];
	[output setObject:@(project.type) forKey:@"type"];
	[output setObject:@(project.japaneseOrder) forKey:@"asian_order_of_reading"];
	[output setObject:@(project.category) forKey:@"category"];
	
	return [NSDictionary dictionaryWithDictionary:output];
}

PROJECT_DATA_EXTRA parseBlocExtra(NSDictionary * bloc)
{
	PROJECT_DATA_EXTRA output;
	PROJECT_DATA shortData = parseBloc(bloc), empty;
	memset(&empty, 0, sizeof(empty));
	
	if(memcmp(&shortData, &empty, sizeof(PROJECT_DATA)))
	{
		memcpy(&output, &shortData, sizeof(shortData));
		
		NSString * crcLarge = [bloc objectForKey:@"hash_URL_large_pic"];
		if(crcLarge != nil && [crcLarge superclass] == [NSMutableString class])
			strncpy(output.hashLarge, [crcLarge cStringUsingEncoding:NSASCIIStringEncoding], LENGTH_HASH);
		else
			memset(output.hashLarge, 0, LENGTH_HASH);
		
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

void* parseJSON(TEAMS_DATA* team, NSDictionary * remoteData, uint * nbElem, bool parseExtra)
{
	void * outputData = NULL;
	NSArray * projects = [remoteData objectForKey:@"projects"];
	
	if(projects == nil || [projects superclass] != [NSArray class])
		return NULL;
	
	size_t size = [projects count];
	outputData = malloc(size * (parseExtra ? sizeof(PROJECT_DATA_EXTRA) : sizeof(PROJECT_DATA)));
	
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
			
			if(parseExtra)
				((PROJECT_DATA_EXTRA*)outputData)[validElements] = parseBlocExtra(remoteData);
			else
				((PROJECT_DATA*)outputData)[validElements] = parseBloc(remoteData);
		
			if(memcmp(&(outputData[validElements]), &emptySample, sizeof(emptySample)))
			{
				PROJECT_DATA * project;
				
				if(parseExtra)
					project = (PROJECT_DATA*) &((PROJECT_DATA_EXTRA*)outputData)[validElements++];
				else
					project = &((PROJECT_DATA*)outputData)[validElements++];
				
				project->team = team;
				if(project->tomesFull != NULL)
				{
					for(uint i = 0; i < project->nombreTomes; i++)
					{
						project->tomesFull[i].details = NULL;
						parseTomeDetails(*project, project->tomesFull[i].ID, &(project->tomesFull[i].details));
					}
				}
			}
		}
		
		if(nbElem != NULL)
			*nbElem = validElements;
	}
	
	return outputData;
}

PROJECT_DATA_EXTRA * parseRemoteData(TEAMS_DATA* team, char * remoteDataRaw, uint * nbElem)
{
	NSError * error = nil;
	NSMutableDictionary * remoteData = [NSJSONSerialization JSONObjectWithData:[NSData dataWithBytes:remoteDataRaw length:ustrlen(remoteDataRaw)] options:0 error:&error];
	
	if(error != nil || remoteData == nil || [remoteData superclass] != [NSMutableDictionary class])
		return NULL;
	
	id teamName = [remoteData objectForKey:@"authorName"], teamURL = [remoteData objectForKey:@"authorURL"];
	if (teamName == nil || [teamName superclass] != [NSMutableString class] || ![(NSString*) teamName isEqualToString:[NSString stringWithUTF8String:team->teamLong]] ||
		teamURL == nil || [teamURL superclass] != [NSMutableString class] || ![(NSString*) teamURL isEqualToString:[NSString stringWithUTF8String:team->URLRepo]])
	{	return NULL;	}
	
	return parseJSON(team, remoteData, nbElem, true);
}

PROJECT_DATA * parseLocalData(TEAMS_DATA ** team, uint nbTeam, unsigned char * remoteDataRaw, uint *nbElem)
{
	if(team == nil || nbElem == NULL)
		return NULL;
	
	NSError * error = nil;
	NSMutableDictionary * remoteData = [NSJSONSerialization JSONObjectWithData:[NSData dataWithBytes:remoteDataRaw length:ustrlen(remoteDataRaw)] options:0 error:&error];
	
	if(error != nil || remoteData == nil || [remoteData superclass] != [NSArray class])
		return NULL;
	
	uint nbElemPart, posTeam;
	PROJECT_DATA *output = NULL, *currentPart;
	id teamName, teamURL;
	
	for(NSDictionary * remoteDataPart in remoteData)
	{
		if([remoteDataPart superclass] != [NSMutableDictionary class])
			continue;
		
		teamName = [remoteDataPart objectForKey:@"authorName"];
		teamURL = [remoteDataPart objectForKey:@"authorURL"];
		if (teamName == nil || [teamName superclass] != [NSMutableString class]|| teamURL == nil || [teamURL superclass] != [NSMutableString class])
			continue;
		
		for(posTeam = 0; posTeam < nbTeam; posTeam++)
		{
			if(team[posTeam] == NULL)
				continue;
			
			if([(NSString*) teamName isEqualToString:[NSString stringWithUTF8String:team[posTeam]->teamLong]] && [(NSString*) teamURL isEqualToString:[NSString stringWithUTF8String:team[posTeam]->URLRepo]])
			{
				nbElemPart = 0;
				currentPart = (PROJECT_DATA*) parseJSON(team[posTeam], remoteDataPart, &nbElemPart, false);
				
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

char * reversedParseData(PROJECT_DATA * data, uint nbElem, TEAMS_DATA ** team, uint nbTeam, size_t * sizeOutput)
{
	if(data == NULL || team == NULL || !nbElem || !nbTeam)
		return NULL;
	
	uint counters[nbTeam], jumpTable[nbTeam][nbElem];
	bool projectLinkedToTeam = false;
	
	memset(counters, 0, sizeof(counters));
	
	//Create a table linking projects to team
	for(uint pos = 0, posTeam; pos < nbElem; pos++)
	{
		for (posTeam = 0; posTeam < nbTeam; posTeam++)
		{
			if(data[pos].team == team[posTeam])
			{
				jumpTable[posTeam][counters[posTeam]++] = pos;
				projectLinkedToTeam = true;
				break;
			}
		}
	}
	
	if(!projectLinkedToTeam)
		return NULL;
	
	NSMutableArray *root = [NSMutableArray array], *projects;
	NSDictionary * currentNode;
	id currentProject;
	
	for(uint pos = 0; pos < nbTeam; pos++)
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
			currentNode = [NSDictionary dictionaryWithObjects:@[[NSString stringWithUTF8String:team[pos]->teamLong], [NSString stringWithUTF8String:team[pos]->URLRepo], projects] forKeys:@[@"authorName", @"authorURL", @"projects"]];
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