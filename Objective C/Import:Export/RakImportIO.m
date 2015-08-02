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

#define THREESOLD_IMAGES_FOR_VOL 80

id <RakImportIO> createIOForFilename(NSString * filename)
{
	NSString * extension = [filename pathExtension];

	//Import a .rak
	if([extension caseInsensitiveCompare:ARCHIVE_FILE_EXT] == NSOrderedSame)
		return [[RakImportDotRakController alloc] initWithFilename:filename];

	//Import a directory
	if([extension isEqualToString:@""] && checkDirExist([filename UTF8String]))
		return [[RakImportDirController alloc] initWithFilename:filename];

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

	return nil;
}

NSArray <RakImportItem *> * getManifestForIOs(NSArray <id <RakImportIO>> * _IOControllers)
{
	NSMutableArray <RakImportItem *> * output = [NSMutableArray array];

	//We iterate input files, we perform independant analysis on each of them
	//We don't try to merge different input files (eg, all flat CT) because it would be an issues
	//	if the reader was importing the daily releases of various projects in one take
	for(id <RakImportIO> IOController in _IOControllers)
	{
		//Cool, simplified analysis available
		if([IOController respondsToSelector:@selector(getManifest)])
		{
			[output addObjectsFromArray:[(id) IOController getManifest]];
			continue;
		}

		//We get the node
		IMPORT_NODE node = [IOController getNode];
		if(!node.isValid)
			continue;

		//Easy case, no analysis needed
		if(node.isFlatCT)
		{
			RakImportItem * item = [RakImportItem new];
			if(item == nil)
				continue;

			item.issue = IMPORT_PROBLEM_NONE;
			item.path = [NSString stringWithUTF8String:node.nodeName];
			item.projectData = getEmptyExtraProject();
			item.IOController = IOController;

			item.isTome = node.nbImages > THREESOLD_IMAGES_FOR_VOL;
			[item inferMetadataFromPathWithHint:YES];
			[output addObject:item];
		}
	}

	return [NSArray arrayWithArray:output];
}