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

enum
{
	BORDER_THUMB = 150
};

@implementation RakSRProjectView

- (instancetype) initWithProject : (PROJECT_DATA) project
{
	if(!project.isInitialized || project.repo == NULL)
		return nil;
	
	self = [self initWithFrame:NSMakeRect(0, 0, RCVC_MINIMUM_WIDTH, RCVC_MINIMUM_HEIGHT)];
	
	if(self != nil)
	{
		//We really don't care about those data, so we don't want the burden of having to update them
		nullifyCTPointers(&project);

		_project = project;
		_workingArea.origin = NSZeroPoint;
		_workingArea.size = [self defaultWorkingSize];
		
		[self initContent];
		
		registerThumbnailUpdate(self, @selector(thumbnailUpdate:), THUMBID_HEAD);
	}
	
	return self;
}

- (void) initContent
{
	[Prefs getCurrentTheme:self];
	
	NSImage * image = loadImageGrid(_project);
	if(image != nil)
	{
		thumbnail = [[NSImageView alloc] initWithFrame: (NSRect) {{0,0}, [self thumbSize]}];
		if(thumbnail != nil)
		{
			thumbnail.image = image;
			[self addSubview:thumbnail];
		}
	}
}

- (void) dealloc
{
	[Prefs deRegisterForChanges:self];
	[[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void) updateProject : (PROJECT_DATA) project
{
	//We really don't care about those data, so we don't want the burden of having to update them
	nullifyCTPointers(&project);
	
	_project = project;
	
	NSImage * image = loadImageGrid(_project);
	if(image != nil)
		thumbnail.image = image;
}

- (void) thumbnailUpdate : (NSNotification *) notification
{
	NSDictionary * dict = notification.userInfo;
	if(dict == nil || !_project.isInitialized)
		return;
	
	NSNumber * project = [dict objectForKey:@"project"], * repo = [dict objectForKey:@"source"];
	
	if(project == nil || repo == nil)
		return;
	
	if([project unsignedIntValue] == _project.projectID && [repo unsignedLongLongValue] == getRepoID(_project.repo))
	{
		[self updateProject:_project];
		[self setNeedsDisplay:YES];
	}
}

- (RakText *) getTextElement : (NSString *) string : (NSColor *) color : (byte) fontCode : (CGFloat) fontSize
{
	RakText * output = [[RakText alloc] initWithText :string : color];
	if(output != nil)
	{
		output.alignment = NSCenterTextAlignment;
		output.font = [NSFont fontWithName:[Prefs getFontName:fontCode] size:fontSize];
		
		[output.cell setWraps : YES];
		[output sizeToFit];
	}
	
	return output;
}

#pragma mark - Color

- (NSColor *) getTextColor
{
	return [Prefs getSystemColor:COLOR_CLICKABLE_TEXT :nil];
}

- (NSColor *) getTagTextColor
{
	return [Prefs getSystemColor:COLOR_TAGITEM_FONT :nil];
}

- (NSColor *) backgroundColor
{
	return [Prefs getSystemColor : COLOR_BACKGROUND_GRID_FOCUS : nil];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];

	[self reloadColors];
	[self setNeedsDisplay:YES];
}

- (void) reloadColors
{
	
}

#pragma mark - Property

- (NSRect) workingArea	{	return _workingArea;	}

- (uint) elementID {	return _project.cacheDBID;	}

#pragma mark - Frames

- (NSSize) defaultWorkingSize
{
	return NSZeroSize;
}

- (NSSize) thumbSize
{
	return NSMakeSize(BORDER_THUMB, BORDER_THUMB);
}

- (NSPoint) originOfThumb : (NSRect) frameRect
{
	NSPoint output;
	
	output.x = frameRect.origin.x + frameRect.size.width / 2 - BORDER_THUMB / 2;
	output.y = frameRect.origin.y + frameRect.size.height - BORDER_THUMB;
	
	return output;
}

- (NSPoint) originOfName : (NSRect) frameRect : (NSPoint) thumbOrigin
{
	NSPoint center = NSCenteredRect(frameRect, projectName.bounds);
	
	center.y = thumbOrigin.y - projectName.bounds.size.height;
	
	return center;
}

- (NSPoint) originOfAuthor : (NSRect) frameRect : (NSPoint) nameOrigin
{
	NSPoint center = NSCenteredRect(frameRect, projectAuthor.bounds);
	
	center.y = nameOrigin.y - projectAuthor.bounds.size.height;
	
	return center;
}

//We hook animator in order to run our logic during animated resizing
- (id) animator
{
	_animationRequested = YES;
	return self;
}

- (void) setFrame:(NSRect)frameRect
{
	BOOL animated = _animationRequested;
	
	if(animated)
	{
		_animationRequested = NO;
		[[super animator] setFrame:frameRect];
	}
	else
		[super setFrame:frameRect];
	
	[self resizeContent:frameRect.size :animated];
}

- (void) setFrameSize:(NSSize)newSize
{
	BOOL animated = _animationRequested;
	
	if(animated)
	{
		_animationRequested = NO;
		[[super animator] setFrameSize:newSize];
	}
	else
		[super setFrameSize:newSize];
	
	[self resizeContent:newSize :animated];
}

- (void) setFrameOrigin:(NSPoint)newOrigin
{
	BOOL animated = _animationRequested;
	
	if(animated)
	{
		_animationRequested = NO;
		[[super animator] setFrameOrigin:newOrigin];
	}
	else
		[super setFrameOrigin:newOrigin];
}

- (NSPoint) resizeContent : (NSSize) newSize : (BOOL) animated
{
	_workingArea.origin = NSCenterSize(newSize, _workingArea.size);
	
	NSPoint previousOrigin = [self originOfThumb : _workingArea];
	
	if(animated)
	{
		[thumbnail.animator setFrameOrigin:		previousOrigin];
		[projectName.animator setFrameOrigin: 	(previousOrigin = [self originOfName : _workingArea : previousOrigin])];
	}
	else
	{
		[thumbnail setFrameOrigin: 		previousOrigin];
		[projectName setFrameOrigin: 	(previousOrigin = [self originOfName : _workingArea : previousOrigin])];
	}
	
	return previousOrigin;
}

@end
