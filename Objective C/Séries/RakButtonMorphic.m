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

@implementation RakButtonMorphic

+ (instancetype) allocImages : (NSArray*) imageNames : (short) defaultState : (id) target : (NSArray *) selectors
{
	if(imageNames == nil || [imageNames count] == 0 || target == nil || [imageNames count] != [selectors count])
		return nil;
	
	RakButtonMorphic * output = [self allocImageWithBackground:imageNames[0] :defaultState :target : NSSelectorFromString(selectors[0])];
	
	if(output != nil)
	{
		output = [output initImages : (NSArray*) imageNames : (short) defaultState : (id) target : selectors];
	}
	
	return output;
}

- (instancetype) initImages : (NSArray*) imageNames : (short) defaultState : (id) target : (NSArray *) selectors
{
	NSMutableArray * _cells = [NSMutableArray array];
	[_cells addObject:self.cell];
	
	RakButtonCell * cell;
	BOOL firstPass = YES;
	for(NSString * imageName in imageNames)
	{
		if(firstPass)
			firstPass = !firstPass;
		else
		{
			cell = [[[[self class] cellClass] alloc] initWithPage:imageName :defaultState];
			if(cell != nil)
				[_cells addObject:cell];
			else
				return nil;
		}
	}
	
	_activeCell = 0;
	
	_defaultState = defaultState;
	_target = target;
	_selectors = selectors;
	
	return self;
}

#pragma mark - Active Cell property

- (uint) activeCell
{
	return _activeCell;
}

- (void) setActiveCell : (uint) activeCell
{
	if(activeCell <= [_cell count])
	{
		self.cell = _cell[activeCell];
		self.action = NSSelectorFromString(_selectors[activeCell]);
		
		[self.cell setState:_defaultState];
		
		_activeCell = activeCell;
	}
}

@end
