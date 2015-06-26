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

@implementation RakImportItem

- (BOOL) isReadable
{
	return checkReadable(_projectData.data.project, _isTome, _contentID);
}

- (void) install : (unzFile *) archive
{
	char * projectPath = getPathForProject(_projectData.data.project);
	if(projectPath == NULL)
		return;

	char basePath[sizeof(projectPath) + 256];
	int selection = _contentID;

	//Local project
	if(_isTome)
	{
		snprintf(basePath, sizeof(basePath), PROJECT_ROOT"%s/", projectPath);
	}
	else
	{
		if(selection % 10)
			snprintf(basePath, sizeof(basePath), PROJECT_ROOT"%s/Chapitre_%d.%d", projectPath, selection / 10, selection % 10);
		else
			snprintf(basePath, sizeof(basePath), PROJECT_ROOT"%s/Chapitre_%d", projectPath, selection / 10);
	}

	free(projectPath);

	//Main decompression cycle

	unzGoToFirstFile(archive);

	//We grab the number of file
	unz_global_info64 globalMeta;
	if(unzGetGlobalInfo64(archive, &globalMeta) != UNZ_OK)
		return;

	const char * startExpectedPath = [_path UTF8String];
	uint lengthExpected = strlen(startExpectedPath);

	for (uint pos = 0; pos < globalMeta.number_entry; pos++)
	{
		//Get current item filename
		char filename[1024] = {0};
		if((unzGetCurrentFileInfo64(archive, NULL, filename, sizeof(filename), NULL, 0, NULL, 0)) == UNZ_OK)
		{
			//If file is within the expected path (but not the dir itself)
			if(!strncmp(filename, startExpectedPath, lengthExpected) && filename[lengthExpected] != '\0')
				extractCurrentfile(archive, NULL, basePath, STRIP_PATH_FIRST, NULL);
		}

		unzGoToNextFile(archive);
	}

	//Decompression is over, now, we need to ensure everything is fine

	if(![self isReadable])
	{
		//Oh, the entry was not valid 😱
		logR("Uh? Invalid import :|");
		removeFolder(basePath);
	}
}

- (void) processThumbs : (unzFile *) archive
{

}

@end