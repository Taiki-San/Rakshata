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

+ (instancetype) allocImages : (NSArray*) imageNames : (short) defaultState : (id) target : (SEL) selector
{
	if(imageNames == nil || [imageNames count] == 0 || target == nil)
		return nil;
	
	RakButtonMorphic * output = [self allocImageWithBackground:imageNames[0] :defaultState :target : selector];
	
	if(output != nil)
	{
		[output.cell setHighlightAllowed:NO];
		output = [output initImages : imageNames : defaultState];
	}
	
	return output;
}

- (instancetype) initImages : (NSArray*) imageNames : (short) defaultState
{
	NSMutableArray * images = [NSMutableArray array];
	
	NSImage * image;
	BOOL firstPass = YES;
	uint currentTheme = [Prefs getCurrentTheme : self];
	
	for(NSString * imageName in imageNames)
	{
		if(firstPass)
		{
			[images addObject : self.image];
			firstPass = !firstPass;
		}
		else
		{
			image = [RakResPath craftResNameFromContext:imageName : NO : YES : currentTheme];
			if(image != nil)
				[images addObject:image];
			else
				return nil;
		}
	}
	
	_activeCell = 0;
	
	_defaultState = defaultState;
	_images = [NSArray arrayWithArray:images];
	
	return self;
}

#pragma mark - Active Cell property

- (uint) activeCell
{
	return _activeCell;
}

- (void) setActiveCell : (uint) activeCell
{
	if(activeCell <= [_images count])
	{
		self.image = _images[activeCell];
		_activeCell = activeCell;
	}
}

@end
