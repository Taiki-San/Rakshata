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
		[output.cell setActiveAllowed:NO];
		output = [output initImages : imageNames : defaultState];
	}
	
	return output;
}

- (instancetype) initImages : (NSArray*) imageNames : (short) defaultState
{
	uint currentTheme = [Prefs getCurrentTheme : self];
	
	if(![self updateImages:imageNames :currentTheme])
		return nil;
	
	_activeCell = 0;
	_defaultState = defaultState;
	_imageNames = [NSArray arrayWithArray:imageNames];
	
	return self;
}

#pragma mark - Context update

- (BOOL) updateImages : (NSArray *) imageNames : (uint) currentTheme
{
	NSMutableArray * images = [NSMutableArray array];
	
	NSImage * image;
	BOOL firstPass = YES;
	
	for(NSString * imageName in imageNames)
	{
		if(firstPass)
		{
			[images addObject : self.image];
			firstPass = !firstPass;
		}
		else
		{
			image = [RakResPath getImageFromTheme:imageName :currentTheme];
			if(image != nil)
				[images addObject:image];
			else
				return NO;
		}
	}
	
	_images = [NSArray arrayWithArray:images];
	
	return YES;
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return;
	
	uint currentTheme = [Prefs getCurrentTheme : nil];
	
	NSImage * image = [RakResPath getImageFromTheme:_imageNames[0] :currentTheme];
	
	if(image != nil)
	{
		self.image = image;
		[self updateImages:_imageNames :currentTheme];
	}
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
