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
		project.chapitresFull = project.chapitresInstalled = NULL;
		project.tomesFull = project.tomesInstalled = NULL;
		project.chapitresPrix = NULL;
		
		_project = project;
		_workingArea.origin = NSZeroPoint;
		_workingArea.size = [self defaultWorkingSize];
		
		[self initContent];
	}
	
	return self;
}

- (void) initContent
{
	NSImage * image = [self loadImage];
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

- (NSImage *) loadImage
{
	char * teamPath = getPathForRepo(_project.repo);
	
	if(teamPath == NULL)
		return nil;
	
	NSImage * image = nil;
	
	NSBundle * bundle = [NSBundle bundleWithPath: [NSString stringWithFormat:@"imageCache/%s/", teamPath]];
	if(bundle != nil)
		image = [bundle imageForResource:[NSString stringWithFormat:@"%d_"PROJ_IMG_SUFFIX_SRGRID, _project.projectID]];
	
	if(image == nil)
		image = [NSImage imageNamed:@"defaultSRImage"];
	
	return image;
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
	return [Prefs getSystemColor:GET_COLOR_CLICKABLE_TEXT :nil];
}

- (NSColor *) getTagTextColor
{
	return [Prefs getSystemColor:GET_COLOR_TAGITEM_FONT :nil];
}

- (NSColor *) backgroundColor
{
	return [NSColor grayColor];
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
