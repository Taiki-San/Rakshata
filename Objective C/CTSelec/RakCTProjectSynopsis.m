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
#define SYNOPSIS_BORDER 20

@implementation RakCTProjectSynopsis

- (instancetype) initWithProject : (PROJECT_DATA) project : (NSRect) frame : (NSSize) headerSize
{
	self = [self initWithFrame : [self frameFromParent : frame : headerSize]];
	
	if(self != nil)
	{
		_title = [[RakMenuText alloc] initWithText : self.bounds : @"Résumé"];
		if(_title != nil)
		{
			[_title sizeToFit];
			_title.ignoreInternalFrameMagic = YES;
			_title.barWidth = 1;

			NSRect titleFrame = [self frameForTitle:self.bounds : _title.bounds.size.height];
			
			[_title setFrame : titleFrame];

			[_title setAlignment : NSRightTextAlignment];
			[self addSubview:_title];
			
			_titleGradient = [[RakCTHImageGradient alloc] init];
			if(_titleGradient)
			{
				_titleGradient.gradientWidth = 1.0f;
				[_titleGradient setFrame:NSMakeRect(0, titleFrame.size.height - _title.barWidth, titleFrame.size.width / 4, _title.barWidth)];

				[Prefs getCurrentTheme:_titleGradient];
				[_titleGradient updateGradient];
				
				[_title addSubview:_titleGradient];
			}
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
	
	if(_synopsis == nil)
	{
		needPostProcessing = YES;
		_synopsis = [[RakText alloc] init];
		if(_synopsis == nil)
			return NO;
		
		_synopsis.fixedWidth = self.bounds.size.width - 2 * SYNOPSIS_BORDER;
		[_synopsis.cell setWraps:YES];
		[_synopsis setAlignment:NSJustifiedTextAlignment];
		
		[_synopsis setTextColor : [Prefs getSystemColor:GET_COLOR_ACTIVE : self]];
	}
	
	if(project.description[0] != 0)
		[_synopsis setStringValue : [[NSString alloc] initWithData:[NSData dataWithBytes:project.description length:sizeof(project.description)] encoding:NSUTF32LittleEndianStringEncoding]];
	else
		[_synopsis setStringValue:@"\nAucun résumé disponible.\n\n"];
	
	const CGFloat titleHeight = _title != nil ? _title.bounds.size.height : 0;
	
	if(needPostProcessing)
	{
		[_synopsis setFrameOrigin : NSZeroPoint];
		
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
	else
		[_scrollview setFrame : [self frameForContent : self.bounds : titleHeight]];
	
	[self updateScrollViewState];
	
	return YES;
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
	
	//We now get the 'clever height', trying to limit our footprint height
	if(_synopsis != nil)
	{
		const CGFloat newSynopsisWidth = mainFrame.size.width - 2 * SYNOPSIS_BORDER;
		
		if(_synopsis.fixedWidth != newSynopsisWidth)
			_synopsis.fixedWidth = newSynopsisWidth;

		const CGFloat limitedHeight = TOP_BORDER_WIDTH + titleHeight + TOP_BORDER_WIDTH + [_synopsis intrinsicContentSize].height;
		
		if(mainFrame.size.height > limitedHeight)
		{
			mainFrame.origin.y -= limitedHeight - mainFrame.size.height;
			mainFrame.size.height = limitedHeight;
		}
	}

	//Update main frame
	if(animated)
		[self.animator setFrame : mainFrame];
	else
		[self setFrame : mainFrame];

	//Set up new frames
	mainFrame.origin = NSZeroPoint;
	
	const NSRect titleFrame =	[self frameForTitle : mainFrame : titleHeight];
	const NSRect gradientFrame = NSMakeRect(0, titleFrame.size.height - _title.barWidth, titleFrame.size.width / 5, _title.barWidth);

	//Update content frames
	if(animated)
	{
		[_title.animator setFrame:titleFrame];
		[_titleGradient.animator setFrame:gradientFrame];

		[_scrollview.animator setFrame: [self frameForContent : mainFrame : titleHeight]];
	}
	else
	{
		[_title setFrame:titleFrame];
		[_titleGradient setFrame:gradientFrame];

		[_scrollview setFrame: [self frameForContent : mainFrame : titleHeight]];
	}

	[self updateScrollViewState];
}

- (void) updateScrollViewState
{
	if(_synopsis.bounds.size.height != _scrollview.bounds.size.height)
		_scrollview.scrollingDisabled = NO;

	else
		_scrollview.scrollingDisabled = YES;
}

#pragma mark - Position routines

- (NSRect) frameFromParent : (NSRect) parentFrame : (NSSize) headerSize
{
	parentFrame.origin.x = headerSize.width;
	parentFrame.size.width -= parentFrame.origin.x;
	
	parentFrame.origin.y = headerSize.height;
	parentFrame.size.height -= parentFrame.origin.y;
	
	return parentFrame;
}

- (NSRect) frameForTitle : (NSRect) mainBounds : (CGFloat) height
{
	mainBounds.origin.y = TOP_BORDER_WIDTH;
	mainBounds.size.height = height;
	
	mainBounds.origin.x = SYNOPSIS_BORDER;
	mainBounds.size.width -= 2 * SYNOPSIS_BORDER;
	return mainBounds;
}

- (NSRect) frameForContent : (NSRect) mainBounds : (CGFloat) titleHeight
{
	mainBounds.origin.x = SYNOPSIS_BORDER;
	mainBounds.origin.y = TOP_BORDER_WIDTH + titleHeight + TOP_BORDER_WIDTH;
	
	if(_synopsis == nil)
	{
		mainBounds.size.width -= SYNOPSIS_BORDER + 5;
		mainBounds.size.height -= mainBounds.origin.y;
	}
	else
	{
		mainBounds.size.width = _synopsis.fixedWidth;
		mainBounds.size.height -= mainBounds.origin.y;
	}
	
	mainBounds.size.width += 30;	//Scroller width
	
	return mainBounds;
}

@end
