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
 *********************************************************************************************/

enum
{
	TITLE_OFFSET = 10,
	RATING_OFFSET = 1
};

#define INFLECTION_POINT 0.7f
#define PROJECT_NAME_PLACEHOLDER @"Project name"

@implementation RakCTSerieHeader

- (instancetype) initWithFrame : (NSRect) frameRect
{
	self = [super initWithFrame : frameRect];
	
	if(self != nil)
	{
		[Prefs registerForChange:self forType:KVO_THEME];
		[self updateGradient];
		_projectHaveFocus = NO;
		
		placeholder = [self craftField:NSLocalizedString(@"SUGGESTIONS", nil)];
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
		
		rating = [[RakStarView alloc] init: getEmptyProject()];
		if(rating != nil)
		{
			rating.wantNumber = YES;
			
			[rating setFrameOrigin:[self ratingOrigin:frameRect]];
			[self addSubview:rating];
		}
		
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(respondToFocus:) name:SR_NOTIFICATION_FOCUS object:nil];
	}
	
	return self;
}

- (void) dealloc
{
	[Prefs deRegisterForChange:self forType:KVO_THEME];
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

- (RakStarView *) craftRating : (PROJECT_DATA) project
{
	RakStarView * output = [[RakStarView alloc] init: project];
	if(rating != nil)
	{
		output.wantNumber = YES;
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
	
	[self lockFocusIfCanDraw];
	[CATransaction begin];
	
	if(getIn)
	{
		title.alphaValue = 1;
		[title setFrame : [self titleFrame:_bounds :NO]];
		[rating setFrameOrigin:[self ratingOrigin:_bounds : NO : YES]];
		
		[CATransaction commit];
		[CATransaction begin];
	}
	
	_projectHaveFocus = getIn;
	
	[placeholder setFrameAnimated : [self titleFrame:_bounds : YES]];
	[title setFrameAnimated : [self titleFrame:_bounds : NO]];
	[rating setFrameOriginAnimated:[self ratingOrigin:_bounds :_projectHaveFocus :YES]];
	
	if(!getIn)
		title. alphaAnimated = 0;
	
	[CATransaction commit];
	[self unlockFocus];
}

- (void) updateProject : (NSNumber *) _projectID
{
	uint projectID = [_projectID unsignedIntValue];
	
	if(_activeProject == projectID)
		return;
	
	_activeProject = projectID;
	PROJECT_DATA project = getProjectByID(projectID);
	
	if(!project.isInitialized)
		return;
	else
		releaseCTData(project);
	
	[rating updateProject : project];
	
	NSString * newName = getStringForWchar(project.projectName);
	
	//Animation is not required
	if(!_projectHaveFocus || [title.stringValue isEqualToString:PROJECT_NAME_PLACEHOLDER])
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
	RakStarView * newStars = [self craftRating:project], * oldRating = rating;
	if(newTitle != nil)
	{
		CGFloat oldY = frame.origin.y;
		frame.origin.y = _bounds.size.height * (compareResult == NSOrderedDescending ? 1 : -1);
		
		[newTitle setFrame:frame];
		[newTitle setStringValue: newName];
		[self addSubview:newTitle];
		
		if(newStars != nil)
		{
			rating = newStars;
			
			[newStars setFrameOrigin:NSMakePoint([self ratingOrigin:_bounds].x, frame.origin.y)];
			
			[self addSubview:newStars];
		}
		
		title = newTitle;
		frame.origin.y = oldY;
	}
	
	[NSAnimationContext beginGrouping];
	
	if(newTitle != nil)
		[newTitle setFrameAnimated : frame];
	
	if(newStars != nil)
		[newStars setFrameOriginAnimated:[self ratingOrigin:_bounds]];
	
	frame.origin.y = _bounds.size.height * (compareResult == NSOrderedDescending ? -1 : 1);
	
	[oldTitle setFrameOriginAnimated:frame.origin];
	[oldRating setFrameOriginAnimated:NSMakePoint(newStars.frame.origin.x, frame.origin.y)];
	
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
		
		[rating setFrameOrigin:[self ratingOrigin:frameRect]];
	}
}

- (void) resizeAnimation : (NSRect) frameRect
{
	_cachedFrame = frameRect;
	
	if(!self.isHidden)
	{
		[self setFrameAnimated:frameRect];
		
		[placeholder setFrameAnimated : [self titleFrame:frameRect : YES]];
		[title setFrameAnimated : [self titleFrame:frameRect : NO]];
		
		[rating setFrameOriginAnimated:[self ratingOrigin:frameRect]];
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

- (NSPoint) ratingOrigin : (NSRect) frame
{
	return [self ratingOrigin:frame : _projectHaveFocus : NO];
}

- (NSPoint) ratingOrigin : (NSRect) frame : (BOOL) isShown : (BOOL) changingState
{
	return NSMakePoint(frame.size.width + (isShown ? (- 5 - rating.bounds.size.width) : (changingState ? frame.size.width : 0)), RATING_OFFSET);
}

#pragma mark - Drawing

- (RakColor *) textColor
{
	return [Prefs getSystemColor:COLOR_INACTIVE];
}

- (RakColor *) backgroundColor
{
	return [Prefs getSystemColor:COLOR_TABS_BACKGROUND];
}

- (RakColor *) getGradientColor
{
	return [Prefs getSystemColor:COLOR_COREVIEW_BORDER];
}

- (void) updateGradient
{
	const RakColor * activeColor = [self getGradientColor];
	_gradient = [[NSGradient alloc] initWithColors:@[activeColor, activeColor, [RakColor clearColor]] atLocations:(const CGFloat [3]) {0.0f, INFLECTION_POINT, 1.0f} colorSpace:[NSColorSpace genericRGBColorSpace]];
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
	if([object class] != [Prefs class])
		return;
	
	[self updateGradient];
	
	placeholder.textColor = [self textColor];
	title.textColor = [self textColor];
	
	[self setNeedsDisplay:YES];
}

@end
