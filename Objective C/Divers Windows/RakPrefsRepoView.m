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
	BORDER = 5,
	WIDTH = 800,
	HEIGHT = 400,
	
	RADIO_OFFSET = 35,
	TEXT_RADIO_OFFSET = 5
};

@implementation RakPrefsRepoView

- (instancetype) init
{
	self = [self initWithFrame:[self frame]];
	
	if(self != nil)
	{
		activeElementInRoot = activeElementInSubRepo = UINT_MAX;

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
		
		radioSwitch = [[RakRadioButton alloc] init];
		if(radioSwitch != nil)
		{
			radioSwitch.target = self;
			radioSwitch.action = @selector(buttonClicked);
			
			[radioSwitch setFrameOrigin:NSMakePoint(RADIO_OFFSET, PREFS_REPO_BORDER_BELOW_LIST / 2 - radioSwitch.bounds.size.height / 2)];
			
			[self addSubview:radioSwitch];
			
			switchMessage = [[RakText alloc] initWithText:NSLocalizedString(@"PREFS-REPO-SWITCH-MESSAGE", nil) :[self textColor]];
			if(switchMessage != nil)
			{
				[switchMessage setFrameOrigin:NSMakePoint(NSMaxX(radioSwitch.frame) + TEXT_RADIO_OFFSET, PREFS_REPO_BORDER_BELOW_LIST / 2 - switchMessage.bounds.size.height / 2)];
				[self addSubview:switchMessage];
			}
		}
	}
	
	return self;
}

- (NSRect) frame
{
	return NSMakeRect(0, PREF_BUTTON_BAR_HEIGHT, WIDTH, HEIGHT);
}

- (NSRect) listFrame
{
	NSRect frame = _bounds;
	const CGFloat height = frame.size.height - PREFS_REPO_BORDER_BELOW_LIST;
	
	frame.origin.x = BORDER;
	frame.origin.y = frame.size.height - height - BORDER;

	frame.size.width = PREFS_REPO_LIST_WIDTH;
	frame.size.height = height;
	
	return frame;
}

- (NSColor *) textColor
{
	return [Prefs getSystemColor:GET_COLOR_SURVOL :nil];
}

#pragma mark - Data interface

- (void) buttonClicked
{
	BOOL isRoot = radioSwitch.state == NSOnState;
	
	[NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {

		details.animator.alphaValue = 0;
		list.rootMode = isRoot;

	} completionHandler:^{
		[self selectionUpdate:isRoot :isRoot ? activeElementInRoot : activeElementInSubRepo];
	}];
}

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
	uint pos = [self posOfParent:parentID];
	if(pos != UINT_MAX)
		return getStringForWchar(root[pos]->name);
	
	return @"";
}

- (uint) posOfParent : (uint) parentID
{
	if(root != NULL)
	{
		for(uint i = 0; i < nbRoot; i++)
		{
			if(root[i] != NULL && root[i]->repoID == parentID)
				return i;
		}
	}
	
	return UINT_MAX;
}

- (void) selectionUpdate : (BOOL) isRoot : (uint) index
{
	void ** _list = [self listForMode:isRoot];
	
	if(_list == NULL || index >= [self sizeForMode:isRoot])
	{
		if(self.alphaValue)
			details.animator.alphaValue = 0;
		return;
	}
	else if(isRoot)
		activeElementInRoot = index;
	else
		activeElementInSubRepo = index;
		
	
	if(list.rootMode != isRoot)
	{
		[radioSwitch setState : isRoot ? NSOnState : NSOffState];
		[NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
			list.rootMode = isRoot;
		} completionHandler:^{
			[list selectIndex : index];
		}];
	}
	
	if(details == nil)
	{
		details = [[RakPrefsRepoDetails alloc] initWithRepo:_bounds :isRoot :_list[index] : self];
		if(details != nil)
			[self addSubview: details];
	}
	else
		[details updateContent:isRoot :_list[index] : YES];
}

@end
