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

enum
{
	TOP_BORDER = 2,
	BORDER_NAME = 1,
	BORDER_STARS = 3,
	BAR_SEPARATOR = 5,
	
	SEPARATOR_WIDTH = 3,		//The - between the type and the tag
	SUB_ELEMENT_FONT_SIZE = 11,
};

@implementation RakCTFocusSRItem

- (instancetype) initWithProject:(PROJECT_DATA)project reason : (byte) reason
{
	_reason = reason;
	self = [self initWithProject:project];
	
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
	
	stars = [[RakStarView alloc] init:_project];
	if(stars != nil)
		[self addSubview:stars];
	
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

- (void) updateProject : (PROJECT_DATA) project
{
	//We really don't care about those data, so we don't want the burden of having to update them
	project.chapitresFull = project.chapitresInstalled = NULL;
	project.tomesFull = project.tomesInstalled = NULL;
	project.chapitresPrix = NULL;
	
	_project = project;
	
	NSImage * image = loadImage(_project, PROJ_IMG_SUFFIX_SRGRID);
	if(image != nil)
		thumbnail.image = image;
	
	projectName.stringValue = getStringForWchar(project.projectName);
	
	[self initReason];
}

#pragma mark - Sizing

- (NSPoint) resizeContent : (NSSize) newSize : (BOOL) animated
{
	NSPoint previousOrigin = [super resizeContent:newSize :animated];
	
	if(_reason == SUGGESTION_REASON_AUTHOR)
	{
		if(animated)
			[projectAuthor.animator setFrameOrigin:	(previousOrigin = [self originOfAuthor : _workingArea : previousOrigin])];
		else
			[projectAuthor setFrameOrigin:	(previousOrigin = [self originOfAuthor : _workingArea : previousOrigin])];
	}
	else
	{
		if(animated)
		{
			[typeProject.animator setFrameOrigin:	(previousOrigin = [self originOfType : _workingArea : previousOrigin])];
			[tagProject.animator setFrameOrigin:	(previousOrigin = [self originOfTag : _workingArea : previousOrigin])];
		}
		else
		{
			[typeProject setFrameOrigin:	(previousOrigin = [self originOfType : _workingArea : previousOrigin])];
			[tagProject setFrameOrigin:		(previousOrigin = [self originOfTag : _workingArea : previousOrigin])];
		}
	}
	
	if(animated)
		[stars.animator setFrameOrigin:	(previousOrigin = [self originOfStars : _workingArea : previousOrigin])];
	else
		[stars setFrameOrigin:			(previousOrigin = [self originOfStars : _workingArea : previousOrigin])];
	
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

- (NSPoint) originOfStars : (NSRect) frameRect : (NSPoint) tagOrigin
{
	NSPoint center = NSCenteredRect(frameRect, stars.bounds);
	
	center.y = tagOrigin.y - BORDER_STARS - stars.bounds.size.height;
	
	return center;
}

- (CGFloat) getMinimumHeight
{
	return TOP_BORDER + 1 + BAR_SEPARATOR + stars.bounds.size.height + BORDER_STARS + (_reason == SUGGESTION_REASON_TAG ? MAX(typeProject.bounds.size.height, tagProject.bounds.size.height) : projectAuthor.bounds.size.height) + projectName.bounds.size.height + BORDER_NAME + [self thumbSize].height + TOP_BORDER;
}

#pragma mark - Drawing

- (NSColor *) borderColor
{
	return [Prefs getSystemColor:GET_COLOR_INACTIVE :nil];
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
	
	if(!_last)
	{
		dirtyRect = _workingArea;
		dirtyRect.size.height = 1;
		
		[[self borderColor] setFill];
		NSRectFill(dirtyRect);
	}
}

#pragma mark - Clic management

- (void) mouseDown:(NSEvent *)theEvent
{
	if(_table != nil)
		_table.clickedID = _project.cacheDBID;
}

- (void) mouseUp:(NSEvent *)theEvent
{
	if(_table == nil || _table.clickedID != _project.cacheDBID)
		return;
	
	NSPoint point = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	
	if(NSPointInRect(point, thumbnail.frame) || NSPointInRect(point, projectName.frame))
	{
		PROJECT_DATA dataToSend = getElementByID(_project.cacheDBID);
		
		if(dataToSend.isInitialized)
			[RakTabView broadcastUpdateContext: [[NSApp delegate] serie] : dataToSend : NO : VALEUR_FIN_STRUCT];
	}
	else if(_reason == SUGGESTION_REASON_AUTHOR && NSPointInRect(point, projectAuthor.frame))
	{
		uint ID = _getFromSearch(NULL, PULL_SEARCH_AUTHORID, &(_project.authorName));
		
		if(ID != UINT_MAX)
		{
			[[NSNotificationCenter defaultCenter] postNotificationName:SR_NOTIFICATION_AUTHOR object:getStringForWchar(_project.authorName) userInfo:@{SR_NOTIF_CACHEID : @(ID), SR_NOTIF_OPTYPE : @(YES)}];
		}
	}
	else if(_reason == SUGGESTION_REASON_TAG)
	{
		if(NSPointInRect(point, typeProject.frame))
		{
			uint ID = _getFromSearch(NULL, PULL_SEARCH_CATID, &(_project.category));
			
			if(ID != UINT_MAX)
			{
				[[NSNotificationCenter defaultCenter] postNotificationName:SR_NOTIFICATION_TYPE object:getStringForWchar(getCatNameForCode(_project.category)) userInfo:@{SR_NOTIF_CACHEID : @(ID), SR_NOTIF_OPTYPE : @(YES)}];
			}
		}
		else if(NSPointInRect(point, tagProject.frame))
		{
			uint ID = _getFromSearch(NULL, PULL_SEARCH_TAGID, &(_project.mainTag));
			
			if(ID != UINT_MAX)
			{
				[[NSNotificationCenter defaultCenter] postNotificationName:SR_NOTIFICATION_TAG object:getStringForWchar(getTagNameForCode(_project.mainTag)) userInfo:@{SR_NOTIF_CACHEID : @(ID), SR_NOTIF_OPTYPE : @(YES)}];
			}
		}
	}
}

@end
