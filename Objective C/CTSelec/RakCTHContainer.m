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

		[self loadProject : project];
	}
	
	return self;
}

- (void) setFrame : (NSRect) frameRect
{
	frameRect = [self frameFromParent : frameRect];

	[super setFrame : frameRect];
	self.gradientMaxWidth = frameRect.size.height;
}

#pragma mark - Interface

- (void) loadProject : (PROJECT_DATA) project
{
	_data = project;
	
	NSString * currentElem = [[NSString alloc] initWithData:[NSData dataWithBytes:_data.projectName length:sizeof(_data.projectName)] encoding:NSUTF32LittleEndianStringEncoding];
	
	//Project name
	if(projectName == nil)
	{
		projectName = [[RakText alloc] initWithText : self.frame : currentElem : [NSColor whiteColor]];
		if(projectName)
		{
			[projectName setFont : [NSFont fontWithName:[Prefs getFontName:GET_FONT_TITLE] size: 18]];
			[projectName setFrameOrigin : [self projectNamePos]];
			[projectName sizeToFit];
		
			[self addSubview: projectName];
		}
	}
	else
		[projectName setStringValue : currentElem];
	
	currentElem = [[NSString alloc] initWithData:[NSData dataWithBytes:_data.authorName length:sizeof(_data.authorName)] encoding:NSUTF32LittleEndianStringEncoding];

	//Author name
	if(authorName == nil)
	{
		authorName = [[RakText alloc] initWithText : self.frame : currentElem : [NSColor whiteColor]];
		if(authorName)
		{
			[authorName setFont : [[NSFontManager sharedFontManager] fontWithFamily:[Prefs getFontName:GET_FONT_TITLE]
																	traits:NSItalicFontMask weight:0 size: 13]];

			[authorName setFrameOrigin : [self authorNamePos]];
			[authorName sizeToFit];
			
			[self addSubview: authorName];
		}
	}
	else
		[authorName setStringValue : currentElem];
}

#pragma mark - Elements positions

- (NSPoint) projectNamePos
{
	return NSMakePoint(26, self.bounds.size.height * 11 / 20);
}

- (NSPoint) authorNamePos
{
	return NSMakePoint(35, self.bounds.size.height * 5 / 20);
}

#pragma mark - UI utilities

- (NSRect) frameFromParent : (NSRect) parentFrame
{
	parentFrame.size.height *= 0.4f;
	
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

@end
