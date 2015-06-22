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

ROOT_REPO_DATA ** parserRakSourceFile(NSData * fileContent, uint * nbElem)
{
	NSError * parseError = nil;
	NSDictionary * parseData = [NSJSONSerialization JSONObjectWithData:fileContent options:0 error:&parseError];
	
	if(parseError != nil)
	{
#ifdef DEV_VERSION
		NSLog(@"%@", parseError);
#endif
		return NULL;
	}
	
	else if(parseData == nil || ARE_CLASSES_DIFFERENT(parseData, [NSDictionary class]))
		return NULL;
	
	//Check the file version
	NSNumber * version = objectForKey(parseData, JSON_RAK_MIN_VERSION, @"version", [NSNumber class]);
	if(version == nil || [version unsignedIntValue] > VERSION_RAK_FILE)
	{
		NSLog(@"Rakshata couldn't read the file you submitted, sorry :/");
		return NULL;
	}
	
	//Ok, let's extract the payload
	uint nbElements;
	NSArray * payload = objectForKey(parseData, JSON_RAK_PAYLOAD, @"payload", [NSArray class]);
	if(payload == nil || !(nbElements = [payload count]))
	{
		NSLog(@"Empty file");
		return NULL;
	}
	
	//We allocate the maximum possible amount of memory
	int repoVersion;
	uint nbRealElements = 0;
	char * downloadBuffer = NULL;
	ROOT_REPO_DATA ** output = malloc(nbElements * sizeof(ROOT_REPO_DATA *)), dummyRepo;
	
	if(output == NULL)
	{
		memoryError(nbElements * sizeof(ROOT_REPO_DATA *));
		return NULL;
	}
	
	//We can now interate the paypload entries
	for (NSDictionary * entry in payload)
	{
		if(entry == nil || ARE_CLASSES_DIFFERENT(entry, [NSDictionary class]))
		{
#ifdef DEV_VERSION
			NSLog(@"Invalid entry: %@", entry);
#endif
			continue;
		}
		
		//Analysing the structure
		NSNumber * type;
		NSString * URL;
		NSArray * preselection;
		
		type = objectForKey(entry, JSON_RAK_TYPE, @"type", [NSNumber class]);
		if(type == nil || ![type unsignedCharValue] || [type unsignedCharValue] > MAX_TYPE_DEPOT)
			continue;
		
		URL = objectForKey(entry, JSON_RAK_URL, @"URL", [NSString class]);
		if(URL == nil || ![URL length] || [URL length] >= REPO_URL_LENGTH)
			continue;
		
		preselection = objectForKey(entry, JSON_RAK_PRESELECTION, @"preselection", [NSArray class]);
		if(preselection != nil || ![preselection count])
			continue;
		
		//Craft a structure to send to the update routine in order to gather the data
		dummyRepo.type = [type unsignedCharValue];
		strncpy(dummyRepo.URL, [URL UTF8String], REPO_URL_LENGTH);
		dummyRepo.URL[REPO_URL_LENGTH - 1] = 0;

		size_t length;
		//Error when downloading the repo data
		if((repoVersion = getUpdatedRepo(&downloadBuffer, &length, dummyRepo)) == -1 || downloadBuffer == NULL || length == 0)
			continue;

		//If parsing success
		if(parseRemoteRootRepo(downloadBuffer, repoVersion, &(output[nbRealElements])))
		{
			//We have to apply the preselection, we first disable everything (the default) then opt-in
			uint max = output[nbRealElements]->nombreSubrepo;
			
			for (uint pos = 0; pos < max; pos++)
				((REPO_DATA_EXTRA*) output[nbRealElements]->subRepo)[pos].data->active = false;
			
			if(preselection != nil)
			{
				for(NSNumber * object in preselection)
				{
					if(ARE_CLASSES_DIFFERENT(object, [NSNumber class]))
						continue;
					
					uint currentValue = [object unsignedIntValue];
					if(currentValue < max)
						((REPO_DATA_EXTRA*) output[nbRealElements]->subRepo)[currentValue].data->active = true;
				}
			}

			nbRealElements++;
		}
		
		free(downloadBuffer);
		downloadBuffer = NULL;
	}
	
	//If some elements were discarded, we reduce our buffer size
	if(nbRealElements != nbElements)
	{
		void * tmp = realloc(output, nbRealElements * sizeof(ROOT_REPO_DATA*));
		if(tmp != NULL)
			output = tmp;
	}
	
	//We update the output pointers then return
	*nbElem = nbRealElements;
	return output;
}