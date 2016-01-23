/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

@implementation RakOutlineListItem

- (instancetype) init
{
	self = [super init];
	
	if(self != nil)
	{
		children = [[NSMutableArray alloc] init];
	}
	
	return self;
}

- (BOOL) isRootItem
{
	return _isRootItem;
}

- (uint) getNbChildren
{
	return self.isRootItem ? _nbChildren : 0;
}

- (void) setChild : (id) child atIndex : (NSUInteger) index
{
	if(children != nil)
	{
		[children insertObject:child atIndex:index];
	}
}

- (id) getChildAtIndex : (NSUInteger) index
{
	if(children != nil && [children count] > index)
		return [children objectAtIndex:index];
	
	return nil;
}

- (id) getData
{
	return dataString;
}

- (void) enumerateChildrenWithBlock : (void (^)(id child, uint indexChild, BOOL * stop))block
{
	if(!_isRootItem)
		return;
	
	[children enumerateObjectsUsingBlock:^(id  _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop)
	{
		block(obj, idx, stop);
	}];
}

@end
