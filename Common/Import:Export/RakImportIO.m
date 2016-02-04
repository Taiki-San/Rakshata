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

RakImportBaseController <RakImportIO> * createIOForFilename(NSString * filename)
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
		return [[RakImportRarController alloc] initWithFilename:filename];

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

	return nil;
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
		//Cool, simplified analysis available
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
			
			//Easy case, no analysis needed
			if(node.isFlatCT)
			{
				RakImportItem * item = [RakImportItem new];
				if(item == nil)
					continue;
				
				item.issue = IMPORT_PROBLEM_METADATA;
				item.path = node.nodeName;
				item.projectData = getEmptyExtraProject();
				item.IOController = node.IOController;
				if(item.IOController == nil)
					item.IOController = IOController;
				
				item.contentID = INVALID_VALUE;
				item.isTome = node.nbImages > THREESOLD_IMAGES_FOR_VOL;
				NSString * inferedName = [item inferMetadataFromPathWithHint:YES];
				
				PROJECT_DATA_EXTRA extraProject = item.projectData;
				
				if(inferedName == nil)
					inferedName = [[item.path lastPathComponent] stringByDeletingPathExtension];
				
				//We try to find if we can easily match a project
				extraProject.data.project.cacheDBID = getProjectByName([inferedName UTF8String]);
				
				//No luck ¯\_(ツ)_/¯
				if(extraProject.data.project.cacheDBID == INVALID_VALUE)
				{
					wstrncpy(extraProject.data.project.projectName, LENGTH_PROJECT_NAME, getStringFromUTF8((const byte *) [inferedName UTF8String]));
					extraProject.data.project.locale = true;
					extraProject.data.project.projectID = getEmptyLocalSlot(extraProject.data.project);
				}
				else
				{
					extraProject.data.project = getProjectByID(extraProject.data.project.cacheDBID);
				}
				
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
