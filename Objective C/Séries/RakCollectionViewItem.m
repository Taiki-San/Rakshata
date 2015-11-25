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

enum	{	BORDER_BOTTOM	= 7	};

@implementation RakCollectionViewItem

- (instancetype) initWithProject : (PROJECT_DATA) project : (uint *) sharedActive
{
	self = [self initWithProject : project];
	
	if(self != nil)
	{
		_selected = NO;
		_sharedActive = sharedActive;
		
		[self setFrameSize: NSMakeSize(_workingArea.size.height + BORDER_BOTTOM, _workingArea.size.width)];
		
		//The menu will reuse our appearance
		CONFIGURE_APPEARANCE_DARK(self);
		
		menuManager = [[RakProjectMenu alloc] initWithProject:_project];
		if(menuManager != nil)
			[menuManager configureCleanMenu:(superCriticalFlag = ((RakWindow *) self.window).optionPressed) toView:self];
	}
	
	return self;
}

- (NSSize) defaultWorkingSize
{
	return NSMakeSize(RCVC_MINIMUM_WIDTH, RCVC_MINIMUM_HEIGHT);
}

- (void) initContent
{
	[super initContent];
	
	projectName = [self getTextElement :getStringForWchar(_project.projectName) : [self getTextColor] : GET_FONT_SR_TITLE : 13];
	if(projectName != nil)
	{
		projectName.fixedWidth = RCVC_MINIMUM_WIDTH * 0.8;
		[self addSubview:projectName];
	}
	
	projectAuthor = [self getTextElement:getStringForWchar(_project.authorName) : [self getTextColor] : GET_FONT_STANDARD : 10];
	if(projectAuthor != nil)
	{
		projectAuthor.fixedWidth = RCVC_MINIMUM_WIDTH * 0.8;
		[self addSubview:projectAuthor];
	}
	
	mainTag = [self getTextElement:getStringForWchar(getTagNameForCode(_project.mainTag)) :[self getTagTextColor] : GET_FONT_STANDARD : 10];
	if(mainTag != nil)
		[self addSubview:mainTag];
	
	_requestedHeight = MAX(RCVC_MINIMUM_HEIGHT, [self getMinimumHeight]);
	
	_workingArea.size.height = _requestedHeight;
	_workingArea.origin.x = _bounds.size.width / 2 - _workingArea.size.width / 2;
	_workingArea.origin.y = _bounds.size.height - _requestedHeight;
}

- (void) updateProject:(PROJECT_DATA)project
{
	[super updateProject:project];
	
	mainTag.stringValue = getStringForWchar(getTagNameForCode(_project.mainTag));
	[mainTag sizeToFit];
	
	menuManager = [[RakProjectMenu alloc] initWithProject:_project];
	if(menuManager != nil)
		[menuManager configureCleanMenu:superCriticalFlag toView:self];
	
	[self reloadOrigin];
}

#pragma mark - Mouse handling

- (BOOL) mouseDownCanMoveWindow
{
	return NO;
}

- (void) mouseEntered:(NSEvent *)theEvent
{
	if(!_selected)
	{
		_selected = YES;
		sessionCode = ++(*_sharedActive);
		[self updateFocus];
		[self setNeedsDisplay:YES];
	}
}

- (void) mouseExited:(NSEvent *)theEvent
{
	if(_selected)
	{
		_selected = NO;
		[self setNeedsDisplay:YES];
	}
}

- (void) mouseDown:(NSEvent *)theEvent
{
	RakCollectionView * view = (id) self.superview;
	
	if([view class] == [RakCollectionView class])
		view.clickedView = self;
	
	[view mouseDown:theEvent];
}

- (void) mouseUp:(NSEvent *)theEvent
{
	NSPoint point = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	
	//Three cases: click on tag, click on author, other
	//The first two trigger a tag, the last select
	
	if(NSPointInRect(point, mainTag.frame))
	{
		uint ID = _getFromSearch(NULL, PULL_SEARCH_TAGID, &(_project.mainTag));
		
		if(ID != UINT_MAX)
		{
			[[NSNotificationCenter defaultCenter] postNotificationName:SR_NOTIFICATION_TAG object:getStringForWchar(getTagNameForCode(_project.mainTag)) userInfo:@{SR_NOTIF_CACHEID : @(ID), SR_NOTIF_OPTYPE : @(YES)}];
		}
	}
	else if(NSPointInRect(point, projectAuthor.frame))
	{
		uint ID = _getFromSearch(NULL, PULL_SEARCH_AUTHORID, &(_project.authorName));
		
		if(ID != UINT_MAX)
		{
			[[NSNotificationCenter defaultCenter] postNotificationName:SR_NOTIFICATION_AUTHOR object:getStringForWchar(_project.authorName) userInfo:@{SR_NOTIF_CACHEID : @(ID), SR_NOTIF_OPTYPE : @(YES)}];
		}
	}
	else if(point.y > mainTag.frame.origin.y)	//We exclude when we are below the main tag
	{
		if(NSPointInRect(point, _workingArea))	//We check we are actually inside the valid area (excluding the padding
		{
			PROJECT_DATA dataToSend = getProjectByID(_project.cacheDBID);
			
			if(dataToSend.isInitialized)
				[RakTabView broadcastUpdateContext: [[NSApp delegate] serie] : dataToSend : NO : INVALID_VALUE];
		}
	}
}

- (void) updateFocus
{
	if(!_project.isInitialized)
		return;
	
	dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(RCVC_FOCUS_DELAY * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
		
		if(*_sharedActive == sessionCode)
			[self updateFocusNow];
	});
}

- (void) updateFocusNow
{
	if([NSThread isMainThread])
		[[NSNotificationCenter defaultCenter] postNotificationName:SR_NOTIFICATION_FOCUS object:@(_project.cacheDBID) userInfo:@{SR_FOCUS_IN_OR_OUT : @(_selected)}];

	else
		dispatch_sync(dispatch_get_main_queue(), ^{	[self updateFocusNow];	});
}

#pragma mark - Resizing code

- (NSPoint) resizeContent : (NSSize) newSize : (BOOL) animated
{
	NSPoint previousOrigin = [super resizeContent:newSize :animated];
	
	if(animated)
	{
		[projectAuthor.animator setFrameOrigin:	(previousOrigin = [self originOfAuthor : _workingArea : previousOrigin])];
		[mainTag.animator setFrameOrigin: 		(previousOrigin = [self originOfTag : _workingArea : previousOrigin])];
	}
	else
	{
		[projectAuthor setFrameOrigin:	(previousOrigin = [self originOfAuthor : _workingArea : previousOrigin])];
		[mainTag setFrameOrigin: 		(previousOrigin = [self originOfTag : _workingArea : previousOrigin])];
	}
	
	return previousOrigin;
}

- (NSPoint) reloadOrigin
{
	NSPoint previousOrigin = [super reloadOrigin];

	[projectAuthor setFrameOrigin:	(previousOrigin = [self originOfAuthor : _workingArea : previousOrigin])];
	[mainTag setFrameOrigin: 		(previousOrigin = [self originOfTag : _workingArea : previousOrigin])];

	return previousOrigin;
}

- (NSPoint) originOfTag : (NSRect) frameRect : (NSPoint) authorOrigin
{
	NSPoint center = NSCenteredRect(frameRect, mainTag.bounds);
	
	center.y = authorOrigin.y - mainTag.bounds.size.height;
	
	return center;
}

- (CGFloat) getMinimumHeight
{
	return mainTag.bounds.size.height + projectAuthor.bounds.size.height + projectName.bounds.size.height + [self thumbSize].height;
}

#pragma mark - Color & Drawing

- (void) reloadColors
{
	projectName.textColor = projectAuthor.textColor = [self getTextColor];
	mainTag.textColor = [self getTagTextColor];
}

- (void) drawRect:(NSRect)dirtyRect
{
	if(_selected)
	{
		NSRect baseFrame = thumbnail.frame;
		
		baseFrame.size.height = baseFrame.origin.y - _workingArea.origin.y + 3;
		baseFrame.origin.y = _workingArea.origin.y;
		
		NSBezierPath * path = [NSBezierPath bezierPathWithRoundedRect:baseFrame xRadius:3 yRadius:3];
		
		[[self backgroundColor] setFill];
		[path fill];
	}
}

#pragma mark - Menu

//Prevent the menu from appearing if clicking outside the active area
- (void) rightMouseDown:(NSEvent *)theEvent
{
	NSPoint point = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	
	if(NSPointInRect(point, _workingArea))
	{
		if(((RakWindow *) self.window).optionPressed != superCriticalFlag)
		{
			superCriticalFlag = !superCriticalFlag;
			menuManager = [[RakProjectMenu alloc] initWithProject:_project];
			if(menuManager != nil)
				[menuManager configureCleanMenu:superCriticalFlag toView:self];
		}
		
		[super rightMouseDown:theEvent];
	}
}

- (void) editSerie
{
	[[[RakEditDetails alloc] initWithProject:_project] queryUser];
}

- (void) deleteSerie
{
	_selected = NO;
	[self setNeedsDisplay:YES];
	[self updateFocusNow];
}

@end
