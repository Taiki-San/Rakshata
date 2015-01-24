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

#define BORDER_BETWEEN_NAME_AND_TABLE 10

@implementation RakCTHContainer

- (id) initWithProject : (NSRect) frame : (PROJECT_DATA) project
{
	self = [self initWithFrame : [self frameFromParent : frame]];
	
	if(self != nil)
	{
		[self initGradient];
		self.gradientMaxWidth = frame.size.height;
		self.gradientWidth = 100;
		self.angle = 270;
		self.autoresizesSubviews = NO;

		[self loadProject : project];
	}
	
	return self;
}

- (BOOL) isFlipped
{
	return YES;
}

- (void) setFrame : (NSRect) frameRect
{
	frameRect = [self frameFromParent : frameRect];
	lastKnownHeight = frameRect.size.height;
	
	[super setFrame : frameRect];
	
	self.gradientMaxWidth = frameRect.size.height;
	
	[_tableController setFrame : self.bounds];
	CGFloat tableControllerBaseX = _tableController.baseX, oldNameHeight = projectName.bounds.size.height, oldAuthorHeight = authorName.bounds.size.height;
	
	[projectName setFrameOrigin:[self projectNamePos : self.bounds.size]];
	projectName.fixedWidth = tableControllerBaseX - projectName.frame.origin.x - BORDER_BETWEEN_NAME_AND_TABLE;
	
	if(projectName.bounds.size.height != oldNameHeight)	//fixedWidth may warp the lines
		[projectName setFrameOrigin:[self projectNamePos : self.bounds.size]];
	
	[authorName setFrameOrigin:[self authorNamePos : self.bounds.size]];
	authorName.fixedWidth = tableControllerBaseX - authorName.frame.origin.x - BORDER_BETWEEN_NAME_AND_TABLE;
	
	if(authorName.bounds.size.height != oldAuthorHeight)	//fixedWidth may warp the lines
		[authorName setFrameOrigin:[self authorNamePos : self.bounds.size]];
}

- (void) resizeAnimation : (NSRect) frameRect
{
	frameRect = [self frameFromParent : frameRect];
	lastKnownHeight = frameRect.size.height;
	
	[self.animator setFrame : frameRect];
	
	self.gradientMaxWidth = frameRect.size.height;
	
	NSPoint nameOrigin = [self projectNamePos : frameRect.size], authorOrigin = [self authorNamePos : frameRect.size];
	CGFloat oldNameHeight = projectName.bounds.size.height, oldAuthorHeight = authorName.bounds.size.height;

	[projectName.animator setFrameOrigin : nameOrigin];
	[authorName.animator setFrameOrigin : authorOrigin];
	
	frameRect.origin = NSZeroPoint;
	
	CGFloat tableControllerBaseX = [_tableController rawBaseX:frameRect];
	projectName.fixedWidth = tableControllerBaseX - nameOrigin.x - BORDER_BETWEEN_NAME_AND_TABLE;
	authorName.fixedWidth = tableControllerBaseX - authorOrigin.x - BORDER_BETWEEN_NAME_AND_TABLE;
	
	if(projectName.bounds.size.height != oldNameHeight)	//fixedWidth may warp the lines
		[projectName.animator setFrameOrigin:[self projectNamePos : frameRect.size]];
	
	if(authorName.bounds.size.height != oldAuthorHeight)	//fixedWidth may warp the lines
		[authorName.animator setFrameOrigin:[self authorNamePos : frameRect.size]];
	
	[_tableController resizeAnimation : frameRect];
}

#pragma mark - Interface

- (void) loadProject : (PROJECT_DATA) project
{
	_data = project;
	
	BOOL needProcessName = NO, needProcessAuthor = NO;
	NSString * currentElem = getStringForWchar(_data.projectName);
	
	//Project name
	if(projectName == nil)
	{
		projectName = [[RakText alloc] initWithText : self.frame : currentElem : [self textColor]];
		if(projectName != nil)
		{
			[projectName setAlignment:NSLeftTextAlignment];
			[projectName.cell setWraps:YES];
			needProcessName = YES;
			
			[projectName setFont : [NSFont fontWithName:[Prefs getFontName:GET_FONT_TITLE] size: 18]];
			projectName.fixedWidth = projectName.fixedWidth;	//Will refresh our width
			[projectName setFrameOrigin : [self projectNamePos : self.bounds.size]];
		
			[self addSubview: projectName];
		}
	}
	else if(![currentElem isEqualToString:projectName.stringValue])
	{
		[projectName setStringValue : currentElem];
		projectName.fixedWidth = projectName.fixedWidth;	//Will refresh our width
		[projectName setFrameOrigin : [self projectNamePos : self.bounds.size]];
	}
	
	currentElem = getStringForWchar(_data.authorName);

	//Author name
	if(authorName == nil)
	{
		authorName = [[RakText alloc] initWithText : self.frame : currentElem : [self textColor]];
		if(authorName)
		{
			[authorName setAlignment:NSLeftTextAlignment];
			[authorName.cell setWraps:YES];
			needProcessAuthor = YES;

			[authorName setFont : [[NSFontManager sharedFontManager] fontWithFamily:[Prefs getFontName:GET_FONT_TITLE]
																	traits:NSItalicFontMask weight:0 size: 13]];
			authorName.fixedWidth = authorName.fixedWidth;	//Will refresh our width
			[authorName setFrameOrigin : [self authorNamePos : self.bounds.size]];
			
			[self addSubview: authorName];
		}
	}
	else if(![currentElem isEqualToString:authorName.stringValue])
	{
		[authorName setStringValue : currentElem];
		authorName.fixedWidth = authorName.fixedWidth;	//Will refresh our width
		[authorName setFrameOrigin : [self authorNamePos : self.bounds.size]];
	}
	
	if(_tableController == nil)
	{
		_tableController = [[RakCTHTableController alloc] initWithProject : _data frame : self.bounds];
		
		if(_tableController != nil)
		{
			if(_tableController.scrollView != nil)
				[self addSubview:_tableController.scrollView];
			else
				_tableController = nil;
		}
	}
	else
	{
		[_tableController updateProject:_data];

		NSRect bounds = self.bounds;
		if(bounds.size.height != lastKnownHeight)	//May happend during animation
			bounds.size = [self frameFromParent:bounds].size;

		[_tableController setFrame : bounds];	//We refresh scrollview size
	}
	
	if(_tableController != nil && (needProcessAuthor || needProcessName))
	{
		CGFloat tableControllerBaseX = _tableController.baseX, oldHeight;
		
		if(needProcessName)
		{
			oldHeight = projectName.bounds.size.height;
			projectName.fixedWidth = tableControllerBaseX - projectName.frame.origin.x - BORDER_BETWEEN_NAME_AND_TABLE;
			
			if(projectName.bounds.size.height != oldHeight)
				[projectName setFrameOrigin : [self projectNamePos : self.bounds.size]];

		}
		
		if(needProcessAuthor)
		{
			oldHeight = authorName.bounds.size.height;
			authorName.fixedWidth = tableControllerBaseX - authorName.frame.origin.x - BORDER_BETWEEN_NAME_AND_TABLE;
			
			if(authorName.bounds.size.height != oldHeight)
				[authorName setFrameOrigin : [self authorNamePos : self.bounds.size]];
			
		}
	}
}

#pragma mark - Elements positions

- (NSPoint) projectNamePos : (NSSize) size
{
	CGFloat height = size.height * 6.5 / 20;
	
	if(projectName != nil)
		height -= projectName.bounds.size.height / 2;
	
	return NSMakePoint(size.width * 7 / 100, height);
}

- (NSPoint) authorNamePos : (NSSize) size
{
	CGFloat height = size.height * 13 / 20;
	
	if(authorName != nil)
		height -= authorName.bounds.size.height / 2;
	
	return NSMakePoint(size.width * 9 / 100, height);
}

#pragma mark - UI utilities

- (NSRect) frameFromParent : (NSRect) parentFrame
{
	parentFrame.size.height *= 0.45f;
	
	return parentFrame;
}

- (NSRect) grandientBounds
{
	CGFloat height  = MIN(self.gradientMaxWidth, self.bounds.size.height * self.gradientWidth);
	return NSMakeRect(0, 0, self.bounds.size.width, height);
}

- (NSColor *) startColor
{
	return [Prefs getSystemColor : GET_COLOR_CTHEADER_GRADIENT_START : NO];
}

- (NSColor *) endColor : (const NSColor *) startColor
{
	return [Prefs getSystemColor : GET_COLOR_CTHEADER_GRADIENT_END : NO];
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return;
	
	if(projectName != nil)
		[projectName setTextColor:[self textColor]];
	
	if(authorName != nil)
		[authorName setTextColor:[self textColor]];
}

- (NSColor *) textColor
{
	return [Prefs getSystemColor:GET_COLOR_CTHEADER_FONT : nil];
}

@end
