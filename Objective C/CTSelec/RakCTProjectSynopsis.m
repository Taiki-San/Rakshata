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

#define TOP_BORDER_WIDTH 5
#define SPACING	5
#define SYNOPSIS_BORDER 20
#define MAIN_TEXT_BORDER 10

@implementation RakCTProjectSynopsis

- (instancetype) initWithProject : (PROJECT_DATA) project : (NSRect) frame : (NSSize) headerSize
{
	self = [self initWithFrame : [self frameFromParent : frame : headerSize]];
	
	if(self != nil)
	{
		self.wantsLayer = YES;
		self.layer.backgroundColor = [Prefs getSystemColor : GET_COLOR_BACKGROUD_CT_READERMODE :self].CGColor;
		self.layer.cornerRadius = 4;
		
		_title = [[RakMenuText alloc] initWithText : self.bounds : @"Résumé"];
		if(_title != nil)
		{
			[_title sizeToFit];
			_title.ignoreInternalFrameMagic = YES;
			_title.drawGradient = YES;
			_title.barWidth = 1;
			_title.widthGradient = 0.4f;

			NSRect titleFrame = [self frameForTitle:self.bounds : _title.bounds.size.height];
			
			[_title setFrame : titleFrame];


			[_title setAlignment : NSRightTextAlignment];
			[self addSubview:_title];
		}
		
		if([self setStringToSynopsis : project])
		{
			[self updateFrame : frame : headerSize : NO];
		}
	}
	
	return self;
}

- (void) updateProject : (PROJECT_DATA) newProject
{
	[self setStringToSynopsis : newProject];
}

- (BOOL) setStringToSynopsis : (PROJECT_DATA) project
{
	BOOL needPostProcessing = NO;
	
	//Create _synopsis object
	if(_synopsis == nil)
	{
		needPostProcessing = YES;
		placeholderString = NO;

		_synopsis = [[RakText alloc] init];
		if(_synopsis == nil)
			return NO;
		
		_synopsis.fixedWidth = self.bounds.size.width - 2 * MAIN_TEXT_BORDER;
		[_synopsis setFrameOrigin : NSZeroPoint];

		[_synopsis setAlignment:NSJustifiedTextAlignment];
		[_synopsis.cell setWraps:YES];
		
		[_synopsis setTextColor : [Prefs getSystemColor:GET_COLOR_ACTIVE : nil]];
	}
	
	//Update text
	if(project.description[0] != 0)
	{
		placeholderString = NO;
		
		[_synopsis setStringValue : [[NSString alloc] initWithData:[NSData dataWithBytes:project.description length:sizeof(project.description)] encoding:NSUTF32LittleEndianStringEncoding]];
	}
	else
	{
		placeholderString = YES;
		if(_placeholder == nil)
		{
			_placeholder = [[RakText alloc] initWithText:self.bounds : @"Aucun résumé disponible" : [Prefs getSystemColor:GET_COLOR_ACTIVE : nil]];
			if(_placeholder != nil)
			{
#ifdef LARGE_FONT_FOR_PLACEHOLDERS
				[_placeholder setFont: [NSFont systemFontOfSize:15]];
#endif
				[_placeholder sizeToFit];
			}
		}
	}
	
	//Hide the unused view
	if(_synopsis != nil)
		[_synopsis setHidden : placeholderString];
	
	if(_placeholder != nil)
		[_placeholder setHidden: !placeholderString];

	//Postprocessing around the scrollview
	const CGFloat titleHeight = _title != nil ? _title.bounds.size.height : 0;
	if(needPostProcessing)
	{
		_scrollview = [[RakListScrollView alloc] initWithFrame:[self frameForContent : self.bounds : titleHeight]];
		if(_scrollview == nil)
		{
			_synopsis = nil;
			return NO;
		}

		_scrollview.verticalScroller.alphaValue = 0;
		_scrollview.documentView = _synopsis;
		[self addSubview:_scrollview];
	}
	
	//Update the placeholder string
	if(placeholderString)
	{
		if(_placeholder.superview == nil)
			[_scrollview addSubview:_placeholder];

		[_placeholder setFrameOrigin : [self placeholderOrigin : _scrollview.bounds]];
	}
	
	[self updateScrollViewState];
	
	return YES;
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return;
	
	self.layer.backgroundColor = [Prefs getSystemColor : GET_COLOR_BACKGROUD_CT_READERMODE : nil].CGColor;
	
	if(_synopsis != nil)
		[_synopsis setTextColor : [Prefs getSystemColor:GET_COLOR_ACTIVE : nil]];
	
	if(_placeholder != nil)
		[_placeholder setTextColor : [Prefs getSystemColor:GET_COLOR_ACTIVE : nil]];

}

- (BOOL) isFlipped	{	return YES;	}

#pragma mark - Resize

- (void) setFrame : (NSRect) frameRect : (NSSize) headerSize
{
	[self updateFrame : frameRect : headerSize : NO];
}

- (void) resizeAnimation : (NSRect) frameRect : (NSSize) headerSize
{
	[self updateFrame : frameRect : headerSize : YES];
}

- (void) updateFrame : (NSRect) frame : (NSSize) headerSize : (BOOL) animated
{
	NSRect mainFrame = [self frameFromParent:frame : headerSize];
	const CGFloat titleHeight = _title.bounds.size.height;
	
	if(_synopsis != nil)
	{
		const CGFloat newSynopsisWidth = mainFrame.size.width - 2 * SYNOPSIS_BORDER;
		
		if(_synopsis.fixedWidth != newSynopsisWidth)
			_synopsis.fixedWidth = newSynopsisWidth;
	}

	//Update main frame
	if(animated)
		[self.animator setFrame : mainFrame];
	else
		[self setFrame : mainFrame];

	//Set up new frames
	mainFrame.origin = NSZeroPoint;
	
	const NSRect titleFrame =	[self frameForTitle : mainFrame : titleHeight];

	//Update content frames
	NSRect scrollviewRect = [self frameForContent : mainFrame : titleHeight];

	if(animated)
	{
		[_title.animator setFrame:titleFrame];
		
		[_scrollview.animator setFrame: scrollviewRect];
		if(placeholderString)
			[_placeholder.animator setFrameOrigin : [self placeholderOrigin : scrollviewRect]];
	}
	else
	{
		[_title setFrame:titleFrame];

		[_scrollview setFrame: scrollviewRect];
		if(placeholderString)
		{
			[_placeholder setFrameOrigin : [self placeholderOrigin : scrollviewRect]];
		}
	}

	[self updateScrollViewState];
}

- (void) updateScrollViewState
{
	if(!placeholderString && _synopsis.bounds.size.height > _scrollview.bounds.size.height)
		_scrollview.scrollingDisabled = NO;

	else
		_scrollview.scrollingDisabled = YES;
}

#pragma mark - Position routines

- (NSRect) frameFromParent : (NSRect) parentFrame : (NSSize) headerSize
{
	parentFrame.origin.x = headerSize.width + SYNOPSIS_BORDER;
	parentFrame.size.width -= parentFrame.origin.x + SYNOPSIS_BORDER;
	
	parentFrame.origin.y = parentFrame.size.height - headerSize.height;
	parentFrame.size.height = headerSize.height - TOP_BORDER_WIDTH;
	
	return parentFrame;
}

- (NSRect) frameForTitle : (NSRect) mainBounds : (CGFloat) height
{
	mainBounds.size.height = height;
	
	return mainBounds;
}

- (NSRect) frameForContent : (NSRect) mainBounds : (CGFloat) titleHeight
{
	mainBounds.origin.x = MAIN_TEXT_BORDER;
	mainBounds.origin.y = titleHeight + SPACING;
	mainBounds.size.height -= mainBounds.origin.y + SPACING;
	
	if(_synopsis == nil)
		mainBounds.size.width -= 2 * MAIN_TEXT_BORDER;
	else
		mainBounds.size.width = _synopsis.fixedWidth;
	
	mainBounds.size.width += 30;	//Scroller width
	
	return mainBounds;
}

- (NSPoint) placeholderOrigin : (NSRect) scrollviewBounds
{
	scrollviewBounds.origin = NSZeroPoint;
	
	NSPoint origin = NSCenteredRect(scrollviewBounds, _placeholder.bounds);
	
	origin.x -= MAIN_TEXT_BORDER / 2 + 15;
	
	return origin;
}

@end
