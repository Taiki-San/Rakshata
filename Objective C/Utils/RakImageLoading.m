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

#pragma mark - Repo

NSImage * loadImageForRepo(BOOL isRoot, void* repo)
{
	NSImage * image = nil;
	char * repoPath = isRoot ? getPathForRootRepo(repo) : getPathForRepo(repo);
	
	if(repoPath != NULL)
	{
		NSBundle * bundle = [NSBundle bundleWithPath: [NSString stringWithFormat:@""IMAGE_CACHE_DIR"/%s/", repoPath]];
		if(bundle != nil)
			image = [bundle imageForResource:[NSString stringWithUTF8String:REPO_IMG_NAME]];
		
		free(repoPath);
	}
	
	return image == nil ? [NSImage imageNamed: isRoot ? @"defaultRepoRoot" : @"defaultRepo"] : image;
}

#pragma mark - Project

void invalidateCacheForRepoID(uint64_t repoID)
{
	REPO_DATA fakeRepo = getEmptyRepo();	fakeRepo.parentRepoID = getRootFromRepoID(repoID);	fakeRepo.repoID = getSubrepoFromRepoID(repoID);

	char * repoPath = getPathForRepo(&fakeRepo);
	if(repoPath != NULL)
	{
		NSBundle * bundle = [NSBundle bundleWithPath: [NSString stringWithFormat:@""IMAGE_CACHE_DIR"/%s/", repoPath]];
		if(bundle != nil)
			flushBundleCache(bundle);
		
		free(repoPath);
	}
}

NSImage * loadProjectImage(const PROJECT_DATA project, const char * suffix, NSString * defaultName)
{
	NSImage * image = nil;
	
	if(project.isInitialized)
	{
		char * repoPath = getPathForRepo(project.repo);
		
		if(repoPath != NULL)
		{
			NSBundle * bundle = [NSBundle bundleWithPath: [NSString stringWithFormat:@""IMAGE_CACHE_DIR"/%s/", repoPath]];
			if(bundle != nil)
				image = [bundle imageForResource:[NSString stringWithFormat:@"%d_%s", project.projectID, suffix]];
			
			free(repoPath);
		}
	}
	
	return image == nil ? [NSImage imageNamed:defaultName] : image;
}

NSImage * loadCTHeader(const PROJECT_DATA project)
{
	return loadProjectImage(project, PROJ_IMG_SUFFIX_HEAD, @"project_large");
}

NSImage * loadCTThumb(const PROJECT_DATA project)
{
	return loadProjectImage(project, PROJ_IMG_SUFFIX_CT, @"defaultCTImage");
}

NSImage * loadDDThumbnail(const PROJECT_DATA project)
{
	return loadProjectImage(project, PROJ_IMG_SUFFIX_DD, @"defaultDragImage");
}

NSImage * loadImageGrid(const PROJECT_DATA project)
{
	return loadProjectImage(project, PROJ_IMG_SUFFIX_SRGRID, @"defaultSRImage");
}