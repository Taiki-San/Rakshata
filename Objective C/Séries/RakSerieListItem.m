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

- (id) init : (void*) data : (BOOL) isRootItem : (int) initStage : (uint) nbChildren
{
	self = [super init];
	
	if(self != nil)
	{
		_isRootItem = isRootItem;
		
		if(_isRootItem)
		{
			children = [[NSMutableArray alloc] init];
			dataChild	= NULL;
			_expanded = YES;
			_isRecentList = _isDLList = _isMainList = NO;
			_nbChildren = nbChildren;
			
			switch (initStage)
			{
				case INIT_FIRST_STAGE:
				{
					_isRecentList = YES;
					dataRoot = @"Consultées récemment";
					break;
				}
					
				case INIT_SECOND_STAGE:
				{
					_isDLList = YES;
					dataRoot = @"Téléchargées récemment";
					break;
				}
					
				case INIT_THIRD_STAGE:
				{
					_isMainList = YES;
					dataRoot = @"Liste complète";
					break;
				}
			}
		}
		else
		{
			dataRoot	= nil;
			dataChild	= data;
			
			if(dataChild == nil && initStage == INIT_FINAL_STAGE)
				_isMainList = YES;
			else
				_isMainList = NO;
		}
	}
	
	return self;
}

- (BOOL) isRootItem
{
	return _isRootItem;
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
		return 25;
	else if([self isMainList] && _mainListHeight)
		return _mainListHeight;
	
	return 0;
}

- (void) setExpaded : (BOOL) expanded
{
	_expanded = expanded;
}

- (BOOL) isExpanded
{
	return _expanded;
}

- (uint) getNbChildren
{
	if([self isRootItem])
		return _nbChildren;
	return 0;
}

- (void) setChild : (id) child atIndex : (NSInteger) index
{
	if(children != nil)
	{
		[children insertObject:child atIndex:index];
	}
}

- (id) getChildAtIndex : (NSInteger) index
{
	if(children != nil && [children count] > index)
		return [children objectAtIndex:index];
	
	return nil;
}

- (PROJECT_DATA*) getRawDataChild
{
	if (_isRootItem || _isMainList)
		return NULL;
	else
		return dataChild;
}

- (NSString*) getData
{
	if(_isRootItem && dataRoot != NULL)
		return dataRoot;
	else if(!_isRootItem && dataChild != NULL)
		return [[NSString alloc] initWithData:[NSData dataWithBytes:dataChild->projectName length:sizeof(dataChild->projectName)] encoding:NSUTF32LittleEndianStringEncoding];
	else
		return @"Internal error :(";
}

@end