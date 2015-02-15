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

#define TITLE_OFFSET 10
#define INFLECTION_POINT 0.7f

#define PROJECT_NAME_PLACEHOLDER @"Project name"

@implementation RakCTSerieHeader

- (instancetype) initWithFrame : (NSRect) frameRect
{
	self = [super initWithFrame : frameRect];
	
	if(self != nil)
	{
		[Prefs getCurrentTheme:self];
		[self updateGradient];
		_projectHaveFocus = NO;

		placeholder = [self craftField:@"Suggestions"];
		if(placeholder != nil)
		{
			[placeholder setFrame:[self titleFrame : _bounds : YES]];
			[self addSubview:placeholder];
		}
		
		title = [self craftField:PROJECT_NAME_PLACEHOLDER];
		if(title != nil)
		{
			[title setFrame:[self titleFrame:_bounds : NO]];
			[self addSubview:title];
		}
		
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(respondToFocus:) name:SR_NOTIFICATION_FOCUS object:nil];
	}
	
	return self;
}

- (RakText *) craftField : (NSString *) string
{
	RakText * output = [[RakText alloc] initWithText:_bounds : string :[self textColor]];
	if(output != nil)
	{
		output.font = [RakMenuText getFont : 17];
		
		[output sizeToFit];
	}
	
	return output;
}

#pragma mark - Change focus

- (void) respondToFocus : (NSNotification *) notification
{
	if(self.isHidden)
		return;
	
	NSNumber * _project = notification.object, * _inOrOut = [notification.userInfo objectForKey:SR_FOCUS_IN_OR_OUT];
	
	if(_project == nil || ![_project isKindOfClass:[NSNumber class]] || _inOrOut == nil || ![_inOrOut isKindOfClass:[NSNumber class]])
		return;
	
	BOOL inOrOut = [_inOrOut boolValue];

	if(!inOrOut)	//Leaving
	{
		[self changeFocus : NO];
	}
	else
	{
		[self performSelectorOnMainThread:@selector(updateProject:) withObject:_project waitUntilDone:YES];
		[self changeFocus:YES];
	}
}

- (void) changeFocus : (BOOL) getIn
{
	if(_projectHaveFocus == getIn)
		return;
	
	_projectHaveFocus = getIn;
	
	if(_ignoreTransitionAnimation)
	{
		[placeholder setFrame : [self titleFrame:_bounds : YES]];
		[title setFrame : [self titleFrame:_bounds : NO]];
	}
	else
	{
		[CATransaction begin];

		[placeholder.animator setFrame : [self titleFrame:_bounds : YES]];
		[title.animator setFrame : [self titleFrame:_bounds : NO]];
		
		[CATransaction commit];
	}
}

- (void) updateProject : (NSNumber *) _projectID
{
	uint projectID = [_projectID unsignedIntValue];
	
	if(_activeProject == projectID)
		return;
	
	_activeProject = projectID;
	PROJECT_DATA project = getElementByID(projectID);
	
	if (!project.isInitialized)
		return;
	
	NSString * newName = getStringForWchar(project.projectName);
	
	//Animation is not required
	if(_ignoreTransitionAnimation || [title.stringValue isEqualToString:PROJECT_NAME_PLACEHOLDER])
	{
		[title setStringValue: newName];
		return;
	}
	
	int compareResult = [title.stringValue caseInsensitiveCompare:newName];
	
	if(compareResult == NSOrderedSame)
	{
		[title setStringValue: newName];
		return;
	}
	
	//Ok, time to animate
	NSRect frame = [self titleFrame:_bounds :!_projectHaveFocus];
	
	//We generate the new title
	RakText * newTitle = [self craftField:PROJECT_NAME_PLACEHOLDER], * oldTitle = title;
	if(newTitle != nil)
	{
		CGFloat oldY = frame.origin.y;
		frame.origin.y = _bounds.size.height * (compareResult == NSOrderedDescending ? 1 : -1);
		
		[newTitle setFrame:frame];
		[newTitle setStringValue: newName];
		[self addSubview:newTitle];

		title = newTitle;
		frame.origin.y = oldY;
	}

	[NSAnimationContext beginGrouping];
	
	if(newTitle != nil)
		[newTitle.animator setFrame : frame];
	
	frame.origin.y = _bounds.size.height * (compareResult == NSOrderedDescending ? -1 : 1);
	[oldTitle.animator setFrameOrigin:frame.origin];
	
	[NSAnimationContext endGrouping];
	
	dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{	[oldTitle removeFromSuperview];	});
}

#pragma mark - Sizing

- (void) setFrame : (NSRect) frameRect
{
	_cachedFrame = frameRect;

	if(!self.isHidden)
	{
		[super setFrame:frameRect];
		
		[placeholder setFrame : [self titleFrame:frameRect : YES]];
		[title setFrame : [self titleFrame:frameRect : NO]];
	}
}

- (void) resizeAnimation : (NSRect) frameRect
{
	_cachedFrame = frameRect;

	if(!self.isHidden)
	{
		[self.animator setFrame:frameRect];
		
		[placeholder.animator setFrame : [self titleFrame:frameRect : YES]];
		[title.animator setFrame : [self titleFrame:frameRect : NO]];
	}
}

- (void) setHidden:(BOOL)flag
{
	BOOL needUpdate = self.isHidden && flag == NO;
	
	[super setHidden:flag];
	
	if(needUpdate)
		[self setFrame:_cachedFrame];
}

- (NSRect) titleFrame : (NSRect) frame : (BOOL) isPlaceholder
{
	if(isPlaceholder)
	{
		frame.origin.x = _projectHaveFocus ? -frame.size.width : TITLE_OFFSET;

		frame.origin.y = frame.size.height / 2 - placeholder.bounds.size.height / 2;
		frame.size.height = placeholder.bounds.size.height;
	}
	else
	{
		frame.origin.x = _projectHaveFocus ? TITLE_OFFSET : frame.size.width;

		frame.origin.y = frame.size.height / 2 - title.bounds.size.height / 2;
		frame.size.height = title.bounds.size.height;
	}
	
	return frame;
}

#pragma mark - Drawing

- (NSColor *) textColor
{
	return [Prefs getSystemColor:GET_COLOR_INACTIVE : nil];
}

- (NSColor *) backgroundColor
{
	return [Prefs getSystemColor:GET_COLOR_BACKGROUND_TABS :nil];
}

- (NSColor *) getGradientColor
{
	return [Prefs getSystemColor:GET_COLOR_BORDERS_COREVIEWS : nil];
}

- (void) updateGradient
{
	const NSColor * activeColor = [self getGradientColor];
	_gradient = [[NSGradient alloc] initWithColors:@[activeColor, activeColor, [NSColor clearColor]] atLocations:(const CGFloat [3]) {0.0f, INFLECTION_POINT, 1.0f} colorSpace:[NSColorSpace genericRGBColorSpace]];
}

- (void) drawRect:(NSRect)dirtyRect
{
	[[self backgroundColor] setFill];
	NSRectFill(dirtyRect);
	
	dirtyRect.size.width -= 1;
	dirtyRect.origin.x = dirtyRect.size.height = 1;
	[_gradient drawInRect : dirtyRect angle : 0];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if ([object class] != [Prefs class])
		return;
	
	[self updateGradient];
}

@end
