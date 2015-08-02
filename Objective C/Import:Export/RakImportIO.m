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

NSArray <RakImportItem *> * getManifestForIOs(NSArray <id <RakImportIO>> * IOControllers)
{
	if([IOControllers count] == 1)
	{
		id controller = [IOControllers objectAtIndex:0];
		if([controller respondsToSelector:@selector(getManifest)])
			return [controller getManifest];
	}

	return nil;
}