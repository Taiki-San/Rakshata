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

@interface RakOutlineListItem : NSObject
{
	BOOL _isRootItem;
	uint _nbChildren;
	NSMutableArray * children;
	
	NSString * dataString;
}

@property BOOL expanded;
@property (readonly, getter=getHeight) CGFloat height;
@property (readonly, getter=isRootItem) BOOL rootItem;

- (uint) getNbChildren;

- (void) setChild : (id) child atIndex : (NSUInteger) index;
- (id) getChildAtIndex : (NSUInteger) index;

- (id) getData;
- (void) enumerateChildrenWithBlock : (void (^)(id child, uint indexChild, BOOL * stop))block;

@end
