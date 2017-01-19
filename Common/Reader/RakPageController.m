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
 ********************************************************************************************/

@implementation RakPageController

- (void) setPatchedSelectedIndex : (NSInteger) patchedSelectedIndex
{
	if(_flipped)
	{
		NSInteger count = (NSInteger) [self.arrangedObjects count];
		if(patchedSelectedIndex < count)
			self.selectedIndex = count - 1 - patchedSelectedIndex;
		else
			self.selectedIndex = 0;
	}
	else
		self.selectedIndex = patchedSelectedIndex;
}

- (NSInteger) patchedSelectedIndex
{
	if(_flipped)
	{
		NSInteger count = (NSInteger) [self.arrangedObjects count], selectedIndex = [super selectedIndex];
		if(selectedIndex < count)
			return (NSInteger) count - 1 - selectedIndex;
		else
			return 0;
	}
	
	return [super selectedIndex];
}

- (NSUInteger) getPatchedPosForIndex : (NSUInteger) index
{
	if(_flipped)
	{
		NSUInteger count = [self.arrangedObjects count], selectedIndex = index;
		if(selectedIndex < count)
			return count - 1 - selectedIndex;
	}
	
	return index;
}

- (void) setFlipped : (BOOL) flipped
{
	if(_flipped != flipped)
	{
		NSInteger patched = self.patchedSelectedIndex;
		
		_flipped = flipped;
		
		if(self.arrangedObjects.count != 0)
		{
			self.arrangedObjects = [self.arrangedObjects revertArray];
			self.patchedSelectedIndex = patched;
		}
	}
}

@end
