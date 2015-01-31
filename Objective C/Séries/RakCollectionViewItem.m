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

static NSSize workingSize = {RCVC_MINIMUM_WIDTH, RCVC_MINIMUM_HEIGHT};

enum
{
	INTERLINE_NAME_AUTHOR = 0
};

@implementation RakCollectionViewItem

- (instancetype) initWithProject : (PROJECT_DATA) project
{
	self = [self initWithFrame:NSMakeRect(0, 0, RCVC_MINIMUM_WIDTH, RCVC_MINIMUM_HEIGHT)];
	
	if(self != nil)
	{
		_selected = NO;
		
		//We really don't care about those data, so we don't want the burden of having to update them
		project.chapitresFull = project.chapitresInstalled = NULL;
		project.tomesFull = project.tomesInstalled = NULL;
		project.chapitresPrix = NULL;
		
		_project = project;
		[self initContent];
	}
	
	return self;
}

- (void) initContent
{
	name = [[RakText alloc] initWithText :getStringForWchar(_project.projectName) : [self getTextColor]];
	if(name != nil)
	{
		name.alignment = NSCenterTextAlignment;
		name.font = [NSFont fontWithName:[Prefs getFontName:GET_FONT_SR_TITLE] size:13];

		[name.cell setWraps : YES];
		name.fixedWidth = RCVC_MINIMUM_WIDTH * 0.8;
		
		[self addSubview:name];
	}
	
	author = [[RakText alloc] initWithText:getStringForWchar(_project.authorName) : [self getTextColor]];
	if(author != nil)
	{
		author.alignment = NSCenterTextAlignment;
		author.font = [NSFont fontWithName:[Prefs getFontName:GET_FONT_STANDARD] size:10];
		
		[author.cell setWraps : YES];
		author.fixedWidth = RCVC_MINIMUM_WIDTH * 0.8;
		
		[self addSubview:author];
	}
}

- (void) mouseDown:(NSEvent *)theEvent
{
	_selected = !_selected;
	[self setNeedsDisplay : YES];
}

#pragma mark - Resizing code

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

- (void) resizeContent : (NSSize) newSize : (BOOL) animated
{
	NSRect frameRect;
	//We update the frame
	frameRect.origin = NSCenterSize(newSize, workingSize);
	frameRect.size = workingSize;
	
	//We resize our content
	NSPoint previousOrigin;
	
	if(animated)
	{
		previousOrigin = [self originOfName : frameRect];
		[name.animator setFrameOrigin: previousOrigin];
		[author.animator setFrameOrigin:[self originOfAuthor : frameRect : previousOrigin]];
	}
	else
	{
		previousOrigin = [self originOfName : frameRect];
		[name setFrameOrigin: previousOrigin];
		[author setFrameOrigin:[self originOfAuthor : frameRect : previousOrigin]];
	}
}

- (NSPoint) originOfName : (NSRect) frameRect
{
	return NSCenteredRect(frameRect, name.bounds);
}

- (NSPoint) originOfAuthor : (NSRect) frameRect : (NSPoint) nameOrigin
{
	NSPoint center = NSCenteredRect(frameRect, name.bounds);
	
	center.y = nameOrigin.y - (author.bounds.size.height + INTERLINE_NAME_AUTHOR);
	
	return center;
}

#pragma mark - Color & Drawing

- (NSColor *) getTextColor
{
	return [Prefs getSystemColor:GET_COLOR_CLICKABLE_TEXT :nil];
}

- (NSColor *) borderColor
{
	return [NSColor blackColor];
}

- (NSColor *) backgroundColor
{
	if(_selected)
		return [NSColor grayColor];

	return [Prefs getSystemColor:GET_COLOR_BACKGROUD_COREVIEW :nil];
}

- (void) drawRect:(NSRect)dirtyRect
{
	NSRect printArea = {NSCenterSize(dirtyRect.size, workingSize), workingSize};
	NSBezierPath * path = [NSBezierPath bezierPathWithRoundedRect:printArea xRadius:3 yRadius:3];

	[[self backgroundColor] setFill];
	[path fill];
}

@end
