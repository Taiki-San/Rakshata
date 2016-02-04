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
 **			file, You can obtain one at https://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

@implementation RakImportItem

- (BOOL) isReadable
{
	if(!_isTome)
		return checkReadable(_projectData.data.project, false, _contentID);
	
	if(_projectData.data.nbVolumesLocal == 0)
		return false;

	//The volume must be in the list
	META_TOME * tomeFull = _projectData.data.project.volumesFull;
	uint lengthTomeFull = _projectData.data.project.nbVolumes;

	_projectData.data.project.volumesFull = _projectData.data.tomeLocal;
	_projectData.data.project.nbVolumes = _projectData.data.nbVolumesLocal;

	bool output = checkReadable(_projectData.data.project, true, _projectData.data.tomeLocal[0].ID);

	_projectData.data.project.volumesFull = tomeFull;
	_projectData.data.project.nbVolumes = lengthTomeFull;

	return output;
}

- (BOOL) needMoreData
{
	if(_projectData.data.project.isInitialized)
		return false;

	PROJECT_DATA project = _projectData.data.project;

	if(project.projectName[0] == 0)
		return true;

	if(project.authorName[0] == 0)
		return true;

	return false;
}

- (BOOL) installWithUI : (RakImportStatusController *) UI
{
	char * projectPath = getPathForProject(_projectData.data.project);
	if(projectPath == NULL)
		return false;

	if(_contentID == INVALID_VALUE)
	{
		_issue = IMPORT_PROBLEM_METADATA_DETAILS;
		return NO;
	}

	char basePath[sizeof(projectPath) + 256];
	uint selection = _contentID;

	//Local project
	if(_isTome)
	{
		snprintf(basePath, sizeof(basePath), PROJECT_ROOT"%s/", projectPath);
	}
	else
	{
		if(selection % 10)
			snprintf(basePath, sizeof(basePath), PROJECT_ROOT"%s/"CHAPTER_PREFIX"%u.%u/", projectPath, selection / 10, selection % 10);
		else
			snprintf(basePath, sizeof(basePath), PROJECT_ROOT"%s/"CHAPTER_PREFIX"%u/", projectPath, selection / 10);
	}

	//Main decompression cycle
	if([_IOController respondsToSelector:@selector(willStartEvaluateFromScratch)])
		[_IOController willStartEvaluateFromScratch];

	__block BOOL foundOneThing = NO, wantedBroadAccess = NO;
	__block NSString * basePathObj = [NSString stringWithUTF8String:basePath];

	[_IOController evaluateItemFromDir:_path withInitBlock:^(uint nbItems, BOOL wantBroadWriteAccess) {

		if(UI != nil)
			UI.nbElementInEntry = nbItems;
		
		//.rak need to write in the whole dir, to handle shared CT
		//However, arbitrary imports want a more focused decompression
		if(!wantBroadWriteAccess && self.isTome)
		{
			META_TOME tomeData = self.projectData.data.tomeLocal[0];
			uint chapID = tomeData.details[0].ID;
			
			if(chapID % 10)
				basePathObj = [basePathObj stringByAppendingString:[NSString stringWithFormat:@"/"VOLUME_PREFIX"%u/"CHAPTER_PREFIX"%u.%u/", tomeData.ID, chapID / 10, chapID % 10]];
			else
				basePathObj = [basePathObj stringByAppendingString:[NSString stringWithFormat:@"/"VOLUME_PREFIX"%u/"CHAPTER_PREFIX"%u/", tomeData.ID, chapID / 10]];
		}
		else if(wantBroadWriteAccess && self.isTome)
			wantedBroadAccess = YES;
		
		//We create the path if needed
		if(!checkDirExist([basePathObj UTF8String]))
		{
			createPath([basePathObj UTF8String]);
		}

	} andWithBlock:^(id<RakImportIO> controller, NSString * filename, uint index, BOOL * stop)
	{
		if(UI == nil || ![UI haveCanceled])
		{
			if(UI != nil)
				UI.posInEntry = index;

			[controller copyItemOfName:filename toDir:basePathObj];
			foundOneThing = YES;
		}
		else
			*stop = YES;
	}];
	
	[_IOController generateConfigDatInPath:basePathObj];
	
	if(wantedBroadAccess)	//We need to add the unread flag
		basePathObj = [basePathObj stringByAppendingString:[NSString stringWithFormat:@"/"VOLUME_PREFIX"%u/", self.projectData.data.tomeLocal[0].ID]];

	finishInstallationAtPath([basePathObj UTF8String]);

	//Decompression is over, now, we need to ensure everything is fine
	if((UI != nil && [UI haveCanceled]) || !foundOneThing || ![self isReadable])
	{
		//Oh, the entry was not valid 😱
		if(UI != nil && ![UI haveCanceled])
			logR("Uh? Invalid import :|");

		if(_isTome)
			snprintf(basePath, sizeof(basePath), PROJECT_ROOT"%s/"VOLUME_PREFIX"%u", projectPath, _contentID);

		removeFolder(basePath);
		free(projectPath);
		return false;
	}

	free(projectPath);
	return true;
}

- (BOOL) overrideDuplicate
{
	if(_issue != IMPORT_PROBLEM_DUPLICATE)
		return NO;

	[self deleteData];
	if(![self installWithUI:nil])
	{
		_issue = IMPORT_PROBLEM_INSTALL_ERROR;
		return NO;
	}

	[self processThumbs];
	[self registerProject];
	_issue = IMPORT_PROBLEM_NONE;

	return YES;
}

- (BOOL) updateProject : (PROJECT_DATA) project
{
	if(_projectData.data.project.cacheDBID != project.cacheDBID &&  project.nbVolumes > 0 && (project.locale || project.volumesFull[0].ID > MIN_LOCAL_VOLUME_ID))
	{
		uint ID = project.volumesFull[0].ID;
		while(checkTomeReadable(project, ID))
			ID = getVolumeIDForImport(project);
		
		project.volumesFull[0].ID = _contentID = ID;
	}
	
	_projectData.data.project = project;

	[self refreshState];

	//Okay! We're good
	if(_issue == IMPORT_PROBLEM_NONE)
	{
		[self processThumbs];
		[self registerProject];

		return YES;
	}

	return NO;
}

- (void) updateCTIDWith : (uint) contentID tomeName : (NSString *) tomeName isTome : (BOOL) isTome
{
	PROJECT_DATA_EXTRA project = _projectData;
	_contentID = contentID;
	
	if(isTome)
	{
		META_TOME tomeData;
		
		if(tomeName == nil)
			tomeName = @"";
		
		//Somehow, the volume wasn't registered ¯\_(ツ)_/¯
		if(project.data.nbVolumesLocal == 0)
		{
			tomeData.ID = getVolumeIDForImport(project.data.project);
			if(tomeData.ID != INVALID_VALUE)
			{
				tomeData.readingID = (int) contentID;
				wstrncpy(tomeData.readingName, MAX_TOME_NAME_LENGTH + 1, getStringFromUTF8((const byte *) [tomeName UTF8String]));
				
				tomeData.details = malloc(sizeof(CONTENT_TOME));
				if(tomeData.details != NULL)
				{
					tomeData.details[0].ID = CHAPTER_FOR_IMPORTED_VOLUMES;
					tomeData.details[0].isPrivate = true;
					tomeData.lengthDetails = 1;
				}
				
				project.data.tomeLocal = malloc(sizeof(META_TOME));
				if(project.data.tomeLocal != NULL)
				{
					project.data.nbVolumesLocal = 1;
					project.data.tomeLocal[0] = tomeData;
					
					//If it migrated, we remove the other entry
					if(project.data.nbChapterLocal > 0)
					{
						project.data.nbChapterLocal = 0;
						free(project.data.chaptersLocal);
						project.data.chaptersLocal = NULL;
					}
				}
				else
					free(tomeData.details);
			}
			else
				logR("Couldn't grab a volume ID ಠ_ಠ\nYou should restart Rakshata and/or import smaller chuncks (and send us an email)");
		}
		else
		{
			project.data.tomeLocal[0].readingID = (int) contentID;
			wstrncpy(project.data.tomeLocal[0].readingName, MAX_TOME_NAME_LENGTH + 1, getStringFromUTF8((const byte *) [tomeName UTF8String]));
		}
	}
	else if(_isTome != isTome)
	{
		releaseParsedData(project.data);
		nullifyParsedPointers(&project.data);
		
		project.data.chaptersLocal = malloc(sizeof(uint));
		if(project.data.chaptersLocal != NULL)
		{
			project.data.nbChapterLocal = 1;
			project.data.chaptersLocal[0] = contentID;
			generateCTUsable(&project.data);
		}
	}
	else
	{
		project.data.chaptersLocal[0] = contentID;
	}
	
	_projectData = project;
	_isTome = isTome;
}

- (void) refreshState
{
	//Check the data update is enough
	if([self needMoreData])
		_issue = IMPORT_PROBLEM_METADATA;

	//We have a valid targer
	else if(_contentID == INVALID_VALUE || (_isTome && _projectData.data.tomeLocal[0].readingID == INVALID_SIGNED_VALUE
											&& _projectData.data.tomeLocal[0].readingName[0] == 0))
		_issue = IMPORT_PROBLEM_METADATA_DETAILS;

	//Quick check we're not already installed
	else if([self isReadable])
		_issue = IMPORT_PROBLEM_DUPLICATE;

	//Perform installation
	else if(![self installWithUI:nil])
		_issue = IMPORT_PROBLEM_INSTALL_ERROR;

	else
	{
		[self processThumbs];
		[self registerProject];

		_issue = IMPORT_PROBLEM_NONE;
	}
}

- (void) deleteData
{
	if(!_isTome)
		return	internalDeleteCT(_projectData.data.project, false, _contentID);

	//The volume must be in the list
	META_TOME * tomeFull = _projectData.data.project.volumesFull;
	uint lengthTomeFull = _projectData.data.project.nbVolumes;

	_projectData.data.project.volumesInstalled = _projectData.data.project.volumesFull = _projectData.data.tomeLocal;
	_projectData.data.project.nbVolumesInstalled = _projectData.data.project.nbVolumes = _projectData.data.nbVolumesLocal;

	internalDeleteTome(_projectData.data.project, _contentID, true);

	_projectData.data.project.volumesFull = tomeFull;
	_projectData.data.project.nbVolumes = lengthTomeFull;
}

- (void) processThumbs
{
	for(byte pos = 0; pos < NB_IMAGES; pos++)
	{
		if(!_projectData.haveImages[pos])
			continue;

		ICON_PATH path = getPathToIconsOfProject(_projectData.data.project, pos);
		if(path.string[0] == 0)
			continue;

		if(checkFileExist(path.string))
			continue;

		[_IOController copyItemOfName:[NSString stringWithUTF8String:_projectData.URLImages[pos]] toPath:[NSString stringWithUTF8String:path.string]];
	}
}

- (NSData *) queryThumbInWithIndex : (uint) index
{
	if(index >= NB_IMAGES || !_projectData.haveImages[index])
		return nil;

	NSData * data = nil;

	if(![_IOController copyItemOfName:[NSString stringWithUTF8String:_projectData.URLImages[index]] toData:&data])
		return nil;

	return data;
}

- (void) registerProject
{
	if(didRegister)
		return;
	else
		didRegister = YES;
	
	_projectData.data.project.isInitialized = true;
	registerImportEntry(_projectData.data, _isTome);
}

- (BOOL) isEqualTo : (id) object
{
	if(![object isKindOfClass:[self class]])
		return object != nil && [super isEqual:object];
	
	RakImportItem * item = object;
	
	if([_path isEqualToString:item.path] && _IOController == item.IOController
	   && areProjectsIdentical(_projectData.data, item.projectData.data)
	   && _contentID == item.contentID && _isTome == item.isTome
	   && _issue == item.issue)
		return YES;
	
	return NO;
}

#pragma mark - Metadata inference

//If hinted, and we can't figure anything ourselves, we relax the search
- (NSString *) inferMetadataFromPathWithHint : (BOOL) haveHintedCT
{
	NSString * inferedName = nil, * strippedPath = [[_path lastPathComponent] stringByDeletingPathExtension];
	const char * forbiddenChars = "_ -~:;|", nbForbiddenChars = 7;
	NSMutableCharacterSet * charSet = [NSMutableCharacterSet new];
	[charSet addCharactersInString:[NSString stringWithUTF8String:forbiddenChars]];

	//We look for something la C* or [T-V]* followed then exclusively by numbers
	NSArray * tokens = [strippedPath componentsSeparatedByCharactersInSet:charSet];

	BOOL inferingTome, firstPointCrossed, justLookingAtSomeNumbers, inferedSomethingSolid = NO, inferedSomethingUsable = NO, isTomeInfered;
	uint elementID, discardedCloseCalls = 0;

	for(uint i = 0, length, posInChunk, baseDigits; i < [tokens count]; ++i)
	{
		NSData * data = [[tokens objectAtIndex:i] dataUsingEncoding:NSASCIIStringEncoding allowLossyConversion:YES];
		const char * simplifiedChunk = [data bytes];
		length = [data length];

		if(length == 0)
			continue;
		
		justLookingAtSomeNumbers = NO;

		char sample = toupper(simplifiedChunk[0]);

		if(sample == 'C')	//Chapter?
			inferingTome = NO;

		else if(sample == 'V' || sample == 'T')	//Volume? (or Tome)
			inferingTome = YES;
		
		else if(!inferedSomethingSolid && isdigit(sample))
			justLookingAtSomeNumbers = YES;

		else
			continue;

		for(posInChunk = 1; posInChunk < length && !isdigit(simplifiedChunk[posInChunk]); ++posInChunk);

		//Couldn't find digits in the end of the chunk and the next chunk doesn't start with digits
		if(posInChunk == length && (i >= [tokens count] - 1 || [[tokens objectAtIndex:i + 1] length] == 0 || !isdigit([[tokens objectAtIndex:i + 1] UTF8String][0])))
		{
			++discardedCloseCalls;
			continue;
		}

		//Digits analysis

		//If we separated the numbers from the base
		if(posInChunk == length)
		{
			data = [[tokens objectAtIndex:i + 1] dataUsingEncoding:NSASCIIStringEncoding allowLossyConversion:YES];
			simplifiedChunk = [data bytes];
			length = [data length];
			posInChunk = 0;
		}

		//We only want digits at this point, any interruption will result in elimination
		firstPointCrossed = NO;
		for(baseDigits = posInChunk; posInChunk < length; ++posInChunk)
		{
			if(isdigit(simplifiedChunk[posInChunk]))
				continue;

			if(!firstPointCrossed && simplifiedChunk[posInChunk] == '.')
			{
				firstPointCrossed = YES;
				continue;
			}

			break;
		}

		//Close call, but nop, invalid
		if(posInChunk < length)
		{
#ifdef EXTENSIVE_LOGGING
			logR("Close call, this chunk almost got me!");
			NSLog(@"`%@` -> %s", tokens, simplifiedChunk);
#endif
			++discardedCloseCalls;
			continue;
		}

		//Well, we're good
		
		if(!justLookingAtSomeNumbers)
			isTomeInfered = inferingTome;
		
		double inferedElementID = atof(&simplifiedChunk[baseDigits]);
		inferedElementID *= 10;

		if(inferedElementID >= INT_MIN && inferedElementID <= INT_MAX)
		{
			elementID = (uint) inferedElementID;

			if(justLookingAtSomeNumbers)
			{
				inferedSomethingUsable = YES;
				isTomeInfered = _isTome;	//isTome is an heuristic based on the number of element in the archive
			}
			
			else if(inferedSomethingSolid)
				++discardedCloseCalls;
			else
				inferedSomethingSolid = YES;
		}
		else if(!justLookingAtSomeNumbers)
			++discardedCloseCalls;
	}

	if(inferedSomethingSolid || inferedSomethingUsable)
	{
		_isTome = isTomeInfered;
		_contentID = elementID;

		//We'll assume that most of what came before CT was the name
		char * fullName = strdup([[[_path lastPathComponent] stringByDeletingPathExtension] UTF8String]);
		uint fullNameLength = strlen(fullName), posFullName = 0;
		BOOL nextCharNeedChecking = YES;

		for(; posFullName < fullNameLength; ++posFullName)
		{
			char c = fullName[posFullName];
			if(nextCharNeedChecking)
			{
				c = toupper(c);
				if((c == 'C' || c == 'T' || c == 'V') && discardedCloseCalls-- == 0)
				{
					//If it starts with the C/T/V we used, we stop the prediction
					if(posFullName)
					{
						fullName[posFullName] = 0;
						inferedName = [NSString stringWithUTF8String:fullName];
					}
					break;
				}
				else
					nextCharNeedChecking = NO;
			}
			else
			{
				//We don't care about hot chars if they are not the begining of a chunk
				for(byte i = 0; i < nbForbiddenChars; ++i)
				{
					if(c == forbiddenChars[i])
					{
						nextCharNeedChecking = YES;
						break;
					}
				}
			}
		}

		free(fullName);
	}

	//Perform a simpler search
	else if(haveHintedCT)
	{
		charSet = [NSMutableCharacterSet new];
		[charSet addCharactersInString:@"0123456789."];
		NSCharacterSet * notOnlyDigits = [charSet invertedSet];

		for(NSString * chunk in tokens)
		{
			if([chunk length] == 0 || [chunk rangeOfCharacterFromSet:notOnlyDigits].location != NSNotFound)
				continue;

			double inferedElementID = atof([chunk UTF8String]) * 10;

			if(inferedElementID >= INT_MIN && inferedElementID <= INT_MAX)
			{
				elementID = (uint) inferedElementID;
				inferedSomethingSolid = YES;
			}
		}

		//We found something \o/
		if(inferedSomethingSolid)
			_contentID = elementID;
	}

	if(inferedName == nil || inferedName.length == 0)
		inferedName = strippedPath;

	//We try to clean it up a bit
	return [self cleanupString : inferedName];
}

- (NSString *) cleanupString : (NSString *) inferedName
{
	if(inferedName == nil)
		return nil;

	//First, remove digits withing parenthesis
	inferedName = [inferedName stringByReplacingOccurrencesOfString:@"\\(\\d+?\\)"
														 withString:@""
															options:NSRegularExpressionSearch
															  range:NSMakeRange(0, inferedName.length)];

	//Then, anything between []
	inferedName = [inferedName stringByReplacingOccurrencesOfString:@"\\[.*\\]"
														 withString:@""
															options:NSRegularExpressionSearch
															  range:NSMakeRange(0, inferedName.length)];

	if(inferedName != nil && [inferedName length] > 0)
	{
		NSMutableCharacterSet * charSet = [NSMutableCharacterSet new];
		[charSet addCharactersInString:@"_ -~:;|"];

		//Good, now, let's replace _ with spaces, and remove noise from the end
		inferedName = [inferedName stringByReplacingOccurrencesOfString:@"_" withString:@" "];
		inferedName = [inferedName stringByTrimmingCharactersInSet:charSet];
	}

	//Okay, we're going too hard, we give up
	if([inferedName length] == 0)
		return nil;

	return inferedName;
}

@end