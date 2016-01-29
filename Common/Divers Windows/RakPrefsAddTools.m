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

@implementation RakAddRepoItem

- (instancetype) initWithRepo : (void *) data : (BOOL) isRoot
{
	self = [self init];
	
	if(self != nil)
	{
		_data = data;
		_isRootItem = isRoot;
		
		if(_isRootItem)
		{
			self.expanded = YES;
			dataString = getStringForWchar(((ROOT_REPO_DATA *) _data)->name);
			_nbChildren = ((ROOT_REPO_DATA *) _data)->nbSubrepo;
		}
		else
		{
			dataString = getStringForWchar(((REPO_DATA *) _data)->name);
		}
	}
	
	return self;
}

- (void *) getRepo
{
	return _data;
}

- (instancetype) getChildAtIndex:(NSUInteger)index
{
	id output = nil;
	if(_isRootItem && index < _nbChildren && ([children count] >= index || (output = [children objectAtIndex:index]) == nil))
	{
		output = [[RakAddRepoItem alloc] initWithRepo:&(((ROOT_REPO_DATA *) _data)->subRepo[index]) :NO];
		[self setChild:output atIndex:index];
	}
	
	return output;
}

@end

@implementation RakAddRepoListItemView

- (instancetype) initWithFrame:(NSRect)frameRect
{
	self = [super initWithFrame:frameRect];
	
	if(self != nil)
	{
		self.wantActivationState = YES;
		
		//We also listen to a notification sent when our item get updated
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(refreshButtonState) name:REFRESH_ADD_REPO_ITEM_STATUS object:nil];
	}
	
	return self;
}

- (void) dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void) updateContent:(BOOL)isCompact :(BOOL)isDetailColumn :(BOOL)isRoot :(void *)repo :(NSString *)detailString
{
	_unloaded = NO;
	
	[super updateContent:isCompact :isDetailColumn :isRoot :repo :detailString];
}

- (void) refreshButtonState
{
	if(_unloaded)
		return;
	
	if(self.isRoot)
	{
		ROOT_REPO_DATA * _repo = (void*) _repoUsedInDetail;
		BOOL haveOne = NO, allAreActive = YES;
		
		for(uint i = 0; i < _repo->nbSubrepo; i++)
		{
			haveOne |= _repo->subRepo[i].active;
			allAreActive &= _repo->subRepo[i].active;
		}
		
		activationButton.state = allAreActive ? NSOnState : (haveOne ? NSMixedState : NSOffState);
	}
	else
		[super refreshButtonState];
}

@end