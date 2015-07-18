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
				image = [bundle imageForResource:[NSString stringWithFormat: project.locale ? @""LOCAL_PATH_NAME"_%d_%s" : @"%d_%s", project.projectID, suffix]];
			
			free(repoPath);
		}
	}
	
	return image != nil ? image : (defaultName != nil ? [NSImage imageNamed:defaultName] : nil);
}

NSImage * enforceImageSize(NSImage * image, NSSize standard, NSSize retina)
{
	if(image == nil)
		return nil;

	if(NSEqualSizes(retina, NSZeroSize))
	{
		retina.height = standard.height * 2;
		retina.width = standard.width * 2;
	}

	for(NSImageRep * representation in image.representations)
	{
		NSInteger repBackingScaleRounded = floor(representation.pixelsWide / representation.size.width);
		if(repBackingScaleRounded >= 2)
			[representation setSize:retina];
		else
			[representation setSize:standard];

	}

	return image;
}

NSSize getThumbSize(NSImage * image)
{
	NSSize smallestSize = [[[image representations] objectAtIndex:0] size];
	for(NSImageRep * rep in [image representations])
	{
		if(rep.size.width < smallestSize.width)
			smallestSize = rep.size;
	}

	return smallestSize;
}

NSImage * loadCTHeader(const PROJECT_DATA project)
{
	return enforceImageSize(loadProjectImage(project, PROJ_IMG_SUFFIX_HEAD, @"project_large"), NSMakeSize(1000, 563), NSMakeSize(2000, 1125));
}

NSImage * loadCTThumb(const PROJECT_DATA project)
{
	NSImage * image = enforceImageSize(loadProjectImage(project, PROJ_IMG_SUFFIX_CT, nil), NSMakeSize(100, CT_READERMODE_HEIGHT_PROJECT_IMAGE), NSZeroSize);
	
	return image != nil ? image : loadImageGrid(project);
}

NSImage * loadDDThumbnail(const PROJECT_DATA project)
{
	return enforceImageSize(loadProjectImage(project, PROJ_IMG_SUFFIX_DD, @"defaultDragImage"), NSMakeSize(50, 50), NSZeroSize);
}

NSImage * loadImageGrid(const PROJECT_DATA project)
{
	return enforceImageSize(loadProjectImage(project, PROJ_IMG_SUFFIX_SRGRID, @"defaultSRImage"), NSMakeSize(150, 150), NSZeroSize);
}