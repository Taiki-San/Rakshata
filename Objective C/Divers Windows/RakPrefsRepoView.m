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

enum
{
	BORDER = 5
};

@implementation RakPrefsRepoView

- (instancetype) init
{
	self = [self initWithFrame:[self frame]];
	
	if(self != nil)
	{
		root = (ROOT_REPO_DATA **) getCopyKnownRepo(&nbRoot, true);
		repo = (REPO_DATA **) getCopyKnownRepo(&nbRepo, false);
		
		if(root == NULL || repo == NULL)
		{
			freeRootRepo(root);	root = NULL;
			freeRepo(repo);		repo = NULL;
			nbRoot = nbRepo = 0;
		}
		else
		{
			qsort(root, nbRoot, sizeof(ROOT_REPO_DATA *), sortRootRepo);
			qsort(repo, nbRepo, sizeof(REPO_DATA *), sortRepo);
		}
		
		list = [RakPrefsRepoList alloc];
		if(list != nil)
		{
			list.responder = self;
			list.rootMode = NO;;
			
			list = [list initWithFrame:[self listFrame]];
			
			[self addSubview:[list getContent]];
		}
	}
	
	return self;
}

- (NSRect) frame
{
	return NSMakeRect(0, PREF_BUTTON_BAR_HEIGHT, 800, 500);
}

- (NSRect) listFrame
{
	NSRect frame = _bounds;
	const CGFloat height = frame.size.height * 0.7;
	
	frame.origin.x = BORDER;
	frame.origin.y = frame.size.height - height - BORDER;

	frame.size.width *= 0.6;
	frame.size.height = height;
	
	return frame;
}

#pragma mark - Data interface

- (void **) listForMode : (BOOL) rootMode
{
	return rootMode ? (void**) root : (void**) repo;
}

- (uint) sizeForMode : (BOOL) rootMode
{
	return rootMode ? nbRoot : nbRepo;
}

- (NSString *) nameOfParent : (uint) parentID
{
	if(root != NULL)
	{
		for(uint i = 0; i < nbRoot; i++)
		{
			if(root[i] != NULL && root[i]->repoID == parentID)
				return getStringForWchar(root[i]->name);
		}
	}
	
	return @"";
}

- (void) selectionUpdate : (BOOL) isRoot : (uint) index
{
	void ** _list = [self listForMode:isRoot];
	
	if(_list == NULL || index >= [self sizeForMode:isRoot])
		return;
	
	NSLog(@"Clicked %@", getStringForWchar(isRoot ? ((ROOT_REPO_DATA **) _list)[index]->name : ((REPO_DATA **) _list)[index]->name));
}

@end
