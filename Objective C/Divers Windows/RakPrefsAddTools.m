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

@implementation RakPrefsAddRepoItem

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
			_nbChildren = ((ROOT_REPO_DATA *) _data)->nombreSubrepo;
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

- (instancetype) getChildAtIndex:(NSInteger)index
{
	id output = nil;
	if(_isRootItem && index < _nbChildren && ([children count] >= index || (output = [children objectAtIndex:index]) == nil))
	{
		output = [[RakPrefsAddRepoItem alloc] initWithRepo:&(((ROOT_REPO_DATA *) _data)->subRepo[index]) :NO];
		[self setChild:output atIndex:index];
	}
	
	return output;
}

@end

@interface RakPrefsAddListCell()

@end

@implementation RakPrefsAddListCell

@end