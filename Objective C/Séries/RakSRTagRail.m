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

		_currentX = _currentRow = 0;
		tagList = [NSMutableArray array];
		tagNames = [NSMutableArray array];
		[self insertTags:@[@"First Tag", @"Second Tag", @"Third Tag", @"Fourth Tag", @"Fifth Tag"] : self.bounds];
		
		_nbRow = _currentRow + 1;
		
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(receiveNotifTag:) name:SR_NOTIFICATION_TAG object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(receiveNotifAuthor:) name:SR_NOTIFICATION_AUTHOR object:nil];
	}
	
	return self;
}

- (void) dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void) receiveNotifTag : (NSNotification *) notification
{
	NSString * string;
	NSNumber * opType;
	
	if(notification == nil || notification.userInfo == nil || (string = [notification.userInfo objectForKey:SR_NOTIF_CACHEID]) == nil || ![string isKindOfClass:[NSString class]])
		return;
	
	BOOL insertion = (opType = [notification.userInfo objectForKey:SR_NOTIF_OPTYPE]) == nil || ![opType isKindOfClass:[NSNumber class]] || [opType boolValue];
	
	[self performNotification:string :YES : insertion];
}

- (void) receiveNotifAuthor : (NSNotification *) notification
{
	NSString * string;
	NSNumber * opType;
	
	if(notification == nil || notification.userInfo == nil || (string = [notification.userInfo objectForKey:SR_NOTIF_CACHEID]) == nil || ![string isKindOfClass:[NSString class]])
		return;

	BOOL insertion = (opType = [notification.userInfo objectForKey:SR_NOTIF_OPTYPE]) == nil || ![opType isKindOfClass:[NSNumber class]] || [opType boolValue];

	[self performNotification: string : NO : insertion];
}

- (void) performNotification : (NSString *) object : (BOOL) wantTag : (BOOL) insertion
{
	if(insertion)
		[self addTag:object];
	else
	{
		[self removeTag:[tagNames indexOfObject:object]];
	}
}

#pragma mark - Tag management

- (void) addTag : (NSString *) tagName
{
	uint tagLength = [tagList count], length;
	NSRect frame;
	BOOL newRow = NO;
	
	//Create animation and update the model
	[NSAnimationContext beginGrouping];
	
	[self insertTags:@[tagName] :self.bounds];
	[self updateContext];

	//Update the UI
	length = [tagList count];
	for(RakSRTagItem * item; tagLength < length; tagLength++)
	{
		item = [tagList objectAtIndex:tagLength];
		frame = item.frame;
	
		if(newRow || frame.origin.x == 0 || frame.origin.y == 0)
		{
			item.alphaValue = 0;
			item.animator.alphaValue = 1;

			if(frame.origin.x != 0)			newRow = YES;
		}
		else
		{
			item.frame = NSMakeRect(self.bounds.size.width, frame.origin.y, frame.size.width, frame.size.height);
			item.animator.frame = frame;
		}
	}
	
	[NSAnimationContext endGrouping];
}

- (void) updateContext
{
	if(_nbRow != _currentRow + 1)
	{
		_nbRow = _currentRow + 1;
		
		Series * tabSerie = [(RakAppDelegate *) [NSApp delegate] serie];
		if(tabSerie != nil)
		{
			_noReorder = YES;
			[tabSerie resetFrameSize : YES];
			_noReorder = NO;
		}
	}
}

- (void) insertTags : (NSArray *) tags : (NSRect) frame
{
	uint _width = frame.size.width, width = _baseSearchBar != 0 && _currentRow == 0 ? _reducedWidth : _width, index = [tagList count];
	
	RakSRTagItem * tag;
	for(NSString * tagString in tags)
	{
		tag = [[RakSRTagItem alloc] initWithTag : tagString];
		if(tag == nil)
			continue;
		else
		{
			[tagList addObject:tag];
			[tagNames addObject:tagString];
			tag.parent = self;
		}
		
		if(_currentX + tag.bounds.size.width > width)
		{
			if(!_currentRow)
				width = _width;
			
			_currentX = 0;
			_currentRow++;
		}
		
		[tag setFrameOrigin:NSMakePoint(_currentX, _currentRow * (TAG_BUTTON_HEIGHT + TAG_RAIL_INTER_RAIL_BORDER))];
		[self addSubview:tag];
		
		tag.index = index++;
		_currentX += tag.bounds.size.width + TAG_RAIL_INTER_ITEM_BORDER;
	}
}

- (void) reorderTags: (NSRect) frame : (BOOL) animated
{
	uint _width = frame.size.width, width = _baseSearchBar != 0 ? _reducedWidth : _width, index = 0;
	NSPoint point;
	
	_currentX = _currentRow = 0;
	
	for(RakSRTagItem * tag in tagList)
	{
		if(_currentX + tag.bounds.size.width > width)
		{
			if(!_currentRow)
				width = _width;
				
			_currentX = 0;
			_currentRow++;
		}
		
		point = NSMakePoint(_currentX, _currentRow * (TAG_BUTTON_HEIGHT + TAG_RAIL_INTER_RAIL_BORDER));
		
		if(animated)
			[tag.animator setFrameOrigin:point];
		else
			[tag setFrameOrigin:point];
		
		tag.index = index++;
		_currentX += tag.bounds.size.width + TAG_RAIL_INTER_ITEM_BORDER;
	}
	
	if(_nbRow != _currentRow + 1)
	{
		_nbRow = _currentRow + 1;

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
	[tagNames removeObjectAtIndex:index];

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
