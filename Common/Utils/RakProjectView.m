/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 ********************************************************************************************/

enum
{
	BORDER_THUMB = 150,
	
	//RakThumb
	TOP_BORDER = 2,
	BORDER_NAME = 1,
	BAR_SEPARATOR = 5,
	
	SEPARATOR_WIDTH = 3,		//The - between the type and the tag
	SUB_ELEMENT_FONT_SIZE = 11,
};

@implementation RakBasicProjectView

- (instancetype) initWithProject : (PROJECT_DATA) project
{
	return [self initWithProject:project withInsertionPoint:nil];
}

- (instancetype) initWithProject : (PROJECT_DATA) project withInsertionPoint : (NSDictionary *) insertionPoint
{
	if(!project.isInitialized)
		return nil;
	
	self = [self initWithFrame:NSMakeRect(0, 0, RCVC_MINIMUM_WIDTH, RCVC_MINIMUM_HEIGHT)];
	
	if(self != nil)
	{
		_insertionPoint = insertionPoint;
		
		//We really don't care about those data, so we don't want the burden of having to update them
		nullifyCTPointers(&project);

		_project = project;
		_workingArea.origin = NSZeroPoint;
		_workingArea.size = [self defaultWorkingSize];
		
		[self initContent];
		
		registerThumbnailUpdate(self, @selector(thumbnailUpdate:), THUMBID_HEAD);
		[RakDBUpdate registerForUpdate:self :@selector(DBUpdated:)];
	}
	
	return self;
}

- (void) initContent
{
	registerdPref = YES;
	[Prefs registerForChange:self forType:KVO_THEME];
	
	RakImage * image = loadImageGrid(_project);
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

- (void) DBUpdated : (NSNotification *) notification
{
	if([RakDBUpdate analyseNeedUpdateProject:notification.userInfo :_project])
	{
		PROJECT_DATA project = getProjectByID(_project.cacheDBID);
		if(!_project.isInitialized)
			return;

		releaseCTData(project);
		[self updateProject:project];
	}
}

- (void) dealloc
{
	if(registerdPref)
		[Prefs deRegisterForChange:self forType:KVO_THEME];
	
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	[RakDBUpdate unRegister:self];
}

- (void) updateProject : (PROJECT_DATA) project
{
	[self updateProject:project withInsertionPoint:nil];
}

- (void) updateProject : (PROJECT_DATA) project withInsertionPoint : (NSDictionary *) insertionPoint
{
	//We really don't care about those data, so we don't want the burden of having to update them
	nullifyCTPointers(&project);
	
	_project = project;
	_insertionPoint = insertionPoint;
	
	if(projectName != nil)
		projectName.stringValue = getStringForWchar(_project.projectName);
	
	if(projectAuthor)
		projectAuthor.stringValue = getStringForWchar(_project.authorName);
	
	RakImage * image = loadImageGrid(_project);
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

- (RakText *) getTextElement : (NSString *) string : (RakColor *) color : (byte) fontCode : (CGFloat) fontSize
{
	RakText * output = [[RakText alloc] initWithText :string : color];
	if(output != nil)
	{
		output.alignment = NSTextAlignmentCenter;
		output.font = [NSFont fontWithName:[Prefs getFontName:fontCode] size:fontSize];
		
		[output.cell setWraps : YES];
		[output sizeToFit];
	}
	
	return output;
}

#pragma mark - Color

- (RakColor *) getTextColor
{
	return [Prefs getSystemColor:COLOR_CLICKABLE_TEXT];
}

- (RakColor *) getTagTextColor
{
	return [Prefs getSystemColor:COLOR_TAGITEM_FONT];
}

- (RakColor *) backgroundColor
{
	return [Prefs getSystemColor : COLOR_GRID_FOCUS_BACKGROUND];
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
		[thumbnail setFrameOriginAnimated:		previousOrigin];
		[projectName setFrameOriginAnimated: 	(previousOrigin = [self originOfName : _workingArea : previousOrigin])];
	}
	else
	{
		[thumbnail setFrameOrigin: 		previousOrigin];
		[projectName setFrameOrigin: 	(previousOrigin = [self originOfName : _workingArea : previousOrigin])];
	}
	
	return previousOrigin;
}

- (NSPoint) reloadOrigin
{
	NSPoint previousOrigin = [self originOfThumb : _workingArea];
	[thumbnail setFrameOrigin: 		previousOrigin];
	[projectName setFrameOrigin: 	(previousOrigin = [self originOfName : _workingArea : previousOrigin])];
	
	return previousOrigin;
}

@end

@implementation RakThumbProjectView

- (instancetype) initWithProject : (PROJECT_DATA) project reason : (byte) reason insertionPoint : (NSDictionary *) insertionPoint
{
	_reason = reason;
	self = [self initWithProject:project withInsertionPoint:insertionPoint];
	
	if(self != nil)
		[self setFrameSize:NSMakeSize(_workingArea.size.width, _workingArea.size.height + 2 * TOP_BORDER)];
	
	return self;
}

- (void) initContent
{
	[super initContent];
	
	projectName = [self getTextElement :getStringForWchar(_project.projectName) : [self getTextColor] : GET_FONT_STANDARD : 12];
	if(projectName != nil)
	{
		projectName.fixedWidth = 180;
		[self addSubview:projectName];
	}
	
	[self initReason];
	
	_workingArea.size.height = MAX(RCVC_MINIMUM_HEIGHT, [self getMinimumHeight]) - 2 * TOP_BORDER;
	_workingArea.origin = NSCenterSize(_bounds.size, _workingArea.size);
}

- (void) initReason
{
	if(_reason == SUGGESTION_REASON_AUTHOR)
	{
		//Remove the old views if required
		if(typeProject != nil && [typeProject superview] != nil)
			[typeProject removeFromSuperview];
		
		if(tagProject != nil && [tagProject superview] != nil)
			[tagProject removeFromSuperview];
		
		//Create or update the main view
		if(projectAuthor == nil)
			projectAuthor = [self getTextElement:getStringForWchar(_project.authorName) :[self getTagTextColor] : GET_FONT_STANDARD : SUB_ELEMENT_FONT_SIZE];
		else
		{
			projectAuthor.stringValue = getStringForWchar(_project.authorName);
			[projectAuthor sizeToFit];
		}
		
		//If init went well or if we hid it, we insert it again in the view hierarchy
		if(projectAuthor != nil && [projectAuthor superview] == nil)
			[self addSubview:projectAuthor];
	}
	else
	{
		//Remove the old views if required
		if(projectAuthor != nil && [projectAuthor superview] != nil)
			[projectAuthor removeFromSuperview];
		
		//Create or update the main views
		if(typeProject == nil)
			typeProject = [self getTextElement:getStringForWchar(getCatNameForCode(_project.category)) :[self getTagTextColor] : GET_FONT_STANDARD : SUB_ELEMENT_FONT_SIZE];
		else
		{
			typeProject.stringValue = getStringForWchar(getCatNameForCode(_project.category));
			[typeProject sizeToFit];
		}
		
		if(tagProject == nil)
			tagProject = [self getTextElement:getStringForWchar(getTagNameForCode(_project.mainTag)) :[self getTagTextColor] : GET_FONT_STANDARD : SUB_ELEMENT_FONT_SIZE];
		else
		{
			tagProject.stringValue = getStringForWchar(getTagNameForCode(_project.mainTag));
			[tagProject sizeToFit];
		}
		
		//If init went well or if we hid it, we insert it again in the view hierarchy
		if(typeProject != nil && [typeProject superview] == nil)
			[self addSubview:typeProject];
		
		if(tagProject != nil && [tagProject superview] == nil)
			[self addSubview:tagProject];
	}
}

- (void) updateProject : (PROJECT_DATA) project insertionPoint : (NSDictionary *) insertionPoint
{
	[super updateProject:project withInsertionPoint:insertionPoint];
	[self initReason];
	[self reloadOrigin];
}

#pragma mark - Sizing

- (void) updateSizes
{
	NSSize size = _bounds.size;
	
	//Too small
	if(size.width < 10)
		return;
	
	size.width -= 10;
	
	for(RakText * view in [self getTextWithFixedWidth])
	{
		if(_mustHoldTheWidth)
		{
			view.enableWraps = YES;
			view.fixedWidth = size.width;
		}
		else
		{
			view.enableWraps = NO;
			[view sizeToFit];	//setting fixedWidth already triggers the resizing
		}
	}
}

- (NSArray <RakText *> *) getTextWithFixedWidth
{
	return projectAuthor == nil ? @[projectName] : @[projectName, projectAuthor];
}

- (NSPoint) resizeContent : (NSSize) newSize : (BOOL) animated
{
	NSPoint previousOrigin = [super resizeContent:newSize :animated];
	
	if(_reason == SUGGESTION_REASON_AUTHOR)
	{
		if(animated)
			[projectAuthor setFrameOriginAnimated:	(previousOrigin = [self originOfAuthor : _workingArea : previousOrigin])];
		else
			[projectAuthor setFrameOrigin:	(previousOrigin = [self originOfAuthor : _workingArea : previousOrigin])];
	}
	else
	{
		if(animated)
		{
			[typeProject setFrameOriginAnimated:	(previousOrigin = [self originOfType : _workingArea : previousOrigin])];
			[tagProject setFrameOriginAnimated:	(previousOrigin = [self originOfTag : _workingArea : previousOrigin])];
		}
		else
		{
			[typeProject setFrameOrigin:	(previousOrigin = [self originOfType : _workingArea : previousOrigin])];
			[tagProject setFrameOrigin:		(previousOrigin = [self originOfTag : _workingArea : previousOrigin])];
		}
	}
	
	return previousOrigin;
}

- (NSPoint) reloadOrigin
{
	NSPoint previousOrigin = [super reloadOrigin];
	
	if(_reason == SUGGESTION_REASON_AUTHOR)
		[projectAuthor setFrameOrigin:	(previousOrigin = [self originOfAuthor : _workingArea : previousOrigin])];
	else
	{
		[typeProject setFrameOrigin:	(previousOrigin = [self originOfType : _workingArea : previousOrigin])];
		[tagProject setFrameOrigin:		(previousOrigin = [self originOfTag : _workingArea : previousOrigin])];
	}
	
	return previousOrigin;
}

- (NSSize) defaultWorkingSize
{
	return NSMakeSize(RCVC_MINIMUM_WIDTH, RCVC_MINIMUM_HEIGHT);
}

- (NSPoint) originOfName:(NSRect)frameRect :(NSPoint)thumbOrigin
{
	NSPoint output = [super originOfName:frameRect :thumbOrigin];
	
	output.y -= BORDER_NAME;
	
	return output;
}

- (NSPoint) originOfType : (NSRect) frameRect : (NSPoint) nameOrigin
{
	NSSize typeSize = typeProject.bounds.size, tagSize = tagProject.bounds.size;
	NSRect mergedSize = NSZeroRect;
	
	mergedSize.size.height = MAX(typeSize.height, tagSize.height);
	mergedSize.size.width = typeSize.width + SEPARATOR_WIDTH + tagSize.width;
	
	NSPoint center;
	center.x = frameRect.origin.x + frameRect.size.width / 2 - mergedSize.size.width / 2;
	center.y = nameOrigin.y - typeProject.bounds.size.height;
	
	return center;
}

- (NSPoint) originOfTag : (NSRect) frameRect : (NSPoint) typeOrigin
{
	typeOrigin.x += typeProject.bounds.size.width + SEPARATOR_WIDTH;
	
	return typeOrigin;
}

- (CGFloat) getMinimumHeight
{
	return TOP_BORDER + 1 + BAR_SEPARATOR + (_reason == SUGGESTION_REASON_TAG ? MAX(typeProject.bounds.size.height, tagProject.bounds.size.height) : projectAuthor.bounds.size.height) + projectName.bounds.size.height + BORDER_NAME + [self thumbSize].height + TOP_BORDER;
}

#pragma mark - Properties

- (void) setMustHoldTheWidth:(BOOL)mustHoldTheWidth
{
	if(_mustHoldTheWidth == mustHoldTheWidth)
		return;
	
	_mustHoldTheWidth = mustHoldTheWidth;
	[self updateSizes];
	
	NSPoint newLowestOrigin = [self reloadOrigin];
	
	newLowestOrigin.y -= TOP_BORDER;
	
	if(newLowestOrigin.y < 0)
	{
		NSSize size = _bounds.size;
		
		size.height -= newLowestOrigin.y;	//newLowestOrigin is negative
		
		[self setFrameSize:size];
		[self reloadOrigin];
	}
}

#pragma mark - Drawing

- (RakColor *) borderColor
{
	return [Prefs getSystemColor:COLOR_INACTIVE];
}

- (void) reloadColors
{
	projectName.textColor = [self getTextColor];
	
	RakColor * detailColor = [self getTagTextColor];
	
	if(projectAuthor != nil)
		projectAuthor.textColor = detailColor;
	
	if(tagProject != nil)
		tagProject.textColor = detailColor;
	
	if(typeProject != nil)
		typeProject.textColor = detailColor;
}

- (void) drawRect : (NSRect) dirtyRect
{
	if(_reason == SUGGESTION_REASON_TAG)
	{
		NSRect typeFrame = typeProject.frame;
		
		typeFrame.origin.x += typeFrame.size.width;
		typeFrame.origin.y += floor(typeFrame.size.height / 2) - 1;
		typeFrame.size.height = 1;
		typeFrame.size.width = SEPARATOR_WIDTH;
		
		[[self getTextColor] setFill];
		NSRectFill(typeFrame);
	}
}

#pragma mark - Event management

- (void) mouseUp:(NSEvent *)theEvent
{
	byte selection = THUMBVIEW_CLICK_NONE;
	
	//Determine the item to be clicked
	NSPoint point = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	
	if(NSPointInRect(point, thumbnail.frame) || NSPointInRect(point, projectName.frame))
		selection = THUMBVIEW_CLICK_PROJECT;

	else if(_reason == SUGGESTION_REASON_AUTHOR && NSPointInRect(point, projectAuthor.frame))
		selection = THUMBVIEW_CLICK_AUTHOR;

	else if(_reason == SUGGESTION_REASON_TAG)
	{
		if(NSPointInRect(point, typeProject.frame))
			selection = THUMBVIEW_CLICK_CAT;
		
		else if(NSPointInRect(point, tagProject.frame))
			selection = THUMBVIEW_CLICK_TAG;
	}
	
	//Call the callback if asked
	if(_controller != nil && [_controller respondsToSelector:_clickValidation])
	{
		IMP imp = [_controller methodForSelector:_clickValidation];
		if(imp != nil)
		{
			BOOL (*func)(id, SEL, id, byte) = (void *)imp;
			if(!func(_controller, _clickValidation, self, selection))
				return;
		}
	}
	
	//Notify the UI, most common case
	switch (selection)
	{
		case THUMBVIEW_CLICK_PROJECT:
		{
			PROJECT_DATA dataToSend = getProjectByID(_project.cacheDBID);
			
			if(dataToSend.isInitialized)
				[RakTabView broadcastUpdateContext: RakApp.serie : dataToSend : NO : INVALID_VALUE];
			
			releaseCTData(dataToSend);
			break;
		}
			
		case THUMBVIEW_CLICK_AUTHOR:
		{
			uint ID = _getFromSearch(NULL, PULL_SEARCH_AUTHORID, &(_project.authorName));
			
			if(ID != UINT_MAX)
			{
				[[NSNotificationCenter defaultCenter] postNotificationName:SR_NOTIFICATION_AUTHOR object:getStringForWchar(_project.authorName) userInfo:@{SR_NOTIF_CACHEID : @(ID), SR_NOTIF_OPTYPE : @(YES)}];
			}
			
			break;
		}
			
		case THUMBVIEW_CLICK_TAG:
		{
			uint ID = _getFromSearch(NULL, PULL_SEARCH_TAGID, &(_project.mainTag));
			
			if(ID != UINT_MAX)
			{
				[[NSNotificationCenter defaultCenter] postNotificationName:SR_NOTIFICATION_TAG object:getStringForWchar(getTagNameForCode(_project.mainTag)) userInfo:@{SR_NOTIF_CACHEID : @(ID), SR_NOTIF_OPTYPE : @(YES)}];
			}

			break;
		}
			
		case THUMBVIEW_CLICK_CAT:
		{
			uint ID = _getFromSearch(NULL, PULL_SEARCH_CATID, &(_project.category));
			
			if(ID != UINT_MAX)
			{
				[[NSNotificationCenter defaultCenter] postNotificationName:SR_NOTIFICATION_TYPE object:getStringForWchar(getCatNameForCode(_project.category)) userInfo:@{SR_NOTIF_CACHEID : @(ID), SR_NOTIF_OPTYPE : @(YES)}];
			}
			
			break;
		}
	}
}

@end