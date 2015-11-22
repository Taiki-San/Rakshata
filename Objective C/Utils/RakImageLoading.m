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
	REPO_DATA fakeRepo = getEmptyRepoWithID(repoID);

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
	
	if(image != nil || defaultName == nil)
		return image;
	
	image = [NSImage imageNamed:defaultName];
	
	NSSize size = [image size];
	
	//Critical size by which, we write the name of the project on
	if (image != nil && size.width > 125 && size.width < 800)
	{
		image = [image copy];
		
		RakText * text = [[RakText alloc] initWithText:getStringForWchar(project.projectName) :[Prefs getSystemColor:COLOR_HIGHLIGHT]];
		if(text != nil)
		{
			text.font = [NSFont fontWithName:[Prefs getFontName:GET_FONT_TITLE] size:15];
			text.alignment = NSCenterTextAlignment;
			text.enableWraps = YES;
			text.fixedWidth = size.width * 3 / 4;
			text.enableMultiLine = YES;
			
			NSPoint point = NSCenterSize(size, text.bounds.size);
			NSSize oldSize = image.size;
			NSImage * textImage = [text imageOfView];
			size = textImage.size;
			
			[image lockFocus];
			[textImage drawInRect:(NSRect) {{round(point.x), round(point.y)}, size} fromRect:(NSRect) {NSZeroPoint, size} operation:NSCompositeDestinationAtop fraction:1.0];
			[image unlockFocus];
			
			image.size = oldSize;
		}
	}
	
	return image;
}

NSImage * enforceImageSize(NSImage * image, byte code)
{
	if(image == nil)
		return nil;

	NSSize standard = thumbSizeForID(code), retina = NSMakeSize(standard.width * 2, standard.height * 2);

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
	NSSize smallestSize = image.size;
	for(NSImageRep * rep in [image representations])
	{
		if(rep.size.width < smallestSize.width)
			smallestSize = rep.size;
	}

	return smallestSize;
}

NSImage * loadCTHeader(const PROJECT_DATA project)
{
	return enforceImageSize(loadProjectImage(project, PROJ_IMG_SUFFIX_HEAD, @"project_large"), THUMB_INDEX_HEAD);
}

NSImage * loadCTThumb(const PROJECT_DATA project)
{
	NSImage * image = enforceImageSize(loadProjectImage(project, PROJ_IMG_SUFFIX_CT, nil), THUMB_INDEX_CT);
	
	return image != nil ? image : loadImageGrid(project);
}

NSImage * loadDDThumbnail(const PROJECT_DATA project)
{
	return enforceImageSize(loadProjectImage(project, PROJ_IMG_SUFFIX_DD, @"defaultDragImage"), THUMB_INDEX_DD);
}

NSImage * loadImageGrid(const PROJECT_DATA project)
{
	return enforceImageSize(loadProjectImage(project, PROJ_IMG_SUFFIX_SRGRID, @"defaultSRImage"), THUMB_INDEX_SR);
}

NSSize thumbSizeForID(byte ID)
{
	switch(ID)
	{
		case THUMB_INDEX_SR:
			return NSMakeSize(150, 150);

		case THUMB_INDEX_SR2X:
			return NSMakeSize(300, 300);
			
		case THUMB_INDEX_DD:
			return NSMakeSize(50, 50);

		case THUMB_INDEX_DD2X:
			return NSMakeSize(100, 100);

		case THUMB_INDEX_CT:
			return NSMakeSize(100, CT_READERMODE_HEIGHT_PROJECT_IMAGE);

		case THUMB_INDEX_CT2X:
			return NSMakeSize(200, 2 * CT_READERMODE_HEIGHT_PROJECT_IMAGE);

		case THUMB_INDEX_HEAD:
			return NSMakeSize(960, 540);

		case THUMB_INDEX_HEAD2X:
			return NSMakeSize(1920, 1080);
	}

	return NSZeroSize;
}