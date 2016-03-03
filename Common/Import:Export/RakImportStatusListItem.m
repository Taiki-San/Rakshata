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

@interface RakImportStatusListItem()
{
	BOOL cachedMetadataProblem;
}

@end

@implementation RakImportStatusListItem

- (instancetype) initWithProject : (PROJECT_DATA) project
{
	self = [super init];

	if(self != nil)
	{
		_isRootItem = YES;
		nullifyCTPointers(&project);
		_projectData = project;

		children = [NSMutableArray new];

		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(checkRefreshStatusRoot) name:NOTIFICATION_ROOT object:nil];
	}

	return self;
}

- (instancetype) initWithItem : (RakImportItem *) item
{
	self = [super init];

	if(self != nil)
	{
		_isRootItem = NO;
		_itemForChild = item;
		_projectData = _itemForChild.projectData.data.project;
		_status = self.itemForChild.issue == IMPORT_PROBLEM_NONE ? STATUS_BUTTON_OK : STATUS_BUTTON_ERROR;

		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(checkRefreshStatusChild) name:NOTIFICATION_CHILD object:nil];
	}

	return self;
}

- (void) commitFinalList
{
	[children sortUsingComparator:^NSComparisonResult(RakImportStatusListItem * obj1, RakImportStatusListItem * obj2) {

		RakImportItem * first = obj1.itemForChild, * second = obj2.itemForChild;

		if(first.isTome ^ second.isTome)
			return first.isTome ? NSOrderedAscending : NSOrderedDescending;

		return first.contentID > second.contentID ? NSOrderedDescending : NSOrderedAscending;
	}];

	_nbChildren = [children count];
	_status = [self checkStatusFromChildren];
}

- (byte) checkStatusFromChildren
{
	BOOL anythingWrong = NO;
	cachedMetadataProblem = NO;

	for(RakImportStatusListItem * item in children)
	{
		BOOL itemStatus = item.itemForChild.issue != IMPORT_PROBLEM_NONE;

		anythingWrong |= itemStatus;

		if(itemStatus && item.itemForChild.issue == IMPORT_PROBLEM_METADATA)
			cachedMetadataProblem = YES;
	}

	return anythingWrong ? (cachedMetadataProblem ? STATUS_BUTTON_ERROR : STATUS_BUTTON_WARN) : STATUS_BUTTON_OK;
}

- (BOOL) metadataProblem
{
	return _isRootItem ? cachedMetadataProblem : _itemForChild.issue == IMPORT_PROBLEM_METADATA;
}

#pragma mark - Kindergarten

- (void) addItemAsChild : (RakImportItem *) item
{
	[children addObject:[[RakImportStatusListItem alloc] initWithItem:item]];
	_nbChildren = [children count];
}

- (void) insertItemAsChild : (RakImportItem *) item atIndex : (uint) index
{
	[children insertObject:[[RakImportStatusListItem alloc] initWithItem:item] atIndex:index];
	_nbChildren = [children count];
}

- (BOOL) removeChild : (RakImportItem *) item
{
	if(!_isRootItem)
		return NO;
	
	__block uint itemIndex = INVALID_VALUE;
	
	[children enumerateObjectsUsingBlock:^(RakImportStatusListItem * _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
		
		if([item isEqualTo:obj.itemForChild])
		{
			itemIndex = idx;
			*stop = YES;
		}
	}];
	
	if(itemIndex == INVALID_VALUE)
		return NO;
	
	_indexOfLastRemoved = itemIndex;
	
	[children removeObjectAtIndex:itemIndex];
	_nbChildren = [children count];
	[self checkRefreshStatusRoot];
	
	return YES;
}

- (void) sortChildren
{
	
}

#pragma mark - Menu management

- (BOOL) canMoveToIndependentNode
{
	return !_isRootItem;
}

- (BOOL) canConvertToVol
{
	if(!_isRootItem)
		return !_itemForChild.isTome;

	for(RakImportStatusListItem * item in children)
	{
		if([item canConvertToVol])
			return YES;
	}

	return NO;
}

- (void) convertToVol
{
	[self convertToCT:YES];
}

- (BOOL) canConvertToChap
{
	if(!_isRootItem)
		return _itemForChild.isTome;
	
	for(RakImportStatusListItem * item in children)
	{
		if([item canConvertToChap])
			return YES;
	}
	
	return NO;
}

- (void) convertToChap
{
	[self convertToCT:YES];
}

- (void) convertToCT : (BOOL) wantIsTome
{
	if(_isRootItem)
	{
		for(RakImportStatusListItem * _item in children)
		{
			RakImportItem * item = _item.itemForChild;
			if(item.isTome != wantIsTome)
			{
				[item _updateCTIDWith:item.contentID tomeName:nil isTome:wantIsTome];
			}
		}
		
	}
	else
		[_itemForChild _updateCTIDWith:_itemForChild.contentID tomeName:nil isTome:wantIsTome];
	
	[[NSNotificationCenter defaultCenter] postNotificationName:NOTIFICATION_IMPORT_STATUS_UI object:nil];
}

- (void) moveToIndependentNode
{
	[[NSNotificationCenter defaultCenter] postNotificationName:NOTIFICATION_IMPORT_NEED_CALLBACK object:nil userInfo:@{@"item":self, @"payload":@(1)}];
}

- (void) removeItem
{
	[[NSNotificationCenter defaultCenter] postNotificationName:NOTIFICATION_IMPORT_NEED_CALLBACK object:nil userInfo:@{@"item":self, @"payload":@(2)}];
}

#pragma mark - Content update

- (void) checkRefreshStatusChild
{
	_status = _itemForChild.issue == IMPORT_PROBLEM_NONE ? STATUS_BUTTON_OK : STATUS_BUTTON_ERROR;
}

- (void) checkRefreshStatusRoot
{
	if(_nbChildren > 0)
	{
		PROJECT_DATA project = [[children firstObject] itemForChild].projectData.data.project;
		nullifyCTPointers(&project);
		_projectData = project;
	}
	
	_status = [self checkStatusFromChildren];
	
	if(_status == STATUS_BUTTON_WARN)
		[[NSNotificationCenter defaultCenter] postNotificationName:NOTIFICATION_IMPORT_OPEN_ITEM object:self];
}

@end