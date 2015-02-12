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

@implementation RakCTHeader

- (instancetype) initWithData : (NSRect) frameRect : (PROJECT_DATA) project
{
	RakCTHeaderImage * localHeader = [[RakCTHeaderImage alloc] initWithData : frameRect : project];

	if(localHeader != nil)
	{
		self = [self initWithFrame:[self frameByParent:frameRect : localHeader]];
		
		if(self != nil)
		{
			header = localHeader;
			_backgroundColor = [Prefs getSystemColor : GET_COLOR_BACKGROUD_COREVIEW : self];
			_synopsisTitleBackground = [Prefs getSystemColor:GET_COLOR_BACKGROUND_TABS : nil];

			[header setFrame : self.bounds];
			[self addSubview:header];
			
			synopsis = [[RakCTProjectSynopsis alloc] initWithProject:project : self.bounds : header.bounds.size];
			if(synopsis != nil)
			{
				_synopsisTitleHeight = [synopsis titleHeight];	//For now, this height is stable
				[self addSubview:synopsis];
			}
		}
	}
	else
		self = nil;
	
	return self;
}

- (void) updateProject : (PROJECT_DATA) project
{
	[header updateHeaderProject:project];
	
	if(synopsis != nil)
		[synopsis updateProject : project];
	else
	{
		synopsis = [[RakCTProjectSynopsis alloc] initWithProject : project : self.bounds : header.bounds.size];
		if(synopsis != nil)
			[self addSubview:synopsis];
	}
}

- (void) updateProjectDiff : (PROJECT_DATA) oldData : (PROJECT_DATA) newData
{
	uint length;
	if((length = wstrlen(oldData.description)) != wstrlen(newData.description) || memcmp(oldData.description, newData.description, length * sizeof(charType)))
	{
		[synopsis updateProject : newData];
	}
	
	[header updateHeaderProjectInternal :newData];
}

#pragma mark - Resizing

- (NSRect) frameByParent : (NSRect) parentFrame : (RakCTHeaderImage *) _header
{
	NSRect output;
	
	if(_header == nil)
		_header = header;
	
	output.size.width = parentFrame.size.width;
	output.size.height = [_header sizeByParent : parentFrame].height;
	
	output.origin.x = 0;
	output.origin.y = parentFrame.size.height - output.size.height;
	
	return output;
}

- (void) setFrame:(NSRect) frameRect
{
	if(self.isHidden && !forceUpdate)
	{
		_cachedFrame = frameRect;
		return;
	}
		
	[super setFrame:[self frameByParent:frameRect : header]];
	
	[header setFrame : self.bounds];
	[synopsis setFrame : self.bounds : header.bounds.size];
}

- (void) resizeAnimation : (NSRect) frame
{
	if(self.isHidden && !forceUpdate)
	{
		_cachedFrame = frame;
		return;
	}
	
	frame = [self frameByParent : frame : header];

	[self.animator setFrame:frame];

	NSSize headerSize = [header frameByParent : frame].size;
	
	[header resizeAnimation:frame];
	[synopsis resizeAnimation : frame : headerSize];
}

- (void) setHidden : (BOOL) flag
{
	if(!flag && self.isHidden)
	{
		forceUpdate = YES;
		[self setFrame:_cachedFrame];
		forceUpdate = NO;
	}

	[super setHidden:flag];
}

#pragma mark - Drawing

- (void) drawRect:(NSRect)dirtyRect
{
	if(_synopsisTitleHeight)
	{
		[_synopsisTitleBackground setFill];
		NSRectFill(NSMakeRect(0, dirtyRect.size.height - _synopsisTitleHeight, dirtyRect.size.width, _synopsisTitleHeight));
		dirtyRect.size.height -= _synopsisTitleHeight;
	}

	[_backgroundColor setFill];
	
	CGFloat maxX = NSMaxX(synopsis.frame);
	if(dirtyRect.size.width > maxX)
		dirtyRect.size.width = maxX;
	
	NSRectFill(dirtyRect);
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return;
	
	_backgroundColor = [Prefs getSystemColor : GET_COLOR_BACKGROUD_COREVIEW : nil];
	_synopsisTitleBackground = [Prefs getSystemColor:GET_COLOR_BACKGROUND_TABS : nil];
}

@end
