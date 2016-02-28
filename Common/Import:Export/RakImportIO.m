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

#define THREESOLD_IMAGES_FOR_VOL 80

RakImportBaseController <RakImportIO> * _createIOForFilename(NSString * filename)
{
	NSString * extension = [filename pathExtension];

	//Import a .rak
	if([extension caseInsensitiveCompare:ARCHIVE_FILE_EXT] == NSOrderedSame)
		return [[RakImportDotRakController alloc] initWithFilename:filename];

	//Import a directory
	if([extension isEqualToString:@""] && checkDirExist([filename UTF8String]))
		return [[RakImportDirController alloc] initWithDirname:filename];

	//Look for Rar
	BOOL foundIt = NO;
	for(NSString * string in EXTERNAL_FILE_EXT_RAR)
	{
		if([extension caseInsensitiveCompare:string] == NSOrderedSame)
		{
			foundIt = YES;
			break;
		}
	}

	if(foundIt)
		return [[RakImportMultiController alloc] initWithRarFilename:filename];

	//Look for zip
	for(NSString * string in EXTERNAL_FILE_EXT_ZIP)
	{
		if([extension caseInsensitiveCompare:string] == NSOrderedSame)
		{
			foundIt = YES;
			break;
		}
	}

	if(foundIt)
		return [[RakImportZipController alloc] initWithFilename:filename];
	
	if([extension caseInsensitiveCompare:@"pdf"] == NSOrderedSame)
		return [[RakImportDirController alloc] initWithFilename:filename];
	
	for(NSString * string in EXTERNAL_FILE_EXT_MISC)
	{
		if([extension caseInsensitiveCompare:string] == NSOrderedSame)
		{
			foundIt = YES;
			break;
		}
	}
	
	if(foundIt)
		return [[RakImportMultiController alloc] initWithMultiFilename:filename];

	return nil;
}

RakImportBaseController <RakImportIO> * createIOForFilename(NSString * filename)
{
	RakImportBaseController <RakImportIO> * output = _createIOForFilename(filename);
	
	if(output == nil)	//_createIOForFilename should handle the files if the extension is there and correct. Otherwise, we try to recover whatever we can
	{
		FILE * file = fopen([filename UTF8String], "rb");
		if(file != NULL)
		{
			rawData buffer[0x10];
			
			if(fread(buffer, sizeof(rawData), 0x10, file) == 0x10)
			{
				if(isZIP(buffer))
				{
					output = [[RakImportZipController alloc] initWithFilename:filename];
				}
				else if(isRAR(buffer))
				{
					output = [[RakImportMultiController alloc] initWithRarFilename:filename];
				}
				else if(isPDF(buffer))
				{
					output = [[RakImportDirController alloc] initWithFilename:filename];
				}
				else
				{
					output = [[RakImportMultiController alloc] initWithMultiFilename:filename];
				}
			}
			
			fclose(file);
		}
	}
	
	return output;
}

NSArray <RakImportItem *> * getManifestForIOs(NSArray <RakImportBaseController <RakImportIO> * > * _IOControllers)
{
	if(_IOControllers == nil || [_IOControllers count] == 0)
		return nil;
	
	NSMutableArray <RakImportItem *> * output = [NSMutableArray array];

	//We iterate input files, we perform independant analysis on each of them
	//We don't try to merge different input files (eg, all flat CT) because it would be an issues
	//	if the reader was importing the daily releases of various projects in one take
	for(RakImportBaseController <RakImportIO> * IOController in _IOControllers)
	{
		//Cool, simplified analysis available (.rak only :/)
		if([IOController respondsToSelector:@selector(getManifest)])
		{
			[output addObjectsFromArray:[(id) IOController getManifest]];
			continue;
		}

		//We get the node
		RakImportNode * _node = [IOController getNode];
		if(!_node.isValid)
		{
#ifdef EXTENSIVE_LOGGING
			logR("Invalid root node");
#endif
			continue;
		}
		
		NSArray <RakImportNode *> * nodes = [_node getNodesIncludingChildren];
		BOOL hadProbableProject = NO;
		PROJECT_DATA sharedProject;

		for(RakImportNode * node in nodes)
		{
			if(!node.isValid)
			{
#ifdef EXTENSIVE_LOGGING
				logR("Invalid node");
				NSLog(@"%@", node);
#endif
				continue;
			}
			
			//Not something we can handle, just ignore the node
			if(!node.isFlatCT && !node.probablyIsProject)
				continue;
			
			//We build a basic RakImportItem, containing the raw data, needed for further analysis
			RakImportItem * item = [RakImportItem new];
			PROJECT_DATA_EXTRA extraProject = getEmptyExtraProject();
			if(item == nil)
				continue;
			
			item.issue = IMPORT_PROBLEM_METADATA;
			item.path = node.nodeName;
			item.IOController = node.IOController;
			if(item.IOController == nil)
				item.IOController = IOController;
			
			item.contentID = INVALID_VALUE;
			item.isTome = node.nbImages > THREESOLD_IMAGES_FOR_VOL;
			item.guessedProject = YES;
			
			//inferedName is only used if we need a project but inferMetadataFromPathWithHint also guess the # of the archive
			NSString * inferedName = [item inferMetadataFromPathWithHint:YES];
			
			//We don't have a cached project ready to go
			//If we had something we are ready to bet is a group of archives from the same chapter, we only guess its metadata once
			if(node.probablyIsProject || !hadProbableProject)
			{
				if(inferedName == nil)
					inferedName = [[item.path lastPathComponent] stringByDeletingPathExtension];
				
				//We try to find if we can easily match a project
				sharedProject.cacheDBID = getProjectByName([inferedName UTF8String]);
				
				//No luck ¯\_(ツ)_/¯
				if(sharedProject.cacheDBID == INVALID_VALUE)
				{
					wstrncpy(sharedProject.projectName, LENGTH_PROJECT_NAME, getStringFromUTF8((const byte *) [inferedName UTF8String]));
					sharedProject.locale = true;
					sharedProject.projectID = getEmptyLocalSlot(sharedProject);
				}
				else
				{
					sharedProject = getProjectByID(sharedProject.cacheDBID);
					item.guessedProject = NO;
				}
			}
			
			//We apply what we guessed
			extraProject.data.project = sharedProject;
			
			//Easy case, no analysis needed
			if(node.isFlatCT)
			{
				if(item.isTome)
				{
					extraProject.data.tomeLocal = calloc(1, sizeof(META_TOME));
					if(extraProject.data.tomeLocal != NULL)
					{
						CONTENT_TOME * content = malloc(sizeof(CONTENT_TOME));
						if(content != NULL)
						{
							content->ID = CHAPTER_FOR_IMPORTED_VOLUMES;
							content->isPrivate = true;
							
							extraProject.data.tomeLocal[0].details = content;
							extraProject.data.tomeLocal[0].lengthDetails = 1;
							extraProject.data.tomeLocal[0].readingID = item.contentID == INVALID_VALUE ? INVALID_SIGNED_VALUE : (int) item.contentID;
							item.contentID = extraProject.data.tomeLocal[0].ID = getVolumeIDForImport(extraProject.data.project);
							extraProject.data.nbVolumesLocal++;
						}
						else
						{
							memoryError(sizeof(CONTENT_TOME));
							free(extraProject.data.tomeLocal);
							extraProject.data.tomeLocal = NULL;
						}
					}
				}
				else
				{
					extraProject.data.chaptersLocal = malloc(sizeof(uint));
					if(extraProject.data.chaptersLocal != NULL)
					{
						extraProject.data.chaptersLocal[0] = item.contentID;
						extraProject.data.nbChapterLocal++;
					}
				}
				
				item.projectData = extraProject;
				
				[output addObject:item];
			}
			else if(node.probablyIsProject)
			{
				hadProbableProject = YES;
			}
		}
	}

	return [NSArray arrayWithArray:output];
}

void createIOConfigDatForData(NSString * path, char ** filenames, uint nbFiles)
{
    if(path == nil || filenames == NULL || nbFiles == 0)
        return;
    
    NSString * currentString, * extension;
    NSMutableArray * array = [NSMutableArray new];
    NSArray * formats = @[@"png", @"jpg", @"jpeg", @"pdf", @"tiff", @"gif"];
    
    //Look for files with valid formats
    for(uint i = 0; i < nbFiles; ++i)
    {
        if(filenames[i] == NULL)
            continue;
        
        currentString = [NSString stringWithUTF8String:filenames[i]];
        
        if(currentString == nil || [currentString isEqualToString:@""] || [currentString hasPrefix:@"__MACOSX/"])
            continue;
        
        extension = [currentString pathExtension];
        
        for(NSString * currentExtension in formats)
        {
            if([extension caseInsensitiveCompare:currentExtension] == NSOrderedSame)
            {
                [array addObject:[currentString lastPathComponent]];
                break;
            }
        }
    }
    
    //No file
    if([array count] == 0 || [array count] > UINT_MAX)
        return;
	
	if(![path isDirectory])
		path = [path stringByAppendingString:@"/"];
    
    //We craft the config.dat
	
	NSMutableString * outputData = [NSMutableString new];
	NSString * tmpPath = [path stringByAppendingString:@"tmp/"];
	uint counter = 0, additionnalPages = 0;

	mkdirR([tmpPath UTF8String]);

    for(NSString * file in array)
	{
		//We need to rename the file with a cleaner name
		NSString * newFile = [NSString stringWithFormat:@"%d.%@", counter++, [file pathExtension]];
		rename([[path stringByAppendingString:file] UTF8String], [[tmpPath stringByAppendingString:newFile] UTF8String]);
		
		if([[file pathExtension] caseInsensitiveCompare:@"pdf"] != NSOrderedSame)
		{
			[outputData appendFormat:@"\n0 %@", newFile];
		}
		else
		{
			PDFDocument * document = [[PDFDocument alloc] initWithData:[NSData dataWithContentsOfFile:[tmpPath stringByAppendingString:newFile]]];
			if(document != NULL)
			{
				uint length = [document pageCount];
				if(length > 0)
				{
					additionnalPages += length - 1;
					for(uint i = 0; i < length; ++i)
						[outputData appendFormat:@"\n%d %@", i, newFile];
				}
			}
		}
	}
	
	//We remove anything remaining in the directory
	DIR * directory = opendir([path UTF8String]);
	if(directory != NULL)
	{
		struct dirent * entry;

		//Iterate the directory
		while ((entry = readdir(directory)) != NULL)
		{
			//Dirs we don't care about
			if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..") || !strcmp(entry->d_name, "tmp") || entry->d_namlen == 0)
				continue;
			
			if((entry->d_type & DT_DIR) != 0)
				removeFolder([[path stringByAppendingString:[NSString stringWithUTF8String : entry->d_name]] UTF8String]);
			else
				remove([[path stringByAppendingString:[NSString stringWithUTF8String : entry->d_name]] UTF8String]);
		}
		
		closedir(directory);
	}
	
	//We then move back all the files from the tmp directory to the main one
	directory = opendir([tmpPath UTF8String]);
	if(directory != NULL)
	{
		struct dirent * entry;
		
		//Iterate the directory
		while ((entry = readdir(directory)) != NULL)
		{
			//Dirs we don't care about
			if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..") || entry->d_namlen == 0 || (entry->d_type & DT_DIR) != 0)
				continue;

			NSString * filename = [NSString stringWithUTF8String:entry->d_name];
			
			rename([[tmpPath stringByAppendingString:filename] UTF8String], [[path stringByAppendingString:filename] UTF8String]);
		}
		
		closedir(directory);
	}
	
	removeFolder([tmpPath UTF8String]);
	
	FILE * config = fopen([[path stringByAppendingString:@CONFIGFILE] UTF8String], "w+");
	if(config != NULL)
	{
		fprintf(config, "%u\nN", (uint) [array count] + additionnalPages);
		fwrite([outputData UTF8String], [outputData length], 1, config);
		fclose(config);
	}
}

@implementation RakImportNode

- (NSArray <NSString *> *) getChildrenNames
{
	if([_children count] == 0)
		return [NSArray new];
	
	NSMutableArray <NSString *> * childrenNames = [NSMutableArray new];
	for(RakImportNode * child in _children)
		[childrenNames addObject:child.nodeName];
	
	return [NSArray arrayWithArray:childrenNames];
}

- (NSArray <RakImportBaseController <RakImportIO> *> *) getChildrenIOControllers
{
	if([_children count] == 0)
		return [NSArray new];
	
	NSMutableArray <RakImportBaseController <RakImportIO> *> * childrenNames = [NSMutableArray new];
	for(RakImportNode * child in _children)
	{
		if(child.IOController != self.IOController)
			[childrenNames addObject:child.IOController];
	}
	
	return [NSArray arrayWithArray:childrenNames];
}

- (NSArray <RakImportNode *> *) getNodesIncludingChildren
{
	if([self.children count] == 0)
		return @[self];
	
	NSMutableArray <RakImportNode *> * output = [NSMutableArray arrayWithObject:self];

	for(RakImportNode * child in _children)
		[output addObjectsFromArray:[child getNodesIncludingChildren]];

	return [NSArray arrayWithArray:output];
}

@end
