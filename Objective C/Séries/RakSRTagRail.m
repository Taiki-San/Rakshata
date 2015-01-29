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

@implementation RakSRTagRail

- (instancetype) initWithFrame : (NSRect) frameRect : (CGFloat) baseSearchBar
{
	self = [super initWithFrame:frameRect];
	
	if(self != nil)
	{
		_noReorder = NO;
		
		if(baseSearchBar == 0)
		{
			_baseSearchBar = 0;
			_reducedWidth = frameRect.size.width;
		}
		else
		{
			_baseSearchBar = baseSearchBar - 5;
			_reducedWidth = _baseSearchBar - frameRect.origin.x;
		}

		tagList = [NSMutableArray array];
		[self insertTags:@[@"First Tag", @"Second Tag", @"Third Tag", @"Fourth Tag", @"Fifth Tag", @"Sixth Tag"] : self.bounds];
	}
	
	return self;
}

#pragma mark - Tag management

- (void) insertTags : (NSArray *) tags : (NSRect) frame
{
	uint currentX = 0, _width = frame.size.width, width = _baseSearchBar != 0 ? _reducedWidth : _width, currentRow = 0, index = 0;
	
	RakSRTagItem * tag;
	for(NSString * tagString in tags)
	{
		tag = [[RakSRTagItem alloc] initWithTag : tagString];
		if(tag == nil)
			continue;
		else
		{
			[tagList addObject:tag];
			tag.parent = self;
		}
		
		if(currentX + tag.bounds.size.width > width)
		{
			if(!currentRow)
				width = _width;
			
			currentX = 0;
			currentRow++;
		}
		
		[tag setFrameOrigin:NSMakePoint(currentX, currentRow * (TAG_BUTTON_HEIGHT + TAG_RAIL_INTER_RAIL_BORDER))];
		[self addSubview:tag];
		
		tag.index = index++;
		currentX += tag.bounds.size.width + TAG_RAIL_INTER_ITEM_BORDER;
	}
	
	_nbRow = currentRow + 1;
}

- (void) reorderTags: (NSRect) frame : (BOOL) animated
{
	uint currentX = 0, currentRow = 0, _width = frame.size.width, width = _baseSearchBar != 0 ? _reducedWidth : _width, index = 0;
	NSPoint point;
	
	for(RakSRTagItem * tag in tagList)
	{
		if(currentX + tag.bounds.size.width > width)
		{
			if(!currentRow)
				width = _width;
				
			currentX = 0;
			currentRow++;
		}
		
		point = NSMakePoint(currentX, currentRow * (TAG_BUTTON_HEIGHT + TAG_RAIL_INTER_RAIL_BORDER));
		
		if(animated)
			[tag.animator setFrameOrigin:point];
		else
			[tag setFrameOrigin:point];
		
		tag.index = index++;
		currentX += tag.bounds.size.width + TAG_RAIL_INTER_ITEM_BORDER;
	}
	
	if(_nbRow != currentRow + 1)
	{
		_nbRow = currentRow + 1;

		Series * tabSerie = [(RakAppDelegate *) [NSApp delegate] serie];
		if(tabSerie != nil)
		{
			_noReorder = YES;
			[tabSerie resetFrameSize : YES];
			_noReorder = NO;
		}
	}
}

- (void) removeTag : (uint) index
{
	if(index >= [tagList count])
		return;
	
	__block RakSRTagItem * tag = [tagList objectAtIndex:index];
	
	[tagList removeObjectAtIndex:index];

	[NSAnimationContext beginGrouping];

	[[NSAnimationContext currentContext] setCompletionHandler:^{
		[tag removeFromSuperview];
		[tag setAlphaValue:1];
	}];
	
	[tag.animator setAlphaValue:0];
	
	[self reorderTags:self.bounds : YES];
	
	[NSAnimationContext endGrouping];
}

#pragma mark - Properties

- (void) setBaseSearchBar : (CGFloat) baseSearchBar
{
	if(baseSearchBar == 0)
	{
		_baseSearchBar = 0;
		_reducedWidth = self.frame.size.width;
	}
	else
	{
		_baseSearchBar = baseSearchBar - 5;
		_reducedWidth = _baseSearchBar - self.frame.origin.x;
	}
}

#pragma mark - View management

- (void) _resize:(NSRect)frameRect : (BOOL) animated
{
	BOOL haveToReorder = !_noReorder && frameRect.size.width != self.bounds.size.width && _nbRow > 1;
	
	_reducedWidth = _baseSearchBar != 0 ? _baseSearchBar - frameRect.origin.x : frameRect.size.width;
	
	if(animated)
		[self.animator setFrame:frameRect];
	else
		[super setFrame:frameRect];
	
	if(haveToReorder)
		[self reorderTags:frameRect :animated];
}

- (void) setFrame:(NSRect)frameRect
{
	[self _resize:frameRect :NO];
}

- (void) resizeAnimation : (NSRect) frameRect
{
	[self _resize:frameRect :YES];
}

- (BOOL) isFlipped
{
	return YES;
}

@end
