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
			return (NSInteger) [self.arrangedObjects count] - 1 - selectedIndex;
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
		_flipped = flipped;
		self.selectedIndex = self.selectedIndex;
	}
}

@end
