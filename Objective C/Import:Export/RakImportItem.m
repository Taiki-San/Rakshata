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
	if(!_isTome)
		return checkReadable(_projectData.data.project, false, _contentID);

	//The volume must be in the list
	META_TOME * tomeFull = _projectData.data.project.tomesFull;
	uint lengthTomeFull = _projectData.data.project.nombreTomes;

	_projectData.data.project.tomesFull = _projectData.data.tomeLocal;
	_projectData.data.project.nombreTomes = _projectData.data.nombreTomeLocal;

	bool output = checkReadable(_projectData.data.project, true, _contentID);

	_projectData.data.project.tomesFull = tomeFull;
	_projectData.data.project.nombreTomes = lengthTomeFull;

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

	if(project.status == STATUS_INVALID)
		return true;

	return false;
}

- (BOOL) install : (id<RakImportIO>) IOControler withUI : (RakImportStatusController *) UI
{
	char * projectPath = getPathForProject(_projectData.data.project);
	if(projectPath == NULL)
		return false;

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
			snprintf(basePath, sizeof(basePath), PROJECT_ROOT"%s/"CHAPTER_PREFIX"%d.%d", projectPath, selection / 10, selection % 10);
		else
			snprintf(basePath, sizeof(basePath), PROJECT_ROOT"%s/"CHAPTER_PREFIX"%d", projectPath, selection / 10);
	}

	//Main decompression cycle
	if([IOControler respondsToSelector:@selector(willStartEvaluateFromScratch)])
		[IOControler willStartEvaluateFromScratch];

	__block BOOL foundOneThing = NO;
	NSString * basePathObj = [NSString stringWithUTF8String:basePath];

	[IOControler evaluateItemFromDir:_path withInitBlock:^(uint nbItems) {

		if(UI != nil)
			UI.nbElementInEntry = nbItems;

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

	//Decompression is over, now, we need to ensure everything is fine
	if((UI != nil && [UI haveCanceled]) || !foundOneThing || ![self isReadable])
	{
		//Oh, the entry was not valid 😱
		if(UI != nil && ![UI haveCanceled])
			logR("Uh? Invalid import :|");

		if(_isTome)
			snprintf(basePath, sizeof(basePath), PROJECT_ROOT"%s/"VOLUME_PREFIX"%d", projectPath, _contentID);

		removeFolder(basePath);
		free(projectPath);
		return false;
	}

	free(projectPath);
	return true;
}

- (BOOL) overrideDuplicate : (id<RakImportIO>) IOControler
{
	if(_issue != IMPORT_PROBLEM_DUPLICATE)
		return NO;

	[self deleteData];
	if(![self install:IOControler withUI:nil])
	{
		_issue = IMPORT_PROBLEM_INSTALL_ERROR;
		return NO;
	}

	[self processThumbs:IOControler];
	[self registerProject];
	_issue = IMPORT_PROBLEM_NONE;

	return YES;
}

- (BOOL) updateProject : (PROJECT_DATA) project withArchive : (id<RakImportIO>) IOControler
{
	_projectData.data.project = project;

	if([self needMoreData])
	{
		_issue = IMPORT_PROBLEM_DUPLICATE;
		return NO;
	}

	//Okay, perform installation
	if([self isReadable])
		_issue = IMPORT_PROBLEM_DUPLICATE;

	if(![self install:IOControler withUI:nil])
	{
		_issue = IMPORT_PROBLEM_INSTALL_ERROR;
		return NO;
	}

	[self processThumbs:IOControler];
	[self registerProject];
	_issue = IMPORT_PROBLEM_NONE;

	return YES;
}

- (void) deleteData
{
	if(!_isTome)
		return	internalDeleteCT(_projectData.data.project, false, _contentID);

	//The volume must be in the list
	META_TOME * tomeFull = _projectData.data.project.tomesFull;
	uint lengthTomeFull = _projectData.data.project.nombreTomes;

	_projectData.data.project.tomesInstalled = _projectData.data.project.tomesFull = _projectData.data.tomeLocal;
	_projectData.data.project.nombreTomesInstalled = _projectData.data.project.nombreTomes = _projectData.data.nombreTomeLocal;

	internalDeleteTome(_projectData.data.project, _contentID, true);

	_projectData.data.project.tomesFull = tomeFull;
	_projectData.data.project.nombreTomes = lengthTomeFull;
}

- (void) processThumbs : (id<RakImportIO>) IOControler
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

		[IOControler copyItemOfName:[NSString stringWithUTF8String:_projectData.URLImages[pos]] toPath:[NSString stringWithUTF8String:path.string]];
	}
}

- (NSData *) queryThumbIn : (id<RakImportIO>) IOControler withIndex : (uint) index
{
	if(index >= NB_IMAGES || !_projectData.haveImages[index])
		return nil;

	NSData * data = nil;

	if(![IOControler copyItemOfName:[NSString stringWithUTF8String:_projectData.URLImages[index]] toData:&data])
		return nil;

	return data;
}

- (void) registerProject
{
	_projectData.data.project.isInitialized = true;
	registerImportEntry(_projectData.data, _isTome);
}

@end