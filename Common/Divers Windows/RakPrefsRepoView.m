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

enum
{
	BORDER = 5,
	WIDTH = 800,
	HEIGHT = 410,
	
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
			[RakDBUpdate registerForUpdate:self :@selector(repoUpdated:)];
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
		else
		{
			[list reloadContent:NO];
		}
		
		radioSwitch = [[RakSwitchButton alloc] init];
		if(radioSwitch != nil)
		{
			radioSwitch.target = self;
			radioSwitch.action = @selector(buttonClicked);
			
			[radioSwitch setFrameOrigin:NSMakePoint(RADIO_OFFSET, PREFS_REPO_BORDER_BELOW_LIST / 2 - radioSwitch.bounds.size.height / 2 + 3)];
			
			[self addSubview:radioSwitch];
			
			switchMessage = [[RakText alloc] initWithText:NSLocalizedString(@"PREFS-REPO-SWITCH-MESSAGE", nil) :[self textColor]];
			if(switchMessage != nil)
			{
				switchMessage.clicTarget = self;
				switchMessage.clicAction = @selector(textClicked);
				
				[switchMessage setFrameOrigin:NSMakePoint(NSMaxX(radioSwitch.frame) + TEXT_RADIO_OFFSET, PREFS_REPO_BORDER_BELOW_LIST / 2 - switchMessage.bounds.size.height / 2 + 3)];
				[self addSubview:switchMessage];
			}
		}
		
		placeholder = [[RakText alloc] initWithText:NSLocalizedString(@"PREFS-REPO-PLACEHOLDER-TUTORIAL", nil) :[self placeholderColor]];
		if(placeholder != nil)
		{
			[placeholder setFont:[Prefs getFont:FONT_PLACEHOLDER ofSize:13]];
			[placeholder.cell setWraps:YES];
			
			CGFloat availableWidth = (WIDTH - BORDER - PREFS_REPO_LIST_WIDTH);
			placeholder.fixedWidth = availableWidth * 0.8;
			
			[placeholder setFrameOrigin:NSMakePoint(BORDER + PREFS_REPO_LIST_WIDTH + availableWidth / 10, HEIGHT / 2 - placeholder.bounds.size.height / 2)];
			
			[self addSubview:placeholder];
		}
		
		[Prefs registerForChange:self forType:KVO_THEME];
	}
	
	return self;
}

- (void) dealloc
{
	[RakDBUpdate unRegister:self];
	[Prefs deRegisterForChange:self forType:KVO_THEME];
}

- (void) mouseUp:(NSEvent *)theEvent
{
	if(details.alphaValue != 0)
	{
		[self selectionUpdate:list.rootMode :UINT_MAX];
	}
}

#pragma mark - UI

- (NSRect) frame
{
	return NSMakeRect(0, PREF_BUTTON_BAR_HEIGHT, WIDTH, HEIGHT);
}

- (NSRect) listFrame
{
	NSRect frame = _bounds;
	const CGFloat height = frame.size.height - PREFS_REPO_BORDER_BELOW_LIST - BORDER;
	
	frame.origin.x = BORDER;
	frame.origin.y = frame.size.height - height;
	
	frame.size.width = PREFS_REPO_LIST_WIDTH;
	frame.size.height = height;
	
	return frame;
}

- (RakColor *) textColor
{
	return [Prefs getSystemColor:COLOR_SURVOL];
}

- (RakColor *) placeholderColor
{
	return [Prefs getSystemColor:COLOR_REPO_TEXT_PLACEHOLDER];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if ([object class] != [Prefs class])
		return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	
	placeholder.textColor = [self placeholderColor];
	switchMessage.textColor = [self textColor];
}

#pragma mark - Data interface

- (void) textClicked
{
	[radioSwitch performClick:nil];
}

- (void) buttonClicked
{
	BOOL isRoot = radioSwitch.state == NSOnState;
	
	[NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
		
		details. alphaAnimated = 0;
		placeholder. alphaAnimated = 1;
		
	} completionHandler:^{
		[self selectionUpdate:isRoot :isRoot ? activeElementInRoot : activeElementInSubRepo];
	}];
}

- (void *) dataForMode : (BOOL) rootMode index : (uint) index
{
	if((rootMode ? nbRoot : nbRepo) <= index)
		return NULL;
	
	if(rootMode)
		return ((ROOT_REPO_DATA **) root)[index];
	
	return ((REPO_DATA **) repo)[index];
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
	void * _data = [self dataForMode:isRoot index:index];
	
	_selectedIsRoot = isRoot;

	if(_data == NULL)
	{
		if(details.alphaValue)
		{
			details. alphaAnimated = 0;
			placeholder. alphaAnimated = 1;
		}
		
		if(list.rootMode != isRoot)
			list.rootMode = isRoot;
		else
			[list resetSelection:nil];
		
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
	
	if(placeholder.alphaValue)
		placeholder. alphaAnimated = 0;
	
	if(details == nil)
	{
		details = [[RakPrefsRepoDetails alloc] initWithRepo:_bounds :isRoot :_data : self];
		if(details != nil)
			[self addSubview: details];
	}
	else
		[details updateContent:isRoot :_data : YES];
}

#pragma mark - Data update

- (void) repoUpdated : (NSNotification *) notification
{
	if(![RakDBUpdate isFullRepoUpdate:notification.userInfo])
	{
		if(![RakDBUpdate getUpdatedRepo:notification.userInfo :NULL])
			return;
	
//		BOOL isRootUpdate = updatedRepoID < UINT_MAX, foundRepo = false;
//		uint selectedLine;
//		void * dataChunck[2] = {NULL, NULL};	//We get the field we removed, as we should release it before flushing it from list and details
//		
//		//The repo ID of a subRepo is [32 integer > 1 (root ID)] << 33 | [32 integer > 1 (sub ID)]
//		
//		if(isRootUpdate)
//		{
//			for(selectedLine = 0; selectedLine < nbRoot; selectedLine++)
//			{
//				if(root[selectedLine]->repoID == updatedRepoID)
//				{
//					ROOT_REPO_DATA * rootRepo = getRootRepoForID(updatedRepoID);
//					
//					if(rootRepo != NULL)
//					{
//						dataChunck[0] = root[selectedLine];
//						root[selectedLine] = rootRepo;
//					}
//					else
//					{
//						ROOT_REPO_DATA * data = root[selectedLine];
//						
//						for(uint i = selectedLine; i < selectedLine; i++)
//							root[selectedLine] = root[selectedLine + 1];
//						
//						freeSingleRootRepo(data);
//						nbRoot--;
//					}
//					
//					foundRepo = true;
//					break;
//				}
//			}
//		}
//		else
//		{
//			for(selectedLine = 0; selectedLine < nbRepo; selectedLine++)
//			{
//				if(getRepoID(repo[selectedLine]) == updatedRepoID)
//				{
//					REPO_DATA * repoData = getRepoForID(updatedRepoID);
//					
//					if(repoData != NULL)
//					{
//						dataChunck[0] = repo[selectedLine];
//						repo[selectedLine] = repoData;
//					}
//					else
//					{
//						REPO_DATA * data = repo[selectedLine];
//						
//						for(uint i = selectedLine; i < selectedLine; i++)
//							repo[selectedLine] = repo[selectedLine + 1];
//						
//						free(data);
//						nbRepo--;
//					}
//					
//					foundRepo = true;
//					break;
//				}
//			}
//		}
//		
//		if(!foundRepo)
//			selectedLine = UINT_MAX;
//		
//		if(dataChunck[0] != NULL)			//We found something, we update the list
//		{
//			if(list.rootMode == isRootUpdate)
//				[list updateContentAtLine:selectedLine];
//			
//			if(_selectedIsRoot == isRootUpdate && (_selectedIsRoot ? activeElementInRoot : activeElementInSubRepo) == selectedLine)
//				[self selectionUpdate:_selectedIsRoot :(_selectedIsRoot ? activeElementInRoot : activeElementInSubRepo)];
//		}
//		else if(selectedLine != UINT_MAX) 	//Repo was removed
//		{
//			if(list.rootMode == isRootUpdate)
//				[list removeContentAtLine:selectedLine];
//
//			if(_selectedIsRoot == isRootUpdate && (_selectedIsRoot ? activeElementInRoot : activeElementInSubRepo) == selectedLine)
//				[self selectionUpdate:_selectedIsRoot :UINT_MAX];	//UINT_MAX > the number of repo/root we have, so dataForMode will return NULL, and flush the detail
//		}
//
//		return;
	}
	
	uint newNbRoot, newNbRepo;
	ROOT_REPO_DATA ** newRoot = (ROOT_REPO_DATA **) getCopyKnownRepo(&newNbRoot, true);
	REPO_DATA ** newRepo = (REPO_DATA **) getCopyKnownRepo(&newNbRepo, false);
	
	if(newRoot == NULL || newRepo == NULL)
	{
		freeRootRepo(newRoot);	newRoot = NULL;
		freeRepo(newRepo);		newRepo = NULL;
		return;
	}

	qsort(newRoot, newNbRoot, sizeof(ROOT_REPO_DATA *), sortRootRepo);
	qsort(newRepo, newNbRepo, sizeof(REPO_DATA *), sortRepo);
	
	//We update the pointers
	if(activeElementInRoot < nbRoot)
	{
		uint i = 0;
		
		for(; i < newNbRoot && newRoot[i]->repoID != root[activeElementInRoot]->repoID; i++);
		
		if(i < newNbRoot)
			activeElementInRoot = i;
		else
			activeElementInRoot = UINT_MAX;
	}
	else
		activeElementInRoot = UINT_MAX;
	
	if(activeElementInSubRepo < nbRepo)
	{
		uint i = 0;
		uint64_t oldRepoID = getRepoID(repo[activeElementInSubRepo]);
		
		for(; i < newNbRepo && getRepoID(newRepo[i]) != oldRepoID; i++);
		
		if(i < newNbRoot)
			activeElementInSubRepo = i;
		else
			activeElementInSubRepo = UINT_MAX;
	}
	else
		activeElementInSubRepo = UINT_MAX;
	
	freeRootRepo(root);		root = newRoot;		nbRoot = newNbRoot;
	freeRepo(repo);			repo = newRepo;		nbRepo = newNbRepo;

	[list reloadContent:list.rootMode];
	
	if((_selectedIsRoot && activeElementInRoot != UINT_MAX) || (!_selectedIsRoot && activeElementInSubRepo != UINT_MAX))
	{
		[self selectionUpdate:_selectedIsRoot :_selectedIsRoot ? activeElementInRoot : activeElementInSubRepo];
	}
}

@end
