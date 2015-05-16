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

@implementation RakSerieListItem

- (instancetype) init : (void*) data : (BOOL) isRootItem : (int) initStage : (uint) nbChildren
{
	self = [super init];
	
	if(self != nil)
	{
		_isRootItem = isRootItem;
		
		if(_isRootItem)
		{
			children = [[NSMutableArray alloc] init];
			dataChild = getEmptyProject();
			self.expanded = YES;
			_isRecentList = _isDLList = _isMainList = NO;
			_nbChildren = nbChildren;
			
			switch (initStage)
			{
				case INIT_FIRST_STAGE:
				{
					_isRecentList = YES;
					dataString = NSLocalizedString(@"RECENT-OPEN", nil);
					break;
				}
					
				case INIT_SECOND_STAGE:
				{
					_isDLList = YES;
					dataString = NSLocalizedString(@"RECENT-DL", nil);
					break;
				}
					
				case INIT_THIRD_STAGE:
				{
					_isMainList = YES;
					dataString = NSLocalizedString(@"FULL-LIST", nil);
					break;
				}
			}
		}
		else
		{
			dataString = nil;
			
			if(data == NULL && initStage == INIT_FINAL_STAGE)
				_isMainList = YES;
			
			else
			{
				_isMainList = NO;
				
				if(data != NULL)
				{
					dataChild	= *(PROJECT_DATA *) data;
					releaseCTData(dataChild);
				}
				
				dataChild.chapitresFull = dataChild.chapitresInstalled = NULL;
				dataChild.tomesFull = dataChild.tomesInstalled = NULL;
				dataChild.chapitresPrix = NULL;
			}
		}
	}
	
	return self;
}

- (BOOL) isRecentList
{
	return _isRecentList;
}

- (BOOL) isDLList
{
	return _isDLList;
}

- (BOOL) isMainList
{
	return _isMainList;
}

- (void) setMainListHeight : (CGFloat) height
{
	_mainListHeight = height;
}

- (void) resetMainListHeight
{
	if([self isRootItem] && [self isMainList])
	{
		if([children count] == 1)
		{
			RakSerieListItem *mainList = [children objectAtIndex:0];
			[mainList setMainListHeight:0];
		}
	}
	else
		NSLog(@"Invalid request, I'm not the item you're looking for...");
}

- (CGFloat) getHeight
{
	if([self isRootItem])
		return 21;
	else if([self isMainList] && _mainListHeight)
		return _mainListHeight;
	
	return 0;
}

- (void) setNbChildren : (uint) nbChildren : (BOOL) flush
{
	_nbChildren = nbChildren;
	
	if(flush)
		[children removeAllObjects];
}

- (id) getData
{
	if(!_isRootItem && dataChild.isInitialized)
		return getStringForWchar(dataChild.projectName);
	
	return [super getData];
}

- (PROJECT_DATA) getRawDataChild
{
	if (_isRootItem || _isMainList)
		return getEmptyProject();
	
	else
		return dataChild;
}

@end