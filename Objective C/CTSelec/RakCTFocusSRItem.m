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
	SEPARATOR_WIDTH = 3,
	SUB_ELEMENT_FONT_SIZE = 11
};

@implementation RakCTFocusSRItem

- (instancetype) initWithProject:(PROJECT_DATA)project reason : (byte) reason
{
	_reason = reason;
	
	self = [self initWithProject:project];
	
	if(self != nil)
	{
		
	}
	
	return self;
}

- (void) initContent
{
	[super initContent];
	
	projectName = [self getTextElement :getStringForWchar(_project.projectName) : [self getTextColor] : GET_FONT_STANDARD : 13];
	if(projectName != nil)
	{
		projectName.fixedWidth = 180;
		[self addSubview:projectName];
	}
	
	[self initReason];
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
			typeProject = [self getTextElement:getStringForWchar(getTagForCode(_project.type)) :[self getTagTextColor] : GET_FONT_STANDARD : SUB_ELEMENT_FONT_SIZE];
		else
		{
			typeProject.stringValue = getStringForWchar(getTagForCode(_project.type));
			[typeProject sizeToFit];
		}
		
		if(tagProject == nil)
			tagProject = [self getTextElement:getStringForWchar(getTagForCode(_project.tag)) :[self getTagTextColor] : GET_FONT_STANDARD : SUB_ELEMENT_FONT_SIZE];
		else
		{
			tagProject.stringValue = getStringForWchar(getTagForCode(_project.tag));
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
	
	NSImage * image = [self loadImage];
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

	return previousOrigin;
}

- (NSSize) defaultWorkingSize
{
	return NSMakeSize(RCVC_MINIMUM_WIDTH, RCVC_MINIMUM_HEIGHT);
}

- (NSPoint) originOfType : (NSRect) frameRect : (NSPoint) nameOrigin
{
	NSSize typeSize = typeProject.bounds.size, tagSize = tagProject.bounds.size;
	NSRect mergedSize = NSZeroRect;
	
	mergedSize.size.height = MAX(typeSize.height, tagSize.height);
	mergedSize.size.width = typeSize.width + SEPARATOR_WIDTH + tagSize.width;
	
	NSPoint center = NSCenteredRect(frameRect, mergedSize);
	
	center.y = nameOrigin.y - typeProject.bounds.size.height;
	
	return center;
}

- (NSPoint) originOfTag : (NSRect) frameRect : (NSPoint) typeOrigin
{
	typeOrigin.x += typeProject.bounds.size.width + SEPARATOR_WIDTH;
	
	return typeOrigin;
}

#pragma mark - Drawing

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

@end
