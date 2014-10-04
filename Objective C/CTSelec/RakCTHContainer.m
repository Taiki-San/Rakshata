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

- (void) setFrame : (NSRect) frameRect
{
	frameRect = [self frameFromParent : frameRect];

	[super setFrame : frameRect];
	self.gradientMaxWidth = frameRect.size.height;
	
	[projectName setFrameOrigin:[self projectNamePos : self.bounds.size.height]];
	[authorName setFrameOrigin:[self authorNamePos : self.bounds.size.height]];

	[_tableController setFrame : self.bounds];
}

- (void) resizeAnimation : (NSRect) frameRect
{
	frameRect = [self frameFromParent : frameRect];
	
	[self.animator setFrame : frameRect];
	self.gradientMaxWidth = frameRect.size.height;

	[projectName.animator setFrameOrigin:[self projectNamePos : frameRect.size.height]];
	[authorName.animator setFrameOrigin:[self authorNamePos : frameRect.size.height]];
	
	frameRect.origin = NSZeroPoint;
	
	[_tableController resizeAnimation : frameRect];
}

#pragma mark - Interface

- (void) loadProject : (PROJECT_DATA) project
{
	_data = project;
	
	NSString * currentElem = [[NSString alloc] initWithData:[NSData dataWithBytes:_data.projectName length:sizeof(_data.projectName)] encoding:NSUTF32LittleEndianStringEncoding];
	
	//Project name
	if(projectName == nil)
	{
		projectName = [[RakText alloc] initWithText : self.frame : currentElem : [self textColor]];
		if(projectName)
		{
			[projectName setFont : [NSFont fontWithName:[Prefs getFontName:GET_FONT_TITLE] size: 18]];
			[projectName setFrameOrigin : [self projectNamePos : self.bounds.size.height]];
			[projectName sizeToFit];
		
			[self addSubview: projectName];
		}
	}
	else
	{
		[projectName setStringValue : currentElem];
		[projectName sizeToFit];
	}
	
	currentElem = [[NSString alloc] initWithData:[NSData dataWithBytes:_data.authorName length:sizeof(_data.authorName)] encoding:NSUTF32LittleEndianStringEncoding];

	//Author name
	if(authorName == nil)
	{
		authorName = [[RakText alloc] initWithText : self.frame : currentElem : [self textColor]];
		if(authorName)
		{
			[authorName setFont : [[NSFontManager sharedFontManager] fontWithFamily:[Prefs getFontName:GET_FONT_TITLE]
																	traits:NSItalicFontMask weight:0 size: 13]];

			[authorName setFrameOrigin : [self authorNamePos : self.bounds.size.height]];
			[authorName sizeToFit];
			
			[self addSubview: authorName];
		}
	}
	else
	{
		[authorName setStringValue : currentElem];
		[projectName sizeToFit];
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
		[_tableController setFrame:self.bounds];	//We refresh scrollview size
	}
}

#pragma mark - Elements positions

- (NSPoint) projectNamePos : (CGFloat) height
{
	return NSMakePoint(26, height * 11 / 20);
}

- (NSPoint) authorNamePos : (CGFloat) height
{
	return NSMakePoint(35, height * 5 / 20);
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
